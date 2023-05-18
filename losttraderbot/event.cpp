#include "event.hpp"
#include <string>

MarketEvent::MarketEvent() {
    this -> type = 0;
}

SignalEvent::SignalEvent(
        std::string* symbol,
        long long* timestamp,
        double signal, 
        std::string target) {
    this -> type = 1;
    this -> symbol = *symbol;
    this -> timestamp = *timestamp;
    this -> signal = signal;
    this -> target = target;
}
