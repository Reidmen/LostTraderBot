#pragma once
#include <memory>
#include <queue>

#include "data.hpp"
#include "event.hpp"

using SharedOrderType = std::shared_ptr<OrderEvent>;
using SharedHistoricCSVDataHandler = std::shared_ptr<HistoricCSVDataHandler>;

class ExecutionHandler {
   public:
    SharedQueueEventType eventQueue;
    HistoricCSVDataHandler* dataHandler;
    virtual void executeOrder(SharedOrderType order) = 0;
};

class InstantExecutionHandler : ExecutionHandler {
   public:
    InstantExecutionHandler(SharedQueueEventType eventQueue,
                            HistoricCSVDataHandler* dataHandler);

    InstantExecutionHandler() = default;

    void executeOrder(SharedOrderType order);
};
