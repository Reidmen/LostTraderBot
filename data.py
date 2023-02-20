from abc import ABC, abstractmethod

import datetime
import os, os.path

import numpy as np
import pandas as pd

from event import MarketEvent


class DataHandler(ABC):
    """
    DataHandler is an abstract base class providing an interface for all subsequent (inherited)
    data handlers (both live and historic)

    The goal of a (derived) DataHandler object is to output a generated set of bars for
    each symbol.
    """

    @abstractmethod
    def get_latest_bar(self, symbol: str):
        """Returns the last updated bar."""
        raise NotImplementedError("Implement get_latest_bar")

    @abstractmethod
    def get_latest_bars(self, symbol: str, N: int = 1):
        """Returns the last N updated bars."""
        raise NotImplementedError("Implement get_latest_bars")

    @abstractmethod
    def get_latest_bar_datetime(self, symbol: str):
        """Returns a Python datetime object for the last bar."""
        raise NotImplementedError("Implement get_lastest_bar_datetime")

    @abstractmethod
    def get_latest_bar_value(self, symbol: str, value_type: float):
        """Returns one of the Open, High, Low, Close, Volume or OI
        form the last bar.
        """
        raise NotImplementedError("Implement get_latest_bar_value")

    @abstractmethod
    def get_latest_bars_values(self, symbol: str, value_type: float, N: int = 1):
        """Retunrs the last N bar values from the latest symbol list,
        or N-k if less available."""
        raise NotImplementedError("Implement get_latest_bars_values")
