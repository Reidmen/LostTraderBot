#include "backtest.hpp"

#include <iostream>
#include <memory>
#include <vector>

#include "data.hpp"
#include "event.hpp"
#include "execution.hpp"
#include "strategy.hpp"

Backtest::Backtest(SharedSymbolsType symbols, SharedStringType csvDirectory,
                   std::shared_ptr<double> initialCapital)
    : exchange(&eventQueue, &dataHandler) {
    this->symbols = *symbols;
    this->csvDirectory = *csvDirectory;
    this->initialCapital = initialCapital;
    this->continueBacktest = false;
    this->dataHandler = HistoricCSVDataHandler(eventQueue, csvDirectory,
                                               symbols, continueBacktest);
};

void Backtest::run(std::shared_ptr<TradingStrategy> strategy) {
    continueBacktest = true;

    dataHandler.loadData();
    dataHandler.updateBars();

    std::cout << "Starting backtesting..." << std::endl;

    while (continueBacktest) {
        while (!eventQueue.empty()) {
            auto event = eventQueue.front();
            eventQueue.pop();

            switch (event->type) {
                case 0: {
                    strategy->calculateSignals();
                    portfolio.update();
                    break;
                }
                case 1: {
                    auto signal = std::dynamic_pointer_cast(event);
                    portfolio.onSignal(signal);
                }
                case 2: {
                    auto order = std::dynamic_pointer_cast(event);
                    exchange.executeOrder(order);
                    // order->logOrder()
                    break;
                }
            }
        }

        dataHandler.updateBars();
    }

    std::cout << "Backtest ended\n" << std::endl;
    std::cout << "Performance metrics" << std::endl;
    portfolio.getMetrics();
}
