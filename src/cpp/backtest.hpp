/*
    Backtest class
*/
#pragma once
#include <memory>
#include "data.hpp"
#include "event.hpp"
#include "execution.hpp"
#include "portfolio.hpp"
#include "strategy.hpp"

class Backtest : std::enable_shared_from_this<Backtest> {
   public:
    SymbolsType symbols;
    SharedStringType csvDirectory;
    std::shared_ptr<double> initialCapital;
    SharedQueueEventType eventQueue;
    InstantExecutionHandler exchange;
    BasicPortfolio portfolio;
    HistoricCSVDataHandler dataHandler;
    TradingStrategy strategy;

    Backtest(SharedSymbolsType ptr_symbols, SharedStringType csvDirectory,
             std::shared_ptr<double> initialCapital) {
        this->symbols = *ptr_symbols;
        this->csvDirectory = csvDirectory;
        this->initialCapital = initialCapital;
        this->dataHandler = HistoricCSVDataHandler(eventQueue, csvDirectory, ptr_symbols);
        this->exchange = InstantExecutionHandler(eventQueue, &dataHandler);
    };

    void run(std::shared_ptr<TradingStrategy> strategy) {
        // Load data and update bars
        dataHandler.loadDataFromMemory();
        dataHandler.updateBars();

        std::cout << "Starting backtesting..." << std::endl;
        while (!eventQueue->empty()) {
            // get the first event in the queue
            auto event = eventQueue->front();
            eventQueue->pop();

            // logic per event type
            switch (event->type) {
                case EventType::MARKET: {
                    strategy->calculateSignals();
                    portfolio.update();
                    break;
                }
                case EventType::SIGNAL: {
                    auto signal = std::dynamic_pointer_cast<SignalEvent>(event);
                    portfolio.onSignal(signal);
                }
                case EventType::ORDER: {
                    auto order = std::dynamic_pointer_cast<OrderEvent>(event); 
                    exchange.executeOrder(order);
                    order->logOrder();
                    break;
                }
                case EventType::FILL: {
                    auto fill = std::dynamic_pointer_cast<FillEvent>(event);
                    portfolio.onFill(fill);
                    break;
                }
            }
        }

        dataHandler.updateBars();
        std::cout << "Backtest ended\n Performance metrics\n";
        portfolio.getMetrics();
    };
};
