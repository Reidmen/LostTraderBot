#include "data.hpp"

#include <fstream>
#include <ios>
#include <map>
#include <memory>
#include <queue>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "event.hpp"

HistoricCSVDataHandler::HistoricCSVDataHandler(
    QueueEventType eventQueue, SharedStringType csvDirectory,
    SharedSymbolsType symbols, std::shared_ptr<bool> continueBacktest) {
    this->eventQueue = eventQueue;
    this->csvDirectory = *csvDirectory;
    this->symbols = *symbols;
    this->continueBacktest = continueBacktest;

    loadData();
};

void HistoricCSVDataHandler::loadData() {
    std::ifstream fileToLoad(csvDirectory, std::ios::binary);
    if (!fileToLoad.is_open()) throw std::runtime_error("Could not load file");

    std::string line, lineItems;
    HistoricalDataType innerMap;
    // skip for row in the data file
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
    // initialize iterator over data
    this->bar = data[symbols[0]].begin();

    // initialize  consumed data
    innerMap.clear();
    this->consumedData.insert(std::make_pair(symbols[0], innerMap));
}

DatabaseType HistoricCSVDataHandler::getLatestBars(SharedStringType symbol,
                                                   int n) {
    DatabaseType current_database;
    current_database.reserve(n);

    if (this->consumedData[*symbol].size() < n) return current_database;
    for (auto rit = this->consumedData[*symbol].rbegin();
         n > 0 && rit != this->consumedData[*symbol].rend(); ++rit, --n) {
        current_database.emplace_back(rit->second);
    }

    return current_database;
}

void HistoricCSVDataHandler::updateBars() {
    // add a bar to comsumedData
    if (bar != data[symbols[0]].end()) {
        consumedData[symbols[0]][bar->first] = bar->second;
        bar++;
    } else {
        *continueBacktest = false;
    }

    eventQueue.push(std::make_shared<MarketEvent>());
};
