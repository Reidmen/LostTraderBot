#include <fmt/core.h>
#include <ta-lib/ta_common.h>
#include <ta-lib/ta_defs.h>
#include <ta-lib/ta_libc.h>

#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "backtest.hpp"
#include "data.hpp"
#include "strategy.hpp"

// data.hpp defines main symbols -> should be refactored to new file

int main(int argc, char **argv) {
    TA_RetCode initialize_talib = TA_Initialize();
    if (initialize_talib != TA_SUCCESS) {
        fmt::print("Cannot initialize TA-lib");
        return -1;
    }

    auto initialCapital = std::make_shared<double>(1000.0);
    auto csvDirectory = std::make_shared<std::string>(
        "../../examples/datasets/dataset_1h_AAPL.csv");
    auto symbols = std::make_shared<SymbolsType>();
    symbols->push_back("APPL");
    auto backtest = Backtest(symbols, csvDirectory, initialCapital);

    auto dataHandler =
        std::make_shared<HistoricCSVDataHandler>(backtest.dataHandler);
    auto trading_strategy = std::make_shared<TradingStrategy>(dataHandler);

    auto start = std::chrono::high_resolution_clock::now();
    backtest.run(trading_strategy);
    auto end = std::chrono::high_resolution_clock::now();
    auto time =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    fmt::print("Trading backtest done. Took {} ms.", time.count());

    return TA_Shutdown();
}
