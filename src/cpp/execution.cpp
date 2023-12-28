#include "execution.hpp"

#include <memory>
#include <queue>

#include "data.hpp"
#include "event.hpp"

InstantExecutionHandler::InstantExecutionHandler(
    QueueEventType* eventQueue, HistoricCSVDataHandler* dataHandler) {
    this->eventQueue = eventQueue;
    this->dataHandler = dataHandler;
}

void InstantExecutionHandler::executeOrder(SharedOrderType order) {
    auto timestamp = dataHandler->bar->first;
    eventQueue->push(std::make_shared<FillEvent>(
        &order->symbol, &timestamp, &order->quantity, order->direction, 0,
        order->target));
}
