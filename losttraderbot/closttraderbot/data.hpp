#pragma once
#include <cstddef>
#include <fstream>
#include <map>
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
    getLatestBars(std::shared_ptr<std::string> symbol, int n = 1) = 0;
    virtual void updateBars() = 0;
    virtual ~DataHandler() = default;
};

class HistoricCSVDataHandler : public DataHandler {
   public:
    std::string csvDirectory;

    // historical data in format <symbol, <timestamp, [open, high, low, close,
    // volume]>>
    std::unordered_map<
        std::string,
        std::map<long long, std::tuple<double, double, double, double, double>>>
        data;
    // data consumed so far in symbol
    std::unordered_map<
        std::string,
        std::map<long long, std::tuple<double, double, double, double, double>>>
        consumedData;

    // iterator over the historical data contained in data
    std::map<long long, std::tuple<double, double, double, double, double>>::iterator bar;

    HistoricCSVDataHandler(std::queue<std::shared_ptr<Event>> eventQueue,
                           std::shared_ptr<std::string> csvDirectory,
                           std::shared_ptr<std::vector<std::string>> symbols,
                           std::shared_ptr<bool> continueBacktest);

    HistoricCSVDataHandler() = default;

    // retunrs the 'n' latest bar in format <[open, high, low, close, volume]>
    std::vector<std::tuple<double, double, double, double, double>>
    getLatestBars(std::unique_ptr<std::string> symbol, int n = 1);

    // format and load the data into memory
    void loadData();
    // pushes the latest bar onto the eventQueue
    void updateBars();
};
