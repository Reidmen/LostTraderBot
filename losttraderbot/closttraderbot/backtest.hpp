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
        std::unique_ptr<double> initialCapital;
        bool continueBacktest;
        std::queue<std::shared_ptr<Event>> eventQueue;
        InstantExecutionHandler exchange;
        BasicPortfolio portfolio;
        HistoricCSVDataHandler dataHandler;
        TradingStrategy strategy;

        Backtest(
                std::unique_ptr<std::vector<std::string>> symbols,
                std::unique_ptr<std::string> csvDirectory,
                std::unique_ptr<double> initialCapital
                );

        void run(std::unique_ptr<TradingStrategy>);
};
