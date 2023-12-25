#include "portfolio.hpp"

#include <memory>
#include <string>
#include <vector>

#include "data.hpp"

BasicPortfolio::BasicPortfolio(
    std::shared_ptr<HistoricCSVDataHandler> dataHandler,
    std::shared_ptr<SymbolsType> symbols,
    std::shared_ptr<double> initialCapital) {
    this->dataHandler = dataHandler;
    this->eventQueue = dataHandler->eventQueue;
    this->symbols = symbols;
    this->initialCapital = initialCapital;
    this->allPositions = constructAllPositions();
    this->currentPositions = constructCurrentPositions();
    this->allHoldings = constructAllHoldings();
    this->currentHoldings = constructCurrentHoldings();
}

auto BasicPortfolio::constructAllPositions() -> MapPositionsType {
    PositionsType innerMap;
    for (auto symbol : *symbols) {
        innerMap.insert({symbol, 0.0});
    }
    auto firstTimestamp = dataHandler->bar->first;
    MapPositionsType map;
    map.insert({firstTimestamp, innerMap});
    return map;
}

auto BasicPortfolio::constructCurrentPositions() -> PositionsType {
    PositionsType map;
    for (auto symbol : *symbols) {
        map.insert({symbol, 0.0});
    }
    return map;
}

auto BasicPortfolio::constructAllHoldings() -> MapPositionsType {
    PositionsType innerMap;
    for (auto symbol : *symbols) {
        innerMap.insert({symbol, 0.0});
    }
    innerMap.insert({"cash", *initialCapital});
    innerMap.insert({"commission", 0.0});
    innerMap.insert({"slippage", 0.0});
    innerMap.insert({"total", *initialCapital});
    innerMap.insert({"returns", 0.0});
    innerMap.insert({"equity_curve", 0.0});

    auto firstTimestamp = dataHandler->bar->first;
    MapPositionsType map;
    map.insert({firstTimestamp, innerMap});

    return map;
}

auto BasicPortfolio::constructCurrentHoldings() -> PositionsType {
    PositionsType map;
    for (auto symbol : *symbols) {
        map.insert({symbol, 0.0});
    }
    map.insert({"cash", *initialCapital});
    map.insert({"commission", 0.0});
    map.insert({"slippage", 0.0});
    map.insert({"total", *initialCapital});

    return map;
}
