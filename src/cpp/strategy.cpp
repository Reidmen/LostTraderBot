#include "strategy.hpp"

#include <ta-lib/ta_defs.h>
#include <ta-lib/ta_libc.h>

#include <memory>
#include <string>
#include <unordered_map>

#include "data.hpp"
#include "event.hpp"

TradingStrategy::TradingStrategy(
    std::shared_ptr<HistoricCSVDataHandler> dataHandler) {
    this->dataHandler = dataHandler;
    this->eventQueue = dataHandler->eventQueue;

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

        //  get close prices per symbol
        auto ptr_symbol = std::make_shared<std::string>(symbol);
        auto data = dataHandler->getLatestBars(ptr_symbol, n + 1);
        if (data.size() < n + 1) return;
        for (auto close : data) {
            closes.emplace_back(std::get<3>(close));
        }

        double rsi = 0;
        int out_beg;
        int out_number_of_element;

        auto rsi_result = TA_RSI(n, n, closes.data(), n, &out_beg,
                                 &out_number_of_element, &rsi);

        if (rsi_result != TA_SUCCESS) return;

        if (rsi > 70) {
            direction = -1;
        } else if (rsi < 30) {
            direction = 1;
        }

        if (direction != 0 && ((direction == 1 && !bought[symbol]) ||
                               (direction == -1 && bought[symbol]))) {
            auto timestamp =
                dataHandler->consumedData.at(symbol).begin()->first;
            eventQueue->push(std::make_shared<SignalEvent>(
                &symbol, &timestamp, 1.0 * direction, "ALGORITHM"));
            bought[symbol] = !bought[symbol];
        }
    };
};
