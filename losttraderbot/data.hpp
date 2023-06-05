#include <fstream>
#include <string>
#include <memory>
#include "event.hpp"
#include <queue>
#include <tuple>
#include <vector>

class DataHandler {
    public:
        std::string csvDirectory;
        std::unique_ptr<std::queue<std::shared_ptr<Event>>> EventQueue = 0;
        std::unique_ptr<bool> continueBacktesting = 0;
        std::vector<std::string> symbols;

        virtual void loadData() = 0;
        virtual std::vector<std::tuple<double, double, double, double>> getLatestBars(std::string* symbol, int n = 1) = 0;
        virtual void  updateBars() = 0;
        virtual ~DataHandler() = default;
};
