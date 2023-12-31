from __future__ import annotations
import pathlib
import pandas as pd
import numpy as np
from scipy.sparse import data
from talib import RSI, BBANDS, MACD, WILLR, PPO
import matplotlib.pyplot as plt
import seaborn as sns

START = "2000-01-01"
END = "2023-12-10"
# TODO: add luguru logger


def load_data_from_quandl(
    path_to_store: pathlib.Path = pathlib.Path("datasets", "prices_quandl.h5"),
    path_to_read: pathlib.Path = pathlib.Path(
        "datasets", "wiki_quandl_prices.csv"
    ),
) -> None:
    prices_dataframe = pd.read_csv(
        path_to_read.as_posix(),
        parse_dates=["date"],
        index_col=["date", "ticker"],
        infer_datetime_format=True,
    ).sort_index()
    # print(prices_dataframe.info())
    print(prices_dataframe.describe())
    with pd.HDFStore(path_to_store.as_posix()) as hdf:
        hdf.put("quanld/wiki/prices", prices_dataframe)


def create_training_dataset_with_indicators(
    path_to_data: pathlib.Path = pathlib.Path("datasets", "prices_quandl.h5"),
    adj_ohlcv: list[str] = [
        "adj_open",
        "adj_close",
        "adj_low",
        "adj_high",
        "adj_volume",
    ],
    num_of_stocks: int = 500,
    train_window: int = 5,
    indicators_range: tuple[int, int] = (6, 21),
) -> None:
    index = pd.IndexSlice
    with pd.HDFStore(path_to_data.as_posix()) as hdf:
        prices = (
            hdf["quanld/wiki/prices"]
            .loc[index[START:END, :], adj_ohlcv]
            .rename(columns=lambda x: x.replace("adj_", ""))
            .swaplevel()
            .sort_index()
            .dropna()
        )
        # metadata = hdf["us_equities/stocks"].loc[:, ["marketcap", "sector"]]
        ohlcv_names = prices.columns.tolist()
        normalize_datasets_with_index(prices)
        rolling_universe_of_stocks(prices, num_of_stocks, train_window)
        generate_technical_indicators(prices, indicators_range)
        save_dataset_to_hdf5(prices, ohlcv_names, index)


def explore_dataset_and_generate_figures(
    path_to_data: pathlib.Path = pathlib.Path("datasets", "prices_quandl.h5"),
) -> None:
    idx = pd.IndexSlice
    with pd.HDFStore(path_to_data.as_posix()) as stored_data:
        column_names = [
            "adj_open",
            "adj_high",
            "adj_low",
            "adj_close",
            "adj_volume",
        ]
        dataset = (
            stored_data["quanld/wiki/prices"]
            .loc[idx["2007":"2010", "AAPL"], column_names]
            .unstack("ticker")
            .swaplevel(axis=1)
            .loc[:, "AAPL"]
            .rename(columns=lambda x: x.replace("adj_", ""))
        )
        compute_indicators_for_figures(dataset)


def compute_indicators_for_figures(dataset: pd.DataFrame) -> None:
    macd, macd_signal, macd_hist = MACD(
        dataset.close, fastperiod=12, slowperiod=26, signalperiod=9
    )
    dataset = pd.DataFrame(
        {
            "AAPL": dataset.close,
            "MACD": macd,
            "MACD_SIG": macd_signal,
            "MACD_HIST": macd_hist,
        }
    )
    print(dataset.tail(10))

    fig, axes = plt.subplots(nrows=2, figsize=(15, 8))
    dataset.AAPL.plot(ax=axes[0])
    dataset.drop("AAPL", axis=1).plot(ax=axes[1])
    fig.tight_layout()
    plt.show()
    sns.despine()


def save_dataset_to_hdf5(
    universe: pd.DataFrame,
    drop_names: list[str],
    index,
    pathfile_to_save: pathlib.Path = pathlib.Path(
        "datasets", "processed_data.h5"
    ),
) -> None:
    universe_cleaned = universe.drop(drop_names, axis=1)
    print(universe_cleaned.info(verbose=True))
    universe_cleaned = universe_cleaned.sort_index()
    with pd.HDFStore(pathfile_to_save.as_posix()) as to_store:
        to_store.put("features", universe_cleaned.loc[index[:, :"2005"], :])
        to_store.put("targets", universe_cleaned.loc[index[:, "2005":]])


