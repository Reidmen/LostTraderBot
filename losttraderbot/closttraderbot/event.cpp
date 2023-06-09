#include "event.hpp"
#include <cstdio>
#include <iostream>
#include <ostream>
#include <string>

MarketEvent::MarketEvent() {
    this->type = 0;
}

SignalEvent::SignalEvent(
        std::string* symbol,
        long long* timestamp,
        double signal, 
        std::string target) {
    this->type = 1;
    this->symbol = *symbol;
    this->timestamp = *timestamp;
    this->signal = signal;
    this->target = target;
}

OrderEvent::OrderEvent(
        std::string* symbol,
        std::string order_type,
        double* quantity,
        std::string* direction,
        std::string target) {
    this->type = 2;
    this->symbol = *symbol;
    this->order_type = order_type;
    this->quantity = *quantity;
    this->direction= *direction;
    this->target = target;
}

void OrderEvent::logOrder() {
    std::cout << "Order placed!" << std::endl;
}

FillEvent::FillEvent(
        std::string* symbol,
        long long* timestamp,
        double* quantity,
        std::string direction,
        double cost,
        std::string target) {
    this->type = 3;
    this->symbol = *symbol;
    this->timestamp = *timestamp;
    this->quantity = *quantity;
    this->direction = direction;
    this->cost = cost;
    this->commission = computeCommission();
    this->slippage = computeSlippage();
    this->target = target;
}

double FillEvent::computeCommission() {
    return 0.001 * cost;
}

double FillEvent::computeSlippage() {
    return 0.0;
}
