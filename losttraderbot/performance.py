"""
Author: Reidmen Arostica <r.rethmawn@gmail.com>
Date: 04-09-2023
"""
import numpy as np
import pandas as pd


def create_sharpe_ratio(returns: pd.Series, periods: float = 252) -> np.ndarray:
    """Create the Sharpe Ratio for the strategy, based on a benchmark
    of no risk (i.e. no risk-free rate information)"""
    return np.sqrt(periods) * (np.mean(returns)) / np.std(returns)


def create_drawdowns(pnl: pd.Series) -> tuple:
    """Computes the largest peak-to-trough drawdown of the PnL curve
    as well as the duration of the drawdown."""
    assert isinstance(pnl, pd.Series), "requires to be pandas.Series"
    # compute the cumulative returns curve and set up
    # the High-water-mark
    hwm = [0]
    # create drawdown
    idx: pd.Index = pnl.index
    drawdown: pd.Series = pd.Series(index=idx, dtype=np.float64)
    duration: pd.Series = pd.Series(index=idx, dtype=np.float64)

    for t in range(1, len(idx)):
        hwm.append(max(hwm[-1], pnl[t]))
        drawdown[t] = hwm[t] - pnl[t]
        duration[t] = 0 if drawdown[t] == 0 else duration[t - 1] + 1

    return drawdown, drawdown.max(), duration.max()
