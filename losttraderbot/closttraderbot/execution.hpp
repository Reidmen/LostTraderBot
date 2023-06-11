#pragma once
#include "event.hpp"
#include "data.hpp"
#include <memory>
#include <queue>

class ExecutionHandler {
    public:
        std::unique_ptr<std::queue<std::shared_ptr<Event>>> eventQueue;
        std::unique_ptr<HistoricCSVDataHandler> dataHandler;
        virtual void executeOrder(std::shared_ptr<OrderEvent>) = 0;
};

class InstantExecutionHandler: ExecutionHandler {
    public:
        InstantExecutionHandler(
                std::unique_ptr<std::queue<std::shared_ptr<Event>>> eventQueue,
                std::unique_ptr<HistoricCSVDataHandler> dataHandler
                );

        InstantExecutionHandler() = default;

        void executeOrder(std::shared_ptr<OrderEvent> order);
};
