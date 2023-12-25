#include "portfolio.hpp"

#include <memory>
#include <string>
#include <vector>

#include "data.hpp"

BasicPortfolio::BasicPortfolio(
    std::shared_ptr<HistoricCSVDataHandler> dataHandler,
    std::shared_ptr<SymbolsType> symbols,
    std::shared_ptr<double> initialCapital) {
    this->dataHandler = dataHandler;
    this->eventQueue = dataHandler->eventQueue;
    this->symbols = symbols;
    this->initialCapital = initialCapital;
    this->allPositions = constructAllPositions();
    this->currentPositions = constructCurrentPositions();
    this->allHoldings = constructAllHoldings();
    this->currentHoldings = constructCurrentHoldings();
}
