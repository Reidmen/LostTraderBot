import datetime

import numpy as np
import pandas as pd
import statsmodels.api as sm

from strategy import Strategy
from event import SignalEvent
from backtest import Backtest
from data import HistoricCSVDataHandler
from execution import SimulatedExecutionHandler
from portfolio import Portfolio


class MovingAveragesCrossStrategy(Strategy):
    "Moving averages strategy (MAC) for short / long windows of 100 / 400 periods."

    def __init__(
        self, bars: DataHandler, events: Event, short_window=100, long_window=400
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

    def compute_signals(self, event: Event):
        """Compute signals based on the MAC"""
        if event.type == "MARKET":
            for symbol in self.symbol_list:
                bars = self.bars.get_latest_bar_values(
                    symbol, "adj_close", N=self.long_window
                )

            bar_date = self.bars.get_latest_bar_datetime(symbol)
            if bars is not None and len(bar) > 0:
                short_sma = np.mean(bars[-self.short_window :])
                long_sma = np.mean(bars[-self.long_window :])

                dt = datetime.datetime.utcnow()
                sig_dir = ""

                if long_sma < short_sma and self.bought[symbol] == "OUT":
                    print("LONG : ", bar_date)
                    sig_dir = "LONG"
                    signal = SignalEvent(1, symbol, dt, sig_dir, 1.0)
                    self.events.put(signal)
                    self.bought[symbol] = "LONG"

                elif short_sma < long_sma and self.bought[symbol] == "LONG":
                    print("SHORT: ", bar_date)
                    sig_dir = "EXIT"
                    signal = SignalEvent(1, symbol, dt, sig_dir, 1.0)
                    self.events.put(signal)
                    self.bought[symbol] = "OUT"


if __name__ == "__main__":
    # TODO: Make this part user dependent
    symbol_list = ["AAPL"]
    initial_capital = 100_000
    heartbeat = 0.0
    start_date = datetime.datetime(2000, 1, 1, 0, 0, 0)

    backtest = Backtest(
        csv_dir,
        symbol_list,
        initial_capital,
        heartbear,
        start_date,
        HistoricCSVDataHandler,
        SimulatedExecutionHandler,
        Portfolio,
        MovingAveragesCrossStrategy,
    )
    backtest.simulate_trading()
