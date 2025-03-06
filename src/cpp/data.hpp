/*
    Data class
*/
#pragma once
#include <fstream>
#include <map>
#include <memory>
#include <queue>
#include <sstream>
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
    // historical data in format <symbol, <timestamp, [open, high, low, close,
    // volume]>>
    SymbolHistoricalDataType data;
    // data consumed so far in symbol
    SymbolHistoricalDataType consumedData;

    // iterator over the historical data contained in data
    HistoricalDataType::iterator bar;

    HistoricCSVDataHandler(SharedQueueEventType eventQueue,
                           SharedStringType csvDirectory,
                           SharedSymbolsType symbols) {
        this->eventQueue = eventQueue;
        this->csvDirectory = *csvDirectory;
        this->symbols = *symbols;

        loadDataFromMemory();
    };

    HistoricCSVDataHandler() = default;

    // load data from memory
    void loadDataFromMemory() {
        std::ifstream fileToLoad(csvDirectory, std::ios::binary);
        if (!fileToLoad.is_open()) throw std::runtime_error("Could not load file");

        std::string line, lineItems;
        HistoricalDataType innerMap;
        std::getline(fileToLoad, line);

        while (std::getline(fileToLoad, line)) {
            std::stringstream ss(line);
            std::vector<std::string> lineVector;

            while (std::getline(ss, lineItems, ',')) {
                lineVector.emplace_back(lineItems);
            }
            innerMap.insert(
                {std::stoll(lineVector[0]),
                 std::make_tuple(std::stod(lineVector[3]), std::stod(lineVector[4]),
                                 std::stod(lineVector[5]), std::stod(lineVector[6]),
                                 std::stod(lineVector[8]))});
        }

        this->data.insert(std::make_pair(symbols[0], innerMap));
        this->bar = data[symbols[0]].begin();

        innerMap.clear();
        this->consumedData.insert(std::make_pair(symbols[0], innerMap));
    };

    // returns the 'n' latest bar in format <[open, high, low, close, volume]>
    DatabaseType getLatestBars(SharedStringType symbol, int n = 1) {
        DatabaseType current_database;
        current_database.reserve(n);

        if (this->consumedData[*symbol].size() < n) return current_database;
        for (auto rit = this->consumedData[*symbol].rbegin();
             n > 0 && rit != this->consumedData[*symbol].rend(); ++rit, --n) {
            current_database.emplace_back(rit->second);
        }

        return current_database;
    };

    // pushes the latest bar onto the eventQueue
    void updateBars() {
        // add a bar to comsumedData
        if (bar != data[symbols[0]].end()) {
            consumedData[symbols[0]][bar->first] = bar->second;
            bar++;
        } 

        eventQueue->push(std::make_shared<MarketEvent>());
    };
};
