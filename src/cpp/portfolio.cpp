#include "portfolio.hpp"

#include <fmt/core.h>

#include <cmath>
#include <ctime>
#include <memory>
#include <numeric>
#include <string>
#include <tuple>
#include <vector>

#include "data.hpp"
#include "event.hpp"

BasicPortfolio::BasicPortfolio(
    std::shared_ptr<HistoricCSVDataHandler> dataHandler,
    std::shared_ptr<SymbolsType> symbols,
    std::shared_ptr<double> initialCapital) {
    this->dataHandler = dataHandler;
    this->eventQueue = dataHandler->eventQueue;
    this->symbols = symbols;
    this->initialCapital = initialCapital;
    this->allPositions = constructAllPositions();
    this->currentPositions = constructCurrentPositions();
    this->allHoldings = constructAllHoldings();
    this->currentHoldings = constructCurrentHoldings();
}

auto BasicPortfolio::constructAllPositions() -> MapPositionsType {
    PositionsType innerMap;
    for (auto symbol : *symbols) {
        innerMap.insert({symbol, 0.0});
    }
    auto firstTimestamp = dataHandler->bar->first;
    MapPositionsType map;
    map.insert({firstTimestamp, innerMap});
    return map;
}

auto BasicPortfolio::constructCurrentPositions() -> PositionsType {
    PositionsType map;
    for (auto symbol : *symbols) {
        map.insert({symbol, 0.0});
    }
    return map;
}

auto BasicPortfolio::constructAllHoldings() -> MapPositionsType {
    PositionsType innerMap;
    for (auto symbol : *symbols) {
        innerMap.insert({symbol, 0.0});
    }
    innerMap.insert({"cash", *initialCapital});
    innerMap.insert({"commission", 0.0});
    innerMap.insert({"slippage", 0.0});
    innerMap.insert({"total", *initialCapital});
    innerMap.insert({"returns", 0.0});
    innerMap.insert({"equity_curve", 0.0});

    auto firstTimestamp = dataHandler->bar->first;
    MapPositionsType map;
    map.insert({firstTimestamp, innerMap});

    return map;
}

auto BasicPortfolio::constructCurrentHoldings() -> PositionsType {
    PositionsType map;
    for (auto symbol : *symbols) {
        map.insert({symbol, 0.0});
    }
    map.insert({"cash", *initialCapital});
    map.insert({"commission", 0.0});
    map.insert({"slippage", 0.0});
    map.insert({"total", *initialCapital});

    return map;
}

void BasicPortfolio::update() {
    float notCash = 0.0;
    auto prevTotal = allHoldings.rbegin()->second["total"];
    auto prevEquityCurve = allHoldings.rbegin()->second["equity_curve"];
    auto symbol_to_use = (*symbols)[0];
    auto timestamp = dataHandler->consumedData[symbol_to_use].rbegin()->first;
    for (auto symbol : *symbols) {
        allPositions[timestamp][symbol] = currentPositions[symbol];
        auto price =
            std::get<3>(dataHandler->consumedData[symbol].rbegin()->second);
        auto currentValue = currentPositions[symbol] * price;
        allHoldings[timestamp][symbol] = currentValue;
        currentHoldings[symbol] = currentValue;
        notCash += currentValue;
    }

    currentHoldings["total"] = currentHoldings["cash"] + notCash;
    allHoldings[timestamp]["total"] = currentHoldings["total"];
    allHoldings[timestamp]["cash"] = currentHoldings["cash"];
    allHoldings[timestamp]["commission"] = currentHoldings["commission"];
    allHoldings[timestamp]["slippage"] = currentHoldings["slippage"];

    if (allHoldings.size() > 1) {
        auto returns = (allHoldings[timestamp]["total"] / prevTotal) - 1;
        allHoldings[timestamp]["returns"] = returns;
        allHoldings[timestamp]["equity_curve"] =
            (prevEquityCurve + 1) * (returns + 1) - 1;
    }
}

void BasicPortfolio::updatePositionOnFill(SharedFillEventType fill_event) {
    int direction = 0;

    if (fill_event->direction == "LONG") {
        direction = 1;
    } else if (fill_event->direction == "SHORT") {
        direction = -1;
    }

    currentPositions[fill_event->symbol] += direction * fill_event->quantity;
}

void BasicPortfolio::updateHoldingsOnFill(SharedFillEventType fill_event) {
    int direction = 0;
    if (fill_event->direction == "LONG") {
        direction = 1;
    } else if (fill_event->direction == "SHORT") {
        direction = -1;
    }

    auto price = std::get<3>(dataHandler->consumedData.at(fill_event->symbol));
    auto cost = direction * fill_event->quantity * price;

    currentHoldings[fill_event->symbol] += cost;
    currentHoldings["total"] -= (fill_event->commission + fill_event->slippage);
    currentHoldings["cash"] -=
        (cost + fill_event->commission + fill_event->slippage);
    currentHoldings["commission"] += fill_event->commission;
    currentHoldings["slippage"] += fill_event->slippage;
}

