#pragma once
#include <string>
#include <iostream>

class Event {
    public:
        int type;
        std::string target;
        virtual ~Event() = default;
};

class MarketEvent: public Event {
    public:
        MarketEvent();
};

class SignalEvent: public Event {
    public:
        std::string symbol;
        long long timestamp;
        double signal;

        SignalEvent(
                std::string* symbol,
                long long* timestamp,
                double signal,
                std::string target);
};

class OrderEvent: public Event {
    public:
        std::string symbol;
        std::string order_type;
        double quantity;
        std::string direction;

        OrderEvent(
                std::string* symbol,
                std::string order_type,
                double* quantity,
                std::string* direction,
                std::string target);

        void logOrder();
};

class FillEvent: public Event {
    public:
        std::string symbol;
        long long timestamp;
        double quantity;
        std::string direction;
        double cost;
        double commission;
        double slippage;

        FillEvent(
                std::string* symbol,
                long long* timestamp,
                double* quantity,
                std::string direction,
                double cost,
                std::string target);

        double computeCommission();
        double computeSlippage();
};
