#pragma once
#include "event.hpp"
#include "data.hpp"
#include <memory>
#include <queue>
#include <string>
#include <vector>
#include <unordered_map>


class Strategy {
    public:
        // creates signalEvents based on historical data
        virtual void calculateSignals() = 0;
};


class TradingStrategy : Strategy {
    public:
        // pointer to datahandler
        std::shared_ptr<HistoricCSVDataHandler> dataHandler;
        // pointer to event queue
        std::shared_ptr<std::queue<std::shared_ptr<Event>>> eventQueue;
        // flag for assets bought
        std::unordered_map<std::string, bool> bought;

        TradingStrategy(std::shared_ptr<HistoricCSVDataHandler> dataHandler);

        TradingStrategy() = default;

        void calculateSignals();
};
