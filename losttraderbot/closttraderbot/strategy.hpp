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
        virtual void calculateSignals() = 0;
};


class TradingStrategy : Strategy {
    public:
        std::unique_ptr<DataHandler> dataHandler;
        std::unique_ptr<std::queue<std::shared_ptr<Event>>> eventQueue;
        std::unordered_map<std::string, bool> bought;

        TradingStrategy(std::unique_ptr<DataHandler> dataHandler);

        TradingStrategy() = default;

        void calculateSignals();
};
