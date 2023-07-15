#include "strategy.hpp"

#include <memory>
#include <string>
#include <unordered_map>

#include "data.hpp"

TradingStrategy::TradingStrategy(
    std::shared_ptr<HistoricCSVDataHandler> dataHandler) {
    this->dataHandler = dataHandler;
    this->eventQueue = dataHandler->eventQueue;

    std::unordered_map<std::string, bool> bought;

    for (auto symbol : dataHandler->symbols) {
        bought[symbol] = false;
    }
};
