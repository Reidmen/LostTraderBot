class Event:
    """Event is base class providing interface for all other events in the trading infrastructure."""

    pass


class MarketEvent(Event):
    """
    Handles the event of receiving a new market update with coorresponding bars.
    """

    def __init__(self):
        """
        Initializes the MarketEvent.
        """
        self.type = "MARKET"


class SignalEvent(Event):
    """
    Handles the event of sending a Signal form a Strategy object.
    This is received by a Portfolio object and acten upon.
    """

    def __init__(
        self,
        strategy_id: str,
        symbol: str,
        datetime: str,
        signal_type: str,
        strenght: str,
    ):
        self.type = "SIGNAL"
        self.strategy_id = strategy_id
        self.symbol = symbol
        self.datetime = datetime
        self.signal_type = signal_type
        self.strength = strength
