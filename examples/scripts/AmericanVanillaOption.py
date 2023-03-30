"""
American Vanilla Option

It  differs from the European by an early exercise of the option.
Here, `u`, `d` should match the volatility of the stock price. See
Hull, Options, futures and other derivatives, 8th Edition 2012. p. 265.
"""
import numpy as np
import pandas as pd
from math import factorial
from abc import ABC, abstractmethod


class VanillaOption(ABC):
    @abstractmethod
    def _price_tree_computation(self):
        return NotImplementedError()

    def _option_payoff_computation(self):
        return NotImplementedError()


class AmericanVanillaOption(VanillaOption):
    def __init__(self, option_type: str, parameters: dict):
        super().__init__()
        self.option_type = option_type
        self.K = parameters.get("K", 110)
        self.T = parameters.get("T", 1.0)
        self.N = parameters.get("N", 2)
        self.S0 = parameters.get("S0", 100)
        self.dt = self.T / self.N

        self.r = parameters.get("r", 0.01)
        self.mu = parameters.get("mu", 0.0)

        self.F0 = self.S0 * np.exp((self.r - self.mu) * self.T)
        self.a = np.exp((self.r - self.mu) * self.dt)
        self.sigma = parameters.get("sigma", 0.3)
        # percentages of going up and down
        self.u = np.exp(self.sigma * np.sqrt(self.dt))
        self.d = 1 / self.u
        # probability of going up (thus, going down us 1 - prob. up)
        self.p = (self.a - self.d) / (self.u - self.d)

        # compute price tree and option payoff
        self._price_tree_computation()
        self._option_payoff_computation()
        self._backward_discount()

    def _price_tree_computation(self):
        N = self.N
        self._price_tree = np.zeros((N + 1, N + 1))
        for j in range(N + 1):
            for i in range(j, N + 1):
                # row j, column i (aka binomial steps)
                # paths = factorial(i) / factorial(j) / factorial (i - j)
                value = self.u ** (i - j) * (self.d) ** j
                self._price_tree[j, i] = self.F0 * value

    def _option_payoff_computation(self):
        N = self.N
        self._option_payoff = np.zeros((N + 1, N + 1))
        # computing payoff at maturity
        for j in range(N + 1):
            for i in range(j, N + 1):
                if self.option_type == "CALL":
                    self._option_payoff[j, i] = np.maximum(
                        0.0, self._price_tree[j, i] - self.K
                    )
                elif self.option_type == "PUT":
                    self._option_payoff[j, i] == np.maximum(
                        0.0, self.K - self._price_tree[j, i]
                    )

    def _backward_discount(self):
        N = self.N
        for j in range(N + 1, -1, -1):
            for i in range(j + 1, N):
                self._option_payoff[j, i] = (
                    self.p * self._option_payoff[j, i + 1]
                    + (1 - self.p) * self._option_payoff[j + 1, i + 1]
                )
                self._option_payoff[j, i] *= np.exp(-1.0 * (self.r - self.mu) * self.dt)
                # early exercise
                if self.option_type == "CALL":
                    self._option_payoff[j, i] = np.maximum(
                        self._price_tree[j, i] - self.K, self._option_payoff[j, i]
                    )
                elif self.option_type == "PUT":
                    self._option_payoff[j, i] = np.maximum(
                        self.K - self._option_payoff[j, i], self._option_payoff[j, i]
                    )

    def __repr__(self):
        return "Price of American option: {}, tree \n {}".format(
                self._option_payoff[0, 0], self._option_payoff)


class EuropeanVanillaOption(VanillaOption):
    def __init__(self, option_type: str, parameters: dict):
        super().__init__()
        self.option_type = option_type
        self.K = parameters.get("K", 90) # strike price
        self.S0 = parameters.get("S0", 100)
        self.T = parameters.get("T", 1.0)
        self.N = parameters.get("N", 2)
        self.dt = self.T / self.N

        self.r = parameters.get("r", 0.01)
        self.mu = parameters.get("mu", 0.0)

        self.F0 = self.S0 * np.exp((self.r - self.mu) * self.T)
        self.a = np.exp((self.r - self.mu) * self.dt)
        self.sigma = parameters.get("sigma", 0.3)
        # percentages of going up and down
        self.u = np.exp(self.sigma * np.sqrt(self.dt))
        self.d = 1 / self.u
        # probability of going up (thus, going down us 1 - prob. up)
        self.p = (self.a - self.d) / (self.u - self.d)

        # compute price tree and option payoff
        self._price_tree_computation()
        self._option_payoff_computation()
        self._backward_discount()

    def _price_tree_computation(self):
        N = self.N
        self._price_tree = np.zeros((N + 1, N + 1))
        for j in range(N + 1):
            for i in range(j, N + 1):
                # row j, column i (aka binomial steps)
                path = factorial(i) / factorial(j) / factorial(i - j)
                prob = self.p ** (i - j) * (1.0 - self.p) ** j
                value = self.u ** (i - j) * self.d ** j
                self._price_tree[j, i] = self.F0 * prob * path * value

    def _option_payoff_computation(self):
        N = self.N
        self._option_payoff = np.zeros((N + 1, N + 1))
        # computing payoff at maturity
        for j in range(N + 1):
            for i in range(j, N + 1):
                if self.option_type == "CALL":
                    self._option_payoff[j, i] = np.maximum(
                        0.0, self._price_tree[j, i] - self.K
                    )
                elif self.option_type == "PUT":
                    self._option_payoff[j, i] == np.maximum(
                        0.0, self.K - self._price_tree[j, i]
                    )

    def _backward_discount(self):
        N = self.N
        final_payoff = 0.0
        for j in range(N + 1):
            final_payoff += self._option_payoff[j, N]

        final_payoff *= np.exp(-1.0 * (self.r * self.T))

        self.final_payoff = final_payoff

    def __repr__(self):
        return "Payoff European option: {}, Price tree \n {}".format(
                self.final_payoff, self._price_tree)


if __name__ == "__main__":
    parameters = {
        "S0": 50,
        "K": 20,
        "r": 0.01,
        "mu": 0.0,
        "sigma": 0.1,
        "N": 2,
        "T": 1.0,
        "t": 0,
    }
    option = AmericanVanillaOption("CALL", parameters)
    print(option)
    option = EuropeanVanillaOption("CALL", parameters)
    print(option)
