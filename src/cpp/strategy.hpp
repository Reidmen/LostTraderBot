/*
    Strategy Class
    
    The Strategy is responsible for:
    1. Analyzing market data to identify trading opportunities
    2. Generating trading signals based on predefined rules or models
    3. Communicating these signals to the Portfolio component via SignalEvents
*/
#pragma once
#include <memory>
#include <queue>
#include <string>
#include <unordered_map>

#include "data.hpp"
#include "event.hpp"

/*
 * Abstract Strategy class that defines the interface for all trading strategies
 */
class Strategy {
   public:
    // Analyzes market data and generates trading signals
    // This is the core method that implements the trading logic
    virtual void calculateSignals() = 0;
};

/*
 * Concrete implementation of a mean-reversion trading strategy
 */
class TradingStrategy : Strategy {
   public:
    // Pointer to data handler for accessing market data
    std::shared_ptr<HistoricCSVDataHandler> dataHandler;
    
    // Pointer to event queue for publishing signals
    std::shared_ptr<std::queue<std::shared_ptr<Event>>> eventQueue;
    
    // Tracks position state for each symbol (true = long position held)
    // This prevents the strategy from generating duplicate signals
    std::unordered_map<std::string, bool> bought;

    // Constructor initializes the strategy with a data source
    TradingStrategy(std::shared_ptr<HistoricCSVDataHandler> dataHandler) {
        this->dataHandler = dataHandler;
        this->eventQueue = dataHandler->eventQueue;

        std::unordered_map<std::string, bool> bought;

        // Initialize position tracking for all symbols
        for (auto symbol : dataHandler->symbols) {
            bought[symbol] = false;
        }

        this->bought = bought;
    };

    TradingStrategy() = default;

    /*
     * Implements the RSI-based mean reversion strategy
     * 
     * The strategy:
     * 1. Calculates RSI using the last n+1 price bars
     * 2. Generates buy signals when RSI < 30 (oversold condition)
     * 3. Generates sell signals when RSI > 70 (overbought condition)
     * 4. Tracks positions to avoid duplicate signals
     */
    void calculateSignals() {
        for (auto symbol : dataHandler->symbols) {
            int n = 20;  // Lookback period for RSI calculation
            int direction = 0;  // Signal direction: 1=buy, -1=sell, 0=no action

            // Container for closing prices
            std::vector<double> closes;
            closes.reserve(n + 1);

            // Retrieve the latest n+1 bars for the current symbol
            auto ptr_symbol = std::make_shared<std::string>(symbol);
            auto data = dataHandler->getLatestBars(ptr_symbol, n + 1);
            
            // Skip if we don't have enough data for calculation
            if (data.size() < n + 1) return;
            
            // Extract closing prices from the price bars
            for (auto close : data) {
                closes.emplace_back(std::get<3>(close));
            }

            // TODO: Compute the RSI
            // Placeholder for actual RSI calculation
            // In a complete implementation, this would calculate:
            // 1. Price changes for each period
            // 2. Average gains and losses over n periods
            // 3. Relative strength (avg gain / avg loss)
            // 4. RSI = 100 - (100 / (1 + RS))
            double rsi = 20;  // Placeholder value

            // Generate trading signals based on RSI thresholds
            // RSI > 70 indicates overbought conditions (sell signal)
            // RSI < 30 indicates oversold conditions (buy signal)
            if (rsi > 70) {
                direction = -1;  // Sell signal
            } else if (rsi < 30) {
                direction = 1;   // Buy signal
            }

            // Create a SignalEvent if we have a valid signal and position state allows it
            // Only buy if we don't already own the asset
            // Only sell if we currently own the asset
            if (direction != 0 && ((direction == 1 && !bought[symbol]) ||
                                   (direction == -1 && bought[symbol]))) {
                // Get current timestamp from the most recent data point
                auto timestamp = dataHandler->consumedData.at(symbol).begin()->first;
                
                // Create and push a new signal event to the event queue
                eventQueue->push(std::make_shared<SignalEvent>(
                    &symbol, &timestamp, 1.0 * direction, "ALGORITHM"));
                
                // Update position tracking
                bought[symbol] = !bought[symbol];
            }
        }
    };
};
