#pragma once
#include <map>
#include <memory>
#include <queue>
#include <string>
#include <unordered_map>
#include <vector>

#include "data.hpp"
#include "event.hpp"
#include "execution.hpp"

using SharedSignalEventType = std::shared_ptr<SignalEvent>;
using SharedFillEventType = std::shared_ptr<FillEvent>;
using SymbolsType = std::vector<std::string>;
using PositionsType = std::unordered_map<std::string, double>;
using MapPositionsType = std::map<long long, PositionsType>;
using MetricsType = std::map<std::string, double>;

class Portfolio : std::enable_shared_from_this<Portfolio> {
   public:
    virtual void onSignal(SharedSignalEventType event) = 0;
    virtual void onFill(SharedFillEventType event) = 0;
};

class BasicPortfolio : Portfolio, std::enable_shared_from_this<BasicPortfolio> {
   public:
    // pointer to datahandler
    SharedHistoricCSVDataHandler dataHandler;
    // pointer to queue of Event
    SharedQueueEventType eventQueue;
    // vector of symbols
    std::shared_ptr<SymbolsType> symbols;
    // capital of Portfolio
    std::shared_ptr<double> initialCapital;
    // all positions of the system
    MapPositionsType allPositions;
    // current position
    PositionsType currentPositions;
    // all and current holdings
    MapPositionsType allHoldings;
    PositionsType currentHoldings;
    // performance metrics
    MetricsType performanceMetrics;

    BasicPortfolio(std::shared_ptr<SymbolsType> symbols,
                   std::shared_ptr<double> initialCapital,
                   SharedHistoricCSVDataHandler dataHandler) {
        this->dataHandler = dataHandler;
        this->eventQueue = dataHandler->eventQueue;
        this->symbols = symbols;
        this->initialCapital = initialCapital;
        this->allPositions = constructAllPositions();
        this->currentPositions = constructCurrentPositions();
        this->allHoldings = constructAllHoldings();
        this->currentHoldings = constructCurrentHoldings();
    };

    BasicPortfolio() = default;

    // construct allPositions and currentPosition
    auto constructAllPositions() -> MapPositionsType {
        PositionsType innerMap;
        for (auto symbol : *symbols) {
            innerMap.insert({symbol, 0.0});
        }
        auto firstTimestamp = dataHandler->bar->first;
        MapPositionsType map;
        map.insert({firstTimestamp, innerMap});
        return map;
    };

    auto constructCurrentPositions() -> PositionsType {
        PositionsType map;
        for (auto symbol : *symbols) {
            map.insert({symbol, 0.0});
        }
        return map;
    };

    // construct allHoldings and currentHoldings
    auto constructAllHoldings() -> MapPositionsType {
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
    };

    auto constructCurrentHoldings() -> PositionsType {
        PositionsType map;
        for (auto symbol : *symbols) {
            map.insert({symbol, 0.0});
        }
        map.insert({"cash", *initialCapital});
        map.insert({"commission", 0.0});
        map.insert({"slippage", 0.0});
        map.insert({"total", *initialCapital});
        return map;
    };

    void update() {
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
    };

    void onSignal(SharedSignalEventType event) {
        generateOrder(event);
    };
    void onFill(SharedFillEventType event) {
        updatePositionOnFill(event);
        updateHoldingsOnFill(event);
    };

    void updatePositionOnFill(SharedFillEventType event) {
        int direction = 0;
        if (event->direction == "LONG") {
            direction = 1;
        } else if (event->direction == "SHORT") {
            direction = -1;
        }
        currentPositions[event->symbol] += direction * event->quantity;
    };

    void updateHoldingsOnFill(SharedFillEventType event) {
        int direction = 0;
        if (event->direction == "LONG") {
            direction = 1;
        } else if (event->direction == "SHORT") {
            direction = -1;
        }

        auto price = std::get<3>(
            dataHandler->consumedData.at(event->symbol).rbegin()->second);
        auto cost = direction * event->quantity * price;

        currentHoldings[event->symbol] += cost;
        currentHoldings["total"] -= (event->commission + event->slippage);
        currentHoldings["cash"] -= (cost + event->commission + event->slippage);

        currentHoldings["commission"] += event->commission;
        currentHoldings["slippage"] += event->slippage;
    };

    void createOrderonSignal(SharedSignalEventType);
    void createOrderonFill(SharedFillEventType);

    void generateOrder(SharedSignalEventType event) {
        float quantity = 1.0;
        std::string direction;{
        if (event->signal > 0) {
            direction = "LONG";
        } else if (event->signal < 0) {
            direction = "SHORT";
        }
        eventQueue->push(std::make_shared<OrderEvent>(
            &event->symbol, "MARKET", &quantity, &direction, event->target));
        }
    };

    auto getMaximumQuantity(SharedSignalEventType event);

    // computes and returns performace metrics
    void getMetrics();
};