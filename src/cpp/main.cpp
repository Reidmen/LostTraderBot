#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "backtest.hpp"
#include "strategy.hpp"

int main(int argc, char **argv) {
    auto initialCapital = std::make_shared<double>(1000.0);
    auto csvDirectory =
        std::make_shared<std::string>("data/binance_ETH-USDT_hour.csv");
    auto symbols = std::make_shared<std::vector<std::string>>();
    symbols->push_back("ETH/USDT");
    Backtest backtest = Backtest(symbols, csvDirectory, initialCapital);

    auto dataHandler = std::make_shared<HistoricCSVDataHandler>(backtest.dataHandler);
    TradingStrategy TradingStrategy(dataHandler);

    auto start = std::chrono::high_resolution_clock::now();
    // backtest.run(std::shared_ptr<TradingStrategy>)
    auto end = std::chrono::high_resolution_clock::now();
    auto time =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "Trading backtest done. Took " << time.count() << " ms "
              << std::endl;

    return 0;
}
