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
    HistoricCSVDataHandler* dataHandler;
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
                   HistoricCSVDataHandler* dataHandler);

    BasicPortfolio() = default;

    // construct allPositions and currentPosition
    auto constructAllPositions() -> MapPositionsType;
    auto constructCurrentPositions() -> PositionsType;

    // construct allHoldings and currentHoldings
    auto constructAllHoldings() -> MapPositionsType;
    auto constructCurrentHoldings() -> PositionsType;

    void update();

    void onSignal(SharedSignalEventType event);
    void onFill(SharedFillEventType event);

    void updatePositionOnFill(SharedFillEventType event);
    void updateHoldingsOnFill(SharedFillEventType event);

    void createOrderonSignal(SharedSignalEventType);
    void createOrderonFill(SharedFillEventType);

    void generateOrder(SharedSignalEventType event);

    auto getMaximumQuantity(SharedSignalEventType event);

    // computes and returns performace metrics
    void getMetrics();
};
