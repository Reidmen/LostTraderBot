#pragma once
#include <memory>
#include <queue>
#include <string>
#include <unordered_map>
#include <vector>

#include "data.hpp"
#include "event.hpp"

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
