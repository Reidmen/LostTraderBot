import datetime
import logging
from pathlib import Path
from queue import Queue
from typing import Union, List

import statsmodels.api as sm
import yfinance as yf

from losttraderbot.backtest import Backtest
from losttraderbot.data import DataHandler, HistoricCSVDataHandler
from losttraderbot.event import SignalEvent
from losttraderbot.execution import SimulatedExecutionHandler
from losttraderbot.portfolio import Portfolio
from losttraderbot.strategy import Strategy

path_to_file = Path("./logfiles")
path_to_file.mkdir(parents=True, exist_ok=True)
name = path_to_file.joinpath(f"{str(Path(__file__).stem)}.log")
formatter = logging.Formatter(fmt=" %(name)s :: %(levelname)-8s :: %(message)s")

logger = logging.getLogger("Trader")
logger.setLevel(logging.INFO)

file_handler = logging.FileHandler(str(name))
file_handler.setLevel(logging.INFO)
file_handler.setFormatter(formatter)

console_handler = logging.StreamHandler()
console_handler.setLevel(logging.DEBUG)
console_handler.setFormatter(formatter)

logger.addHandler(file_handler)
logger.addHandler(console_handler)


class OLSMeanReversingStrategy(Strategy):
    def __init__(
        self,
        bars: Union[DataHandler, HistoricCSVDataHandler],
        events: Queue,
        ols_window: int = 100,
        zscore_low: float = 0.5,
        zscore_high: float = 3.0,
        symbol_pair: List[str] = ["MSFT", "GOOG"],
    ):
        self.bars = bars
        self.symbol_list = self.bars.symbol_list
        self.events = events
        self.ols_window = ols_window
        self.zscore_low = zscore_low
        self.zscore_high = zscore_high

        self.symbol_pair = symbol_pair
        self.datetime = datetime.datetime.utcnow()

        self.hedge_ratio = 0.0
        self.long_market = False
        self.short_market = False

    def compute_xy_signals(self, zscore_last: float):
        """Computes the actual x, y signal pairings to be
        sent to the signal generator."""
        x_signal, y_signal = None, None
        dt = self.datetime
        p0, p1 = self.symbol_pair
        hedge_ratio = abs(self.hedge_ratio)

        if zscore_last <= -self.zscore_high and not self.long_market:
            self.long_market = True
            x_signal = SignalEvent(1, p0, dt, "LONG", 1.0)
            y_signal = SignalEvent(1, p1, dt, "SHORT", hedge_ratio)
            logger.info(f"LONG : {p0} SHORT: {p1} HEDGE: {hedge_ratio}")

        if abs(zscore_last) <= self.zscore_low and self.long_market:
            self.long_market = False
            x_signal = SignalEvent(1, p0, dt, "EXIT", 1.0)
            y_signal = SignalEvent(1, p1, dt, "EXIT", 1.0)

        if zscore_last >= self.zscore_high and not self.short_market:
            self.short_market = True
            x_signal = SignalEvent(1, p0, dt, "SHORT", 1.0)
            y_signal = SignalEvent(1, p1, dt, "LONG", hedge_ratio)
            logger.info(f"SHORT: {p0} LONG: {p1} HEDGE: {hedge_ratio}")

        if abs(zscore_last) <= self.zscore_low and self.short_market:
            self.short_market = False
            x_signal = SignalEvent(1, p0, dt, "EXIT", 1.0)
            y_signal = SignalEvent(1, p1, dt, "EXIT", 1.0)

        return y_signal, x_signal

    def compute_signals_for_pairs(self):
        """Computes a new set of signals based on the mean reversion
        strategy."""
        # TODO: uses OLS for the hedge ration, (try CADF)
        y = self.bars._get_latest_bars_values(self.symbol_pair[0], "Close", N=self.ols_window)
        x = self.bars._get_latest_bars_values(self.symbol_pair[1], "Close", N=self.ols_window)

        if y is not None and x is not None:
            # check if all window periods are available
            if len(y) >= self.ols_window and len(x) >= self.ols_window:
                self.hedge_ratio = sm.OLS(y, x).fit().params[0]

                spread = y - self.hedge_ratio * x
                zscore_last = ((spread - spread.mean()) / spread.std())[-1]

                # compute signals and add to the events queue
                y_signal, x_signal = self.compute_xy_signals(zscore_last)
                if y_signal is not None and x_signal is not None:
                    self.events.put(y_signal)
                    self.events.put(x_signal)

    def compute_signals(self, event: SignalEvent):
        if event.type == "MARKET":
            self.compute_signals_for_pairs()


def data_scrapper(symbol: str) -> str:
    """Stock price scrapper using yfinance.

    It creates a new csv dataset if not found in
    the relative path.
    """
    assert isinstance(symbol, str), "symbol must be str"

    Path("./datasets").mkdir(parents=True, exist_ok=True)
    filepath = f"./datasets/dataset_1h_{symbol}.csv"
    path = Path(filepath)

    if not path.is_file():
        dataset_ticker = yf.Ticker(symbol)
        start_date = "2022-01-01"
        end_date = datetime.datetime.now().strftime("%Y-%m-%d")
        dataset_history = dataset_ticker.history(
            start=start_date, end=end_date, interval="1h"
        )
        dataset_history.to_csv(filepath)
        logger.info(f"Head of dataset history for {symbol}")
        logger.debug(dataset_history.head())
    return filepath


if __name__ == "__main__":
    symbol_pair = ["MSFT", "GOOG"]
    csv_dir = [data_scrapper(symbol) for symbol in symbol_pair]
    initial_capital = 100_000
    heartbeat = 0.0
    start_date = datetime.datetime(2022, 1, 1, 0, 0, 0)

    backtest = Backtest(
        csv_dir,
        symbol_pair,
        initial_capital,
        start_date,
        heartbeat,
        HistoricCSVDataHandler,
        SimulatedExecutionHandler,
        OLSMeanReversingStrategy,
        Portfolio,
    )
    backtest.simulate_trading()
