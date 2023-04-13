'''
Author: Reidmen Arostica <r.rethmawn@gmail.com>
Date: 04-09-2023
'''
from abc import ABC, abstractmethod
from typing import List
from pathlib import Path
import datetime
import os, os.path

import numpy as np
import pandas as pd

from event import Event, MarketEvent


class DataHandler(ABC):
    """
    DataHandler is an abstract base class providing an interface for all subsequent (inherited)
    data handlers (both live and historic)

    The goal of a (derived) DataHandler object is to output a generated set of bars for
    each symbol.
    """

    @abstractmethod
    def _get_latest_bar(self, symbol: str):
        """Returns the last updated bar."""
        raise NotImplementedError("Implement get_latest_bar")

    @abstractmethod
    def _get_latest_bars(self, symbol: str, N: int = 1):
        """Returns the last N updated bars."""
        raise NotImplementedError("Implement get_latest_bars")

    @abstractmethod
    def _get_latest_bar_datetime(self, symbol: str):
        """Returns a Python datetime object for the last bar."""
        raise NotImplementedError("Implement get_lastest_bar_datetime")

    @abstractmethod
    def _get_latest_bar_value(self, symbol: str, value_type: float):
        """Returns one of the Open, High, Low, Close, Volume or OI
        form the last bar.
        """
        raise NotImplementedError("Implement get_latest_bar_value")

    @abstractmethod
    def _get_latest_bars_values(self, symbol: str, value_type: float, N: int = 1):
        """Retunrs the last N bar values from the latest symbol list,
        or N-k if less available."""
        raise NotImplementedError("Implement get_latest_bars_values")

    @abstractmethod
    def _update_bars(self):
        """Puches the latest bars to the bars_squeue for each symbol
        in the tuple OHLCVI format:
        (datetime, open, high, low, close, volume, open interest).
        """
        raise NotImplementedError("Implement update_bars")


class HistoricCSVDataHandler(DataHandler):
    """
    HistoricCSVDataHandler is designed to read CSV files for each
    requested symbol from disk and provide an interface to obtain the
    latest bar, analogous to a live interface.
    """

    def __init__(self, events: List[Event], csv_dir: str, symbol_list: str):
        """Initializes the historic data handler"""
        self.events = events
        self.csv_dir = csv_dir
        self.symbol_list = symbol_list

        self.symbol_data = {}
        self.latest_symbol_data = {}
        self._continue_backtest = True

        self._open_convert_csv()

    def _open_convert_csv(self):
        """Here we assume the data is taken from yfinance"""
        # TODO Include different sources
        # it requires reformatting the database
        comb_index = None
        for s in self.symbol_list:
            try:
                self.symbol_data[s] = pd.io.parsers.read_csv(
                    str(Path(self.csv_dir)), parse_dates=True, index_col="Datetime"
                )
            except ValueError:
                raise Exception("index column should have 'Datetime' format")

            print(self.symbol_data[s].head())

            if comb_index is None:
                comb_index = self.symbol_data[s].index

            else:
                comb_index.union(self.symbol_data[s].index)

            self.latest_symbol_data[s] = []

        for s in self.symbol_list:
            self.symbol_data[s] = (
                self.symbol_data[s].reindex(index=comb_index, method="pad").iterrows()
            )

    def _get_new_bar(self, symbol):
        """Returns the latest bar from the data feed."""
        for b in self.symbol_data[symbol]:
            yield b

    def _get_latest_bar(self, symbol):
        """Returns the last bar from the latest symbol list."""
        try:
            bars_list = self.latest_symbol_data[symbol]
        except KeyError:
            raise Exception("The symbol is not available in the historical dataset.")
        else:
            return bars_list[-1]

    def _get_latest_bars(self, symbol, N=1):
        """Returns the last N bars from the latest_symbol list or
        N-k if less available."""
        try:
            bars_list = self.latest_symbol_data[symbol]
        except KeyError:
            raise Exception("The symbol is not avaialble in the historical dataset.")
        else:
            return bars_list[-N:]

    def _get_latest_bar_datetime(self, symbol: str):
        """Returns a python datetime object for the last bar."""
        try:
            bars_list = self.latest_symbol_data[symbol]
        except KeyError:
            raise Exception("The symbol is not available int he historical dataset.")
        else:
            return bars_list[-1][0]

    def _get_latest_bar_value(self, symbol: str, value_type):
        """Returns one of the Open, High, Low, Close, Volume or
        IO value from the pandas Bar series object."""
        try:
            bars_list = self.latest_symbol_data[symbol]
        except KeyError:
            raise Exception("The symbol is not available in the historical dataset.")
        else:
            return getattr(bars_list[-1][1], value_type)

    def _get_latest_bars_values(self, symbol, value_type, N=1):
        """Returns the last N bar values from the latest_symbol list
        or N-k if less available."""
        try:
            bars_list = self._get_latest_bars(symbol, N)
        except KeyError:
            raise Exception("The symbol is not available in the historical dataset.")
        else:
            return np.array([getattr(bar[1], value_type) for bar in bars_list])

    def _update_bars(self):
        """ "Pushes the latest bar to the latest symbol data structure."""
        for symbol in self.symbol_list:
            try:
                bar = next(self._get_new_bar(symbol))
            except StopIteration:
                self._continue_backtest = False
            else:
                if bar is not None:
                    self.latest_symbol_data[symbol].append(bar)

        self.events.put(MarketEvent())