def generate_technical_indicators(
    universe: pd.DataFrame, ranges: tuple[int, int]
) -> None:
    """generates technical indicators into dataset"""
    assert "close" in universe.columns
    range_values = list(range(ranges[0], ranges[1]))
    for period in range_values:
        universe[f"{period:02}_RSI"] = universe.groupby(
            level="symbol"
        ).close.apply(RSI, timeperiod=period)

        universe[f"{period:02}_WILLR"](
            universe.groupby(level="symbol", group_keys=False).apply(
                lambda x: WILLR(x.high, x.low, x.close, timeperiod=period)
            )
        )

        bbh, bbl = f"{period:02}_BBH", f"{period:02}_BBL"
        universe = universe.join(
            universe.groupby(level="symbol").close.apply(
                compute_bollinger_bands_indicator, timeperiod=period
            )
        )
        universe[bbh] = (
            universe[bbh]
            .sub(universe.close)
            .div(universe[bbh])
            .apply(np.log1p)
        )
        universe[bbl] = (
            universe.close.sub(universe[bbl])
            .div(universe.close)
            .apply(np.log1p)
        )

        universe[f"{period:02}_PPO"] = universe.groupby(
            level="symbol"
        ).close.apply(PPO, fastperiod=period, matype=1)

        universe[f"{period:02}_MACD"] = universe.groupby(
            "symbol", group_keys=False
        ).close.apply(
            compute_moving_average_convergence_divergence_indicator,
            signalperiod=period,
        )


def compute_moving_average_convergence_divergence_indicator(
    close_price: pd.DataFrame, signalperiod: int
) -> pd.DataFrame:
    macd = MACD(close_price, signalperiod=signalperiod)[0]
    print(type(macd))
    print(macd.info())
    return (macd - np.mean(macd)) / np.std(macd)


def compute_bollinger_bands_indicator(
    close_price: pd.DataFrame, timeperiod: int
) -> pd.DataFrame:
    # TODO: check for index column (?)
    high, _, low = BBANDS(close_price, timeperiod=timeperiod)
    return pd.DataFrame(
        {f"{timeperiod:02}_BBH": high, f"{timeperiod:02}_BBL": low},
        index=close_price.index,
    )


def normalize_datasets_with_index(
    prices: pd.DataFrame, metadata: pd.DataFrame | None = None
) -> None:
    if "volume" in prices.columns:
        prices.volume /= 1e3
    prices.index.names = ["symbol", "date"]

    if isinstance(metadata, pd.DataFrame):
        assert metadata is not None
        metadata.index.name = "symbol"


def rolling_universe_of_stocks(
    prices: pd.DataFrame,
    universe_size: int,
    train_window: int,
) -> None:
    """pick num_of_stocks most-traded stocks in dollars volume"""
    assert all(item in prices.columns for item in ["volume", "close"])
    dollar_volume: pd.DataFrame = (
        prices.close.mul(prices.volume).unstack("symbol").sort_index()
    )
    years_to_consider: list[np.ndarray] = sorted(
        np.unique(
            [
                date.year
                for date in prices.index.get_level_values("date").unique()
            ]
        )
    )
    universe = get_universe_of_stocks(
        prices, dollar_volume, years_to_consider, universe_size, train_window
    )
    check_dataset_and_save(
        universe, pathlib.Path("universe_dataset.h5"), "universe"
    )
    print(universe.info(verbose=True))


def check_dataset_and_save(
    universe: pd.DataFrame, path_to_save: pathlib.Path, key_name: str
) -> None:
    print(universe.info())
    print(universe.groupby("symbol").size().describe())
    universe.to_hdf(path_to_save.as_posix(), key=key_name)


def get_universe_of_stocks(
    prices: pd.DataFrame,
    dollar_volume: pd.DataFrame,
    years: list[np.ndarray],
    universe_size: int,
    train_window: int,
) -> pd.DataFrame:
    universe: list[pd.DataFrame] = []
    idx = pd.IndexSlice
    for i, _ in enumerate(years[train_window:], train_window):
        start = str(years[i - train_window])
        end = str(years[i])
        most_traded = (
            dollar_volume.loc[start:end, :]
            .dropna(thresh=1000, axis=1)
            .median()
            .nlargest(universe_size)
            .index
        )
        universe.append(prices.loc[idx[most_traded, start:end], :])

    universe_dataset: pd.DataFrame = pd.concat(universe)
    universe_dataset: pd.DataFrame = universe_dataset[
        ~universe_dataset.index.duplicated()
    ]

    return universe_dataset


if __name__ == "__main__":
    # TODO: include adequate parser
    # load_data_from_quandl()
    # create_training_dataset_with_indicators()
    explore_dataset_and_generate_figures()
