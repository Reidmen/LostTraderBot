import datetime
import numpy as np
import pandas as pd

from queue import Queue

from strategy import Strategy
from event import SignalEvent
from backtest import Backtest
from data import DataHandler, HistoricCSVDataHandler
from portfolio import PortfolioIntraFT
from execution import SimulaterExecutionHandler


class OLSMeanReversingStrategy(Strategy):
    def __init__(
        self,
        bars: DataHandler,
        events: Queue[Event],
        ols_window: int = 100,
        zscore_low: float = 0.5,
        zscore_high: float = 3.0,
        symbol_pair: tuple = ("AREX", "WLL"),
    ):
        self.bars = bars
        self.symbol_list = self.bars.symbol_list
        self.events = events
        self.ols_window = ols_window
        self.zscore_low = zscore_low
        self.zscore_high = zscore_high

        self.symbol_pair = symbol_pair
        self.datetime = datetime.datetime.utcnow()

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

        if abs(zscore_last) <= self.zscore_low and self.long_market:
            self.long_market = False
            x_signal = SignalEvent(1, p0, dt, "EXIT", 1.0)
            y_signal = SignalEvent(1, p1, dt, "EXIT", 1.0)

        if zscore_last >= self.zscore_high and not self.short_market:
            self.short_market = True
            x_signal = SignalEvent(1, p0, dt, "SHORT", 1.0)
            y_signal = SignalEvent(1, p1, dt, "LONG", hedge_ratio)

        if abs(zscore_last) <= self.zscore_low and self.short_market:
            self.short_market = False
            x_signal = SignalEvent(1, p0, dt, "EXIT", 1.0)
            y_signal = SignalEvent(1, p1, dt, "EXIT", 1.0)

        return y_signal, x_signal

    def compute_signals_for_pairs(self):
        """Computes a new set of signals based on the mean reversion
        strategy."""
        # Uses OLS for the hedge ration, (try CADF)
        y = self.bars.get_latest_bars_values(
                self.pair[0], "close", N=self.ols_window)
        x = self.bars.get_latest_bars_values(
                self.pair[1], "close", N=self.ols_window)

        if y is not None and x is not None:
            # check if all window periods are available
            if len(y) >= self.ols_window and len(x) >= self.ols_window:
                self.hedge_ratio = sm.OLS(y, x).fit().params[0]

                spread = y - self.hedge_ratio * x
                zscore_last = ((spread - spread.mean()) / spread.std())[-1]

                # compute signals and add to the events queue
                y_signal, x_signal = self.compute_xy_signal(zscore_last)
                if y_signal is not None and x_signal is not None:
                    self.events.put(y_signal)
                    self.events.put(x_signal)

    def compute_signals(self, event: SignalEvent):
        if event.type == "MARKET":
            self.compute_signals_for_pair()
