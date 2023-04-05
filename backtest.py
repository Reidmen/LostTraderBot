import queue
import time
from datetime import date
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
        heartbeat: float,
        data_handler_object: DataHandler,
        execution_handler: ExecutionHandler,
        strategy: Strategy,
        portfolio: Portfolio,
    ):
        self.csv_directory = csv_directory
        self.symbol_list = symbol_list
        self.initial_capital = initial_capital
        self.heartbeat = heartbeat
        self.start_date = start_date
        self.events = queue.Queue()

        self._generate_trading_instances(
            data_handler_object, execution_handler, strategy, portfolio
        )
        self.signals = 0
        self.orders = 0
        self.fills = 0
        self.number_of_strategies = 1

    def _generate_trading_instances(
        self,
        data_handler_object: DataHandler,
        execution_handler: ExecutionHandler,
        strategy: Strategy,
        portfolio: Portfolio,
    ):
        """Creates trading instances."""
        print("Creating DataHandler, ExecutionHandler, Strategy and Portfolio")
        self.data_handler = data_handler_object(
            self.events, self.csv_directory, self.symbol_list
        )
        self.strategy = strategy(self.data_handler, self.events)
        self.portfolio = portfolio(
            self.data_handler, self.events, self.start_date, self.initial_capital
        )
        self.execution_handler = execution_handler(self.events)

    def _run_backtest(self):
        """Executes the backtest."""
        i = 0
        while True:
            if not self.data_handler._continue_backtest:
                break
            i += 1
            print(f"Current backtest iteration {i}")
            # update the market bars
            self.data_handler._update_bars()

            # handler events
            while True:
                try:
                    event = self.events.get(False)
                except queue.Empty:
                    break
                else:
                    if event is not None:
                        if event.type == "MARKET":
                            self.strategy.compute_signals(event)
                            self.portfolio.update_timeindex(event)
                        elif event.type == "SIGNAL":
                            self.signals += 1
                            self.portfolio.update_signal(event)
                        elif event.type == "ORDER":
                            self.orders += 1
                            self.execution_handler.execute_order(event)
                        elif event.type == "FILL":
                            self.fills += 1
                            self.portfolio.update_using_fill_event(event)

            time.sleep(self.heartbeat)

    def _output_performance(self):
        """Outputs the strategy performance from the backtest."""
        self.portfolio.create_equity_curve_dataframe()
        print("Creating summary...")
        stats = self.portfolio.output_summary_with_statistics()
        print(stats)

        print("Creating equity curve")
        print(self.portfolio.equity_curve.tail(10))

        print(f"Signals: {self.signals}")
        print(f"Orders: {self.orders}")
        print(f"Fills: {self.fills}")

    def simulate_trading(self):
        """Simulates backend and outputs performace."""
        self._run_backtest()
        self._output_performance()
