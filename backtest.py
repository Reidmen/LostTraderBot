import datetime
import queue
import time
from datime import date
from data import DataHandler
from execution import ExecutionHandler
from strategy import Strategy
from portfolio import Portfolio


class Backtest:
    """Encapsulates the settings and components for carrying
    out an event-driven backtest."""

    def __init__(
        self,
        csv_directory: str,
        symbol_list: list,
        initial_capital: float,
        start_date: date,
        heartbeat: int,
        data_handler: DataHandler,
        execution_handler: ExecutionHandler,
        strategy: Strategy,
        portfolio: Portfolio,
    ):
        self.csv_directory = csv_directory
        self.symbol_list = symbol_list
        self.initial_capital = initial_capital
        self.heartbeat = heartbeat
        self.start_date = start_date

        self._generate_trading_instances(
            data_handler, execution_handler, strategy, portfolio
        )
        # self.data_handler = data_handler
        # self.execution_handler = execution_handler
        # self.strategy = strategy
        # self.portflio = portfolio

        self.events = queue.Queue()

        self.signals = 0
        self.orders = 0
        self.fills = 0
        self.number_of_strategies = 1
        self._generate_trading_instances()

    def _generate_trading_instances(
        self,
        data_handler: DataHandler,
        execution_handler: ExecutionHandler,
        strategy: Strategy,
        portfolio: Portfolio,
    ):
        """Creates trading instances."""
        print("Creating DataHandler, ExecutionHandler, Strategy and Portfolio")
        self.data_handler = data_handler(self.events, self.csv_directory)
        self.strategy = strategy(self.data_handler, self.events)
        self.portfolio = portfolio(
            self.data_handler, self.events, self.start_date, self.initial_capital
        )
        self.execution_handler = execution_handler(self.events)
