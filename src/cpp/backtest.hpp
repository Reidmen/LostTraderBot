#pragma once
#include <memory>
#include <queue>
#include <string>
#include <vector>

#include "data.hpp"
#include "event.hpp"
#include "execution.hpp"
#include "portfolio.hpp"
#include "strategy.hpp"

class Backtest : std::enable_shared_from_this<Backtest> {
   public:
    SymbolsType symbols;
    SharedStringType csvDirectory;
    std::shared_ptr<double> initialCapital;
    bool continueBacktest;
    SharedQueueEventType eventQueue;
    InstantExecutionHandler exchange;
    BasicPortfolio portfolio;
    HistoricCSVDataHandler dataHandler;
    TradingStrategy strategy;

    Backtest(SharedSymbolsType symbols, SharedStringType csvDirectory,
             std::shared_ptr<double> initialCapital);

    void run(std::shared_ptr<TradingStrategy>);
};
