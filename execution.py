from abc import ABC, abstractmethod
from datetime
import queue

from event import FillEvent, OrderEvent

class ExecutionHandler(ABC):
    """The execution handler abstract class handles the interaction
    between a set of order generated by a Portflio and the ultimate
    set of Fill object that actually occur in the market."""

    @abstractmethod
    def execute_order(self, event: FillEvent):
        """Takes and Order and executes it, producing a Fill event
        that gets placed onto the Events queue."""
        raise NotImplementedError("implement execute_order()")


class SimulatedExecutionHandler(ExecutionHandler):
    """TO BE COMPLETED"""
    def __init__(self, event: FillEvent | OrderEvent):
        """Initializes the handler, setting the event queues up
        internally."""
        self.events = events
    def execute_order(self, event: FillEvent):
        """Simply converts Order object into Fill object."""
        if event.type == "FILL":
            fill_event = FillEvent(
                    datetime.datetime.utcnow(), event.symbol,
                    'ARCA', event.quantity, event.direction, None)
            self.events.put(fill_event)