void BasicPortfolio::onSignal(SharedSignalEventType event) {
    generateOrder(event);
}

void BasicPortfolio::onFill(SharedFillEventType event) {
    updateHoldingsOnFill(event);
    updatePositionOnFill(event);
}

void BasicPortfolio::generateOrder(SharedSignalEventType event) {
    float quantity = 1.0;

    std::string direction;
    if (event->signal > 0) {
        direction = "LONG";
    } else if (event->signal < 0) {
        direction = "SHORT";
    }
    eventQueue->push(std::make_shared<OrderEvent>(
        &event->symbol, "MARKET", &quantity, &direction, event->target));
}

void BasicPortfolio::getMetrics() {
    auto symbol = (*symbols)[0];
    auto r_iter = dataHandler->consumedData[symbol].rbegin();
    auto diff = r_iter->first - (++r_iter)->first;

    int period = 0;
    switch (diff) {
        case 86'400'000:
            period = 365;
            break;
        case 3'600'000:
            period = 365 * 24;
            break;
        case 60'000:
            period = 365 * 24 * 60;
            break;
    }

    double mean = std::accumulate(
        allHoldings.begin(), allHoldings.end(), 0.0,
        [](double value, const MapPositionsType::value_type& p) {
            return value + p.second.at("returns");
        });
    mean /= allHoldings.size();
    performanceMetrics["Returns (avg. per bar) [%]"] = mean * 100;
    performanceMetrics["Returns (avg. daily) [%]"] =
        (std::pow(mean + 1, period / 365) - 1) * 100;

    double ret = std::accumulate(
        allHoldings.begin(), allHoldings.end(), 0.0,
        [](double value, const MapPositionsType::value_type& p) {
            return value + std::pow(p.second.at("returns"), 2);
        });
    ret /= allHoldings.size() - 1;
    ret = std::sqrt(ret);
    performanceMetrics["Volatility (daily) [%]"] =
        ret * std::sqrt(period / 365) * 100;

    double negativeBars = std::accumulate(
        allHoldings.begin(), allHoldings.end(), 0.0,
        [](double value, const MapPositionsType::value_type& p) {
            if (p.second.at("returns") < 0)
                return ++value;
            else
                return value;
        });
    performanceMetrics["Positive Bars [%]"] =
        (allHoldings.size() - negativeBars) / allHoldings.size() * 100;

    double stdDownside = std::accumulate(
        allHoldings.begin(), allHoldings.end(), 0.0,
        [](double value, const MapPositionsType::value_type& p) {
            if (p.second.at("returns") < 0)
                return value + pow(p.second.at("returns"), 2);
            else
                return value;
        });
    stdDownside /= negativeBars - 1;
    stdDownside = std::sqrt(stdDownside);
    performanceMetrics["Volatility Downside (Daily) [%]"] =
        stdDownside * std::sqrt(period / 365) * 100;

    double annSharpe = std::sqrt(period) * mean / ret;
    performanceMetrics["Sharpe (Ann.)"] = annSharpe;

    double annSortino = std::sqrt(period) * mean / stdDownside;
    performanceMetrics["Sortino (Ann.)"] = annSortino;

    double equityPeak =
        std::max_element(allHoldings.begin(), allHoldings.end(),
                         [](const MapPositionsType::value_type& p1,
                            const MapPositionsType::value_type& p2) {
                             return p1.second.at("total") <
                                    p2.second.at("total");
                         })
            ->second.at("total");
    performanceMetrics["Equity Peak [$]"] = equityPeak;

    double equityBottom =
        std::min_element(allHoldings.begin(), allHoldings.end(),
                         [](const MapPositionsType::value_type& p1,
                            const MapPositionsType::value_type& p2) {
                             return p1.second.at("total") <
                                    p2.second.at("total");
                         })
            ->second.at("total");
    performanceMetrics["Equity Bottom [$]"] = equityBottom;

    double equityFinal = allHoldings.rbegin()->second.at("total");
    performanceMetrics["Equity Final [$]"] = equityFinal;

    double years = (double)allHoldings.size() / period;
    performanceMetrics["Duration [years]"] = years;

    performanceMetrics["CAGR [%]"] =
        (std::pow(equityFinal / *initialCapital, 1 / years) - 1) * 100;

    double returns = equityFinal / *initialCapital * 100;
    performanceMetrics["Total Return [%]"] = returns;

    for (auto metric : performanceMetrics) {
        fmt::print("{0}{1}", metric.first, metric.second);
    }
}
