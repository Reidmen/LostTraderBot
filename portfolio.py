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

        return holdings

    def update_timeindex(self, event: Event):
        """Adds a new record to the positions matrix for the current
        market bar. This reflects the PREVIOUS bar, i.e. all current
        market data at this stage is known (OHLCV).
        """
        latest_datetime = self.bars.get_latest_bar_datetime(self.symbol_list[0])
        # update positions
        positions = dict((k, v) for k, v in [(s, 0) for s in self.symbol_list])
        positions["datetime"] = latest_datetime

        for symbol in self.symbol_list:
            positions[symbol] = self.current_positions[symbol]

        self.all_positions.append(positions)

        # update holdings
        holdings = dict((k, v) for k, v in [(s, 0) for s in self.symbol_list])
        holdings["datetime"] = latest_datetime
        holdings["cash"] = self.current_holdings["cash"]
        holdings["commission"] = self.current_holdings["commission"]
        holdings["total"] = self.current_holdings["cash"]

        for symbol in self.symbol_list:
            market_value = self.current_positions[
                symbol
            ] * self.bars.get_latest_bar_value(s, "adj_close")
            holdings[symbol] = market_value
            holdings["total"] += market_value

        self.all_holdings.append(holdings)

    def update_positions_from_fill(self, fill: FillEvent):
        """Takes a Fill object and updates the position martix to
        reflect the new position."""
        fill_direction = 0
        if fill.direction == "BUY":
            fill_direction = 1
        if fill.direction == "SELL":
            fill_direction = -1

        self.current_positions[fill.symbol] += fill_direction + fill.quantity

    def update_holdings_from_fill(self, fill: FillEvent):
        """Takes a Fill object and updates the holdings matrix to
        reflect the holdings value."""
        fill_direction = 0
        if fill.direction == "BUY":
            fill_direction = 1
        if fill.direction == "SELL":
            fill_direction = -1

        fill_cost = self.bars.get_latest_bar_value(fill.symbol, "adj_close")
        cost = fill_direction * fill_cost * fill.quantity
        self.current_holdings[fill.symbol] += cost
        self.current_holdings["commission"] += fill.commission
        self.current_holdings["cash"] -= (cost + fill.commission)
        self.current_holdings["total"] -= (cost + fill.commission)

    def update_using_fill_event(self, event: Event):
        """Updates the portfolio current positions and holdings
        using the fill event"""
        if event.type == "FILL":
            self.update_positions_from_fill(event)
            self.update_holdings_from_fill(event)
