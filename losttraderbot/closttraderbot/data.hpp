#pragma once
#include <cstddef>
#include <fstream>
#include <memory>
#include <queue>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "event.hpp"

class DataHandler {
   public:
    std::string csvDirectory;
    std::queue<std::shared_ptr<Event>> eventQueue;
    std::shared_ptr<bool> continueBacktest;
    std::vector<std::string> symbols;

    virtual void loadData() = 0;
    virtual std::vector<std::tuple<double, double, double, double, double>>
    getLatestBars(std::unique_ptr<std::string> symbol, int n = 1) = 0;
    virtual void updateBars() = 0;
    virtual ~DataHandler() = default;
};

class HistoricCSVDataHandler : public DataHandler {
   public:
    std::string csvDirectory;

    HistoricCSVDataHandler(std::queue<std::shared_ptr<Event>> eventQueue,
                           std::shared_ptr<std::string> csvDirectory,
                           std::shared_ptr<std::vector<std::string>> symbols,
                           std::shared_ptr<bool> continueBacktest);

    HistoricCSVDataHandler() = default;

    std::vector<std::tuple<double, double, double, double, double>>
    getLatestBars(std::unique_ptr<std::string> symbol, int n = 1);

    void loadData();
    void updateBars();
};
