#include "data.hpp"

#include <fstream>
#include <ios>
#include <map>
#include <memory>
#include <queue>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

HistoricCSVDataHandler::HistoricCSVDataHandler(
    std::queue<std::shared_ptr<Event>> eventQueue,
    std::shared_ptr<std::string> csvDirectory,
    std::shared_ptr<std::vector<std::string>> symbols,
    std::shared_ptr<bool> continueBacktest) {
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
    std::map<long long, std::tuple<double, double, double, double, double>>
        innerMap;

    // skip for row in the data file
    std::getline(fileToLoad, line);

    while (std::getline(fileToLoad, line)) {
        std::stringstream ss(line);
        std::vector<std::string> lineVector;

        while (std::getline(ss, lineItems, ',')) {
            lineVector.emplace_back(lineItems);
        };
        // TODO inser into innerMap
    };
};

void HistoricCSVDataHandler::updateBars(){};
