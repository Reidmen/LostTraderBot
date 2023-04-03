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
        strength: str,
    ):
        self.type = "SIGNAL"
        self.strategy_id = strategy_id
        self.symbol = symbol
        self.datetime = datetime
        self.signal_type = signal_type
        self.strength = strength


class OrderEvent(Event):
    """
    Handles the event of sending an Order to an execution system.
    The order contains a symbol (e.g. GOOG), a type (market or limit),
    quantity and a direction.
    """

    def __init__(self, symbol: str, order_type: str, quantity: int, direction: str):
        self.type = "ORDER"
        self.symbol = symbol
        self.order_type = order_type
        self.quantity = quantity
        self.direction = direction

    def __repr__(self):
        """String representation of the OrderEvent"""
        return "Order: Symbol={}, Type={}, Quantity={}, Direction={}".format(
            self.symbol, self.type, self.quantity, self.direction
        )


class FillEvent(Event):
    """Encapsulated a filled order, as returned from a brokerage.
    Stores the quantity of an instrument actually filled and at what price and
    stores the commission of the trade from the brokerage.
    """

    def __init__(
        self,
        timeindex: str,
        symbol: str,
        exchange: str,
        quantity: int,
        direction: str,
        fill_cost: float,
        commission: float = None,
    ):
        self.type = "FILL"
        self.timeindex = timeindex
        self.symbol = symbol
        self.exchange = exchange
        self.direction = direction
        self.fill_costs = fill_costs
        if commission is None:
            self.commission = self.compute_ib_commission()
        else:
            self.commission = commission

    # TODO: check commission on interactive brokers and other providers
    def compute_ib_commission(self):
        """Computes the fees of trading based on an Interactive Brokers fee structure
        for API, in USD.

        It does not include exchange or ECN fees.
        """
        full_cost = 1.3
        if self.quantity <= 500:
            full_cost = max(1.3, 0.013 * self.quantity)
        else:
            full_cost = max(1.3, 0.008 * self.quantity)

        return full_cost
