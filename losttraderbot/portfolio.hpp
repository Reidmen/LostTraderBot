#pragma once
#include "event.hpp"
#include "data.hpp"
#include <memory>
#include <queue>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>


class Portfolio {
    public:
        virtual void onSignal(std::shared_ptr<SignalEvent> event) = 0;
        virtual void onFill(std::shared_ptr<FillEvent> event) = 0;
};

class BasicPortfolio: Portfolio {
    public:
        // pointer to datahandler
        std::unique_ptr<DataHandler> dataHandler;
        // pointer to queue of Event
        std::unique_ptr<std::queue<std::shared_ptr<Event>>> eventQueue;
        // vector of symbols
        std::vector<std::string> symbols;
        // capital of Portfolio
        std::unique_ptr<double> initialCapital;
        // all positions of the system
        std::map<long long, std::unordered_map<std::string, double>> allHoldings;
        // current holdings
        std::unordered_map<std::string, double> performanceMetrics;

        BasicPortfolio(
                std::unique_ptr<DataHandler> dataHandler,
                std::vector<std::string> symbols,
                std::unique_ptr<double> initialCapital
                );

        BasicPortfolio() = default;

        void update();

        void upadtePositionOnFill(
                std::shared_ptr<FillEvent> event);
        void updateHoldingsOnFill(
                std::shared_ptr<FillEvent> event);

        // computes and returns performace metrics
        void getMetrics();
};
