/*
    Event class
*/
#pragma once
#include <iostream>
#include <string>

// Define event types
enum EventType {
   MARKET = 0,
   SIGNAL = 1,
   ORDER = 2,
   FILL = 3
};

class Event {
   public:
      EventType type;
      std::string target;
      virtual ~Event() = default;
};

class MarketEvent : public Event {
   public:
    MarketEvent() { this->type = EventType::MARKET; }
};

class SignalEvent : public Event {
   public:
    std::string symbol;
    long long timestamp;
    double signal;

    SignalEvent(std::string* symbol, long long* timestamp, double signal, std::string target) {
      this->type = EventType::SIGNAL;
      this->symbol = *symbol;
      this->timestamp = *timestamp;
      this->signal = signal;
      this->target = target;
    };
};

class OrderEvent : public Event {
   public:
    std::string symbol;
    std::string order_type;
    double quantity;
    std::string direction;

    OrderEvent(std::string* symbol, std::string* order_type, double* quantity,
               std::string* direction, std::string target) {
      this->type = EventType::ORDER;
      this->symbol = *symbol;
      this->order_type = *order_type;
      this->quantity = *quantity;
      this->direction = *direction;
      this->target = target;
    };

    void logOrder() { std::cout << "Order placed!" << std::endl; }
};

class FillEvent : public Event {
   public:
    std::string symbol;
    long long timestamp;
    double quantity;
    std::string direction;
    double cost;
    double commission;
    double slippage;

    FillEvent(std::string* symbol, long long* timestamp, double* quantity,
              std::string direction, double cost, std::string target) {
      this->type = EventType::FILL;
      this->symbol = *symbol;
      this->timestamp = *timestamp;
      this->quantity = *quantity;
      this->direction = direction;
      this->cost = cost;
      this->commission = computeCommission();
      this->slippage = computeSlippage();
      this->target = target;
    };

    double computeCommission() { return 0.001 * cost; }
    double computeSlippage() { return 0.0; }
};
