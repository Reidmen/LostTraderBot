from abc import ABC, abstractmethod
import sys

if sys.version < '3.8':
    raise Exception(f"Upgrade Python, found {sys.version}")

import datetime
import queue

import numpy as np
import pandas as pd

from event import SignalEvent


class Strategy(ABC):
    """Strategy is an abstract base class providing an interface for
    all subsequent (inherited) strategy objects.

    The goal of a strategy object is to generate a Signal object
    for particular symbols based on the inputs of Bars (OHLCV) generated
    by a DataHandler object.

    Its designed for historic and live data since (inherited) strategies
    are agnostic from the data source.
    """

    @abstractmethod
    def compute_signals(self):
        """Computes the mechanics for a list of singals."""
        raise NotImplementedError("Must implement compute_signals()")
