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

// database format <[open, high, low, close, volume]>
using DatabaseType =
    std::vector<std::tuple<double, double, double, double, double>>;
// historical data  <timestamp, [open, high, low, close, volume]>
using HistoricalDataType =
    std::map<long long, std::tuple<double, double, double, double, double>>;
// Historical data with symbol
using SymbolHistoricalDataType =
    std::unordered_map<std::string, HistoricalDataType>;
using SharedStringType = std::shared_ptr<std::string>;
using QueueEventType = std::queue<std::shared_ptr<Event>>;
using SharedQueueEventType = std::shared_ptr<QueueEventType>;
using SymbolsType = std::vector<std::string>;
using SharedSymbolsType = std::shared_ptr<SymbolsType>;

class DataHandler : std::enable_shared_from_this<DataHandler> {
   public:
    std::string csvDirectory;
    SharedQueueEventType eventQueue;
    bool* continueBacktest;
    std::vector<std::string> symbols;

    virtual void loadData() = 0;
    virtual DatabaseType getLatestBars(SharedStringType symbol, int n = 1) = 0;
    virtual void updateBars() = 0;
    virtual ~DataHandler() = default;
};

class HistoricCSVDataHandler
    : public DataHandler,
      std::enable_shared_from_this<HistoricCSVDataHandler> {
   public:
    std::string csvDirectory;

    // historical data in format <symbol, <timestamp, [open, high, low, close,
    // volume]>>
    SymbolHistoricalDataType data;
    // data consumed so far in symbol
    SymbolHistoricalDataType consumedData;

    // iterator over the historical data contained in data
    HistoricalDataType::iterator bar;

    HistoricCSVDataHandler(SharedQueueEventType eventQueue,
                           SharedStringType csvDirectory,
                           SharedSymbolsType symbols, bool* continueBacktest);

    HistoricCSVDataHandler() = default;

    // retunrs the 'n' latest bar in format <[open, high, low, close, volume]>
    DatabaseType getLatestBars(SharedStringType symbol, int n = 1);

    // format and load the data into memory
    void loadData();
    // pushes the latest bar onto the eventQueue
    void updateBars();
};
