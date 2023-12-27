#include "portfolio.hpp"

#include <ctime>
#include <memory>
#include <string>
#include <tuple>
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

void BasicPortfolio::update() {
    float notCash = 0.0;
    auto prevTotal = allHoldings.rbegin()->second["total"];
    auto prevEquityCurve = allHoldings.rbegin()->second["equity_curve"];
    auto symbol_to_use = (*symbols)[0];
    auto timestamp = dataHandler->consumedData[symbol_to_use].rbegin()->first;
    for (auto symbol : *symbols) {
        allPositions[timestamp][symbol] = currentPositions[symbol];
        auto price =
            std::get<3>(dataHandler->consumedData[symbol].rbegin()->second);
        auto currentValue = currentPositions[symbol] * price;
        allHoldings[timestamp][symbol] = currentValue;
        currentHoldings[symbol] = currentValue;
        notCash += currentValue;
    }

    currentHoldings["total"] = currentHoldings["cash"] + notCash;
    allHoldings[timestamp]["total"] = currentHoldings["total"];
    allHoldings[timestamp]["cash"] = currentHoldings["cash"];
    allHoldings[timestamp]["commission"] = currentHoldings["commission"];
    allHoldings[timestamp]["slippage"] = currentHoldings["slippage"];

    if (allHoldings.size() > 1) {
        auto returns = (allHoldings[timestamp]["total"] / prevTotal) - 1;
        allHoldings[timestamp]["returns"] = returns;
        allHoldings[timestamp]["equity_curve"] =
            (prevEquityCurve + 1) * (returns + 1) - 1;
    }
}

void BasicPortfolio::updatePositionOnFill(SharedFillEventType fill_event) {
    int direction = 0;

    if (fill_event->direction == "LONG") {
        direction = 1;
    } else if (fill_event->direction == "SHORT") {
        direction = -1;
    }

    currentPositions[fill_event->symbol] += direction * fill_event->quantity;
}
