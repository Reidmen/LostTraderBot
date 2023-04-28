import datetime
import os
from pathlib import Path
import numpy as np
import pandas as pd
import statsmodels.api as sm
import yfinance as yf
import logging

from losttraderbot.strategy import Strategy
from losttraderbot.event import Event, MarketEvent, SignalEvent
from losttraderbot.backtest import Backtest
from losttraderbot.data import DataHandler, HistoricCSVDataHandler
from losttraderbot.execution import SimulatedExecutionHandler
from losttraderbot.portfolio import Portfolio

# create logger, console and add handler
# logger_name = Path(__file__).stem
path_to_file = Path("./logfiles")
path_to_file.mkdir(parents=True, exist_ok=True)
name = path_to_file.joinpath("trader_events.log")
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

class MovingAveragesCrossStrategy(Strategy):
    "Moving averages strategy (MAC) for short / long windows of 100 / 400 periods."

    def __init__(
        self, bars: DataHandler, events: Event, short_window=12, long_window=20
    ):
        self.bars = bars
        self.symbol_list = self.bars.symbol_list
        self.events = events
        self.short_window = short_window
        self.long_window = long_window

        self.bought = self._compute_initial_bought()

    def _compute_initial_bought(self):
        """Add keys to the bought dict for all symbols and set them to 'OUT'"""
        bought = {}
        for symbol in self.symbol_list:
            bought[symbol] = "OUT"

        return bought

    def compute_signals(self, event: MarketEvent):
        """Compute signals based on the MAC"""
        if event.type == "MARKET":
            for symbol in self.symbol_list:
                bars = self.bars._get_latest_bars_values(
                    symbol, "Close", N=self.long_window
                )

            bar_date = self.bars._get_latest_bar_datetime(symbol)
            if bars is not None and len(bars) > 0:
                short_sma = np.mean(bars[-self.short_window :])
                long_sma = np.mean(bars[-self.long_window :])

                dt = datetime.datetime.utcnow()
                sig_dir = ""

                if long_sma < short_sma and self.bought[symbol] == "OUT":
                    logger.info(f"LONG : {bar_date} ")
                    sig_dir = "LONG"
                    signal = SignalEvent(1, symbol, dt, sig_dir, 1.0)
                    self.events.put(signal)
                    self.bought[symbol] = "LONG"

                elif short_sma < long_sma and self.bought[symbol] == "LONG":
                    logger.info(f"SHORT: {bar_date}")
                    sig_dir = "EXIT"
                    signal = SignalEvent(1, symbol, dt, sig_dir, 1.0)
                    self.events.put(signal)
                    self.bought[symbol] = "OUT"


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
        logger.info(f"head of dataset history for {symbol}")
        print(dataset_history.head())
    return filepath


if __name__ == "__main__":
    # TODO: Make this part user dependent
    symbol_list = ["AAPL"]
    csv_dir = [data_scrapper(symbol) for symbol in symbol_list]
    initial_capital = 100_000
    heartbeat = 0.0
    start_date = datetime.datetime(2022, 1, 1, 0, 0, 0)

    backtest = Backtest(
        csv_dir[-1],
        symbol_list,
        initial_capital,
        start_date,
        heartbeat,
        HistoricCSVDataHandler,
        SimulatedExecutionHandler,
        MovingAveragesCrossStrategy,
        Portfolio,
    )
    backtest.simulate_trading()
