#pragma once
#include "data.hpp"
#include "event.hpp"
#include "execution.hpp"
#include "portfolio.hpp"
#include "strategy.hpp"
#include <memory>
#include <queue>
#include <string>
#include <vector>

class Backtest {
    public:
        std::vector<std::string> symbols;
        std::string csvDirectory;
        std::shared_ptr<double> initialCapital;
        bool continueBacktest;
        std::queue<std::shared_ptr<Event>> eventQueue;
        InstantExecutionHandler exchange;
        BasicPortfolio portfolio;
        HistoricCSVDataHandler dataHandler;
        TradingStrategy strategy;

        Backtest(
                std::shared_ptr<std::vector<std::string>> symbols,
                std::shared_ptr<std::string> csvDirectory,
                std::shared_ptr<double> initialCapital
                );

        void run(std::shared_ptr<TradingStrategy>);
};
