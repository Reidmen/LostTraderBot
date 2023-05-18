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

