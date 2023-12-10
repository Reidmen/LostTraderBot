#pragma once
#include <memory>
#include <queue>

#include "data.hpp"
#include "event.hpp"

class ExecutionHandler {
   public:
    std::shared_ptr<std::queue<std::shared_ptr<Event>>> eventQueue;
    std::shared_ptr<HistoricCSVDataHandler> dataHandler;
    virtual void executeOrder(std::shared_ptr<OrderEvent>) = 0;
};

class InstantExecutionHandler : ExecutionHandler {
   public:
    InstantExecutionHandler(QueueEventType* eventQueue,
                            HistoricCSVDataHandler* dataHandler);

    InstantExecutionHandler() = default;

    void executeOrder(std::shared_ptr<OrderEvent> order);
};
