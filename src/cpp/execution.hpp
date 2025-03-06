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
    SharedHistoricCSVDataHandler dataHandler;
    virtual void executeOrder(SharedOrderType order) = 0;
};

class InstantExecutionHandler : ExecutionHandler {
   public:
    InstantExecutionHandler(SharedQueueEventType eventQueue,
                            SharedHistoricCSVDataHandler dataHandler){
        this->eventQueue = eventQueue;
        this->dataHandler = dataHandler;
    };

    InstantExecutionHandler() = default;

    void executeOrder(SharedOrderType order) {
        auto timestamp = dataHandler->bar->first;
        eventQueue->push(std::make_shared<FillEvent>(
            &order->symbol, &timestamp, &order->quantity, order->direction, 0,
            order->target));
    };
};
