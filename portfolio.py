import datetime
from math import floor
import queue

import numpy as np
import pandas as pd

from event import FillEvent, OrderEvent

# from performance import create_sharpe_ratio


class Portfolio:
    """Portfolio class handles the positions and market value
    of all instruments at a resolution of a 'bar'.

    The positions pd.DataFrame stores a time-index of the
    quantity of positions held.
    """

    def __init__(
        self, bars: DataHandler, events: Event, start_date: str, initial_capital: float
    ):
        """
        Parameters
            bars: The dataHandler object with current market data
            events: The Event queue object
            start_date: The start bar of the portfolio
            initial_capital: in USD
        """
        self.bars = bars
        self.events = events
        self.symbol_list = self.bar.symbol_list
        self.start_date = start_date
        self.initial_capital = initial_capital

        self.all_positions = self.construct_all_positions()

        self.current_positions = dict(
            (k, v) for k, v in [(s, 0) for s in self.symbol_list]
        )

        self.all_holdings = self.construct_all_holdings()
        self.current_holdings = self.construct_current_holdings

    def construct_all_positions(self):
        """Constructs the positions list using the start_date
        to determine the start index."""
        symbols_tuple = [(s, 0) for s in self.symbol_list]
        positions = dict((k, v) for k, v in symbols_tuple)
        positions["datetime"] = self.start_date

        return [positions]

    def construct_all_holdings(self):
        """Constructs the holdings list using the start_date."""
        symbols_tuple = [(s, 0.0) for s in self.symbol_list]
        holdings = dict((k, v) for k, v in symbols_tuple)
        holdings["timedate"] = self.start_date
        holdings["cash"] = self.initial_capital
        holdings["commission"] = 0.0
        holdings["total"] = self.initial_capital

        return [holdings]

    def construct_current_holdings(self):
        """Contruct dictionary holding the values over all symbols."""
        symbols_tuple = [(s, 0.0) for s in self.symbol_list]
        holdings = dict((k, v) for k, v in symbols_tuple)
        holdings["cash"] = self.initial_capital
        holdings["commission"] = 0.0
        holdings["total"] = self.initial_capital

        return [holdings]
