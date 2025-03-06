/*
    Data Handler Class
    
    
    The DataHandler is responsible for:
    1. Loading historical or live market data
    2. Providing a consistent interface to access this data
    3. Generating MarketEvents when new data is available
    
    This implementation focuses on historical backtesting with CSV data.
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

// Type definitions to improve code readability and maintainability

// Database format <[open, high, low, close, volume]>
// Represents a collection of OHLCV bars
using DatabaseType =
    std::vector<std::tuple<double, double, double, double, double>>;

// Historical data mapping <timestamp, [open, high, low, close, volume]>
// Provides time-indexed access to price bars
using HistoricalDataType =
    std::map<long long, std::tuple<double, double, double, double, double>>;

// Maps symbols to their respective historical data
// Allows handling multiple instruments simultaneously
using SymbolHistoricalDataType =
    std::unordered_map<std::string, HistoricalDataType>;

// Shared pointer types for efficient memory management and object passing
using SharedStringType = std::shared_ptr<std::string>;
using QueueEventType = std::queue<std::shared_ptr<Event>>;
using SharedQueueEventType = std::shared_ptr<QueueEventType>;
using SymbolsType = std::vector<std::string>;
using SharedSymbolsType = std::shared_ptr<SymbolsType>;

/*
 * Abstract DataHandler class that defines the interface for all data handlers
 */
class DataHandler : std::enable_shared_from_this<DataHandler> {
   public:
    std::string csvDirectory;       // Directory containing data files
    SharedQueueEventType eventQueue; // Reference to the system's event queue
    bool* continueBacktest;         // Flag to control backtest execution
    std::vector<std::string> symbols; // Financial instruments being traded

    virtual void loadDataFromMemory() = 0;
    
    // Retrieves the latest n bars for a given symbol
    virtual DatabaseType getLatestBars(SharedStringType symbol, int n = 1) = 0;
    
    virtual void updateBars() = 0;
    
    virtual ~DataHandler() = default;
};

/*
 * Concrete implementation of DataHandler for historical CSV data
 */
class HistoricCSVDataHandler
    : public DataHandler,
      std::enable_shared_from_this<HistoricCSVDataHandler> {
   public:
    // Historical data in format <symbol, <timestamp, [open, high, low, close, volume]>>
    // Complete dataset loaded from CSV
    SymbolHistoricalDataType data;
    
    // Data consumed so far in the simulation
    SymbolHistoricalDataType consumedData;

    // Iterator over the historical data contained in data
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

    // Load data from CSV file into memory
    // This implementation assumes a specific CSV format with columns:
    // timestamp, symbol, exchange, open, high, low, close, adjusted_close, volume
    void loadDataFromMemory() {
        std::ifstream fileToLoad(csvDirectory, std::ios::binary);
        if (!fileToLoad.is_open()) throw std::runtime_error("Could not load file");

        std::string line, lineItems;
        HistoricalDataType innerMap;
        std::getline(fileToLoad, line); // Skip header row

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

    // Returns the 'n' latest bars in format <[open, high, low, close, volume]>
    // This method is used by strategy components to access recent price history
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

    // Pushes the latest bar onto the eventQueue
    // This simulates the arrival of new market data in a live system
    void updateBars() {
        // Add a bar to consumedData if we haven't reached the end
        if (bar != data[symbols[0]].end()) {
            consumedData[symbols[0]][bar->first] = bar->second;
            bar++;
        } 

        // Generate a MarketEvent to notify the system of new data
        eventQueue->push(std::make_shared<MarketEvent>());
    };
};
