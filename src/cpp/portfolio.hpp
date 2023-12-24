#pragma once
#include <map>
#include <memory>
#include <queue>
#include <string>
#include <unordered_map>
#include <vector>

#include "data.hpp"
#include "event.hpp"

using SharedSignalEventType = std::shared_ptr<SignalEvent>;
using SharedFillEventType = std::shared_ptr<FillEvent>;
using PositionsType = std::unordered_map<std::string, double>;
using MapPositionsType = std::map<long long, PositionsType>;
using MetricsType = std::map<std::string, double>;

class Portfolio {
   public:
    virtual void onSignal(SharedSignalEventType event) = 0;
    virtual void onFill(SharedFillEventType event) = 0;
};

class BasicPortfolio : Portfolio {
   public:
    // pointer to datahandler
    std::unique_ptr<HistoricCSVDataHandler> dataHandler;
    // pointer to queue of Event
    QueueEventType* eventQueue;
    // vector of symbols
    std::vector<std::string> symbols;
    // capital of Portfolio
    std::unique_ptr<double> initialCapital;
    // all positions of the system
    MapPositionsType allPositions;
    // current position
    PositionsType currentPositions;
    // all and current holdings
    MapPositionsType allHoldings;
    PositionsType currentHoldings;
    // performance metrics
    MetricsType performanceMetrics;

    BasicPortfolio(std::unique_ptr<HistoricCSVDataHandler> dataHandler,
                   std::vector<std::string> symbols,
                   std::unique_ptr<double> initialCapital);

    BasicPortfolio() = default;

    // construct allPositions and currentPosition
    auto constructAllPositions() -> MapPositionsType;
    auto constructCurrentPositions() -> PositionsType;

    // construct allHoldings and currentHoldings
    auto constructAllHoldings() -> MapPositionsType;
    auto constructCurrentHoldings() -> PositionsType;

    void update();

    void updatePositionOnFill(SharedFillEventType event);
    void updateHoldingsOnFill(SharedFillEventType event);

    void createOrderonSignal(SharedSignalEventType);
    void createOrderonFill(SharedFillEventType);

    void generateOrder(SharedSignalEventType event);

    auto getMaximumQuantity(SharedSignalEventType event);

    // computes and returns performace metrics
    void getMetrics();
};
