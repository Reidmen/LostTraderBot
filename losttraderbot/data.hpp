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
        std::queue<std::shared_ptr<Event>>* eventQueue = 0;
        bool* continueBacktesting = 0;
        std::vector<std::string> symbols;

        virtual void loadData() = 0;
        virtual std::vector<std::tuple<double, double, double, double>> getLatestBars(std::string* symbol, int n = 1) = 0;
        virtual void  updateBars() = 0;
        virtual ~DataHandler() = default;
};
