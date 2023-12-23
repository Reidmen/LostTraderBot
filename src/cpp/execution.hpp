#pragma once
#include <memory>
#include <queue>

#include "data.hpp"
#include "event.hpp"

using SharedOrderType = std::shared_ptr<OrderEvent>;

class ExecutionHandler {
   public:
    QueueEventType* eventQueue;
    HistoricCSVDataHandler* dataHandler;
    virtual void executeOrder(SharedOrderType order) = 0;
};

class InstantExecutionHandler : ExecutionHandler {
   public:
    InstantExecutionHandler(QueueEventType* eventQueue,
                            HistoricCSVDataHandler* dataHandler);

    InstantExecutionHandler() = default;

    void executeOrder(SharedOrderType order);
};
