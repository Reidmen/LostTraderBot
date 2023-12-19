#include "strategy.hpp"

#include <memory>
#include <string>
#include <unordered_map>

#include "data.hpp"

TradingStrategy::TradingStrategy(
    std::shared_ptr<HistoricCSVDataHandler> dataHandler) {
    this->dataHandler = dataHandler;
    *this->eventQueue = dataHandler->eventQueue;

    std::unordered_map<std::string, bool> bought;

    for (auto symbol : dataHandler->symbols) {
        bought[symbol] = false;
    }

    this->bought = bought;
};

void TradingStrategy::calculateSignals() {
    // harcoded value, change?
    int n = 20;
    for (auto symbol : dataHandler->symbols) {
        int direction = 0;

        std::vector<double> closes;
        closes.reserve(n + 1);

        auto ptr_symbol = std::make_shared<std::string>(symbol);
        auto data = dataHandler->getLatestBars(ptr_symbol, n + 1);
        if (data.size() < n + 1) return;
        for (auto close : data) {
            closes.emplace_back(std::get<3>(close));
        }
    };
};
