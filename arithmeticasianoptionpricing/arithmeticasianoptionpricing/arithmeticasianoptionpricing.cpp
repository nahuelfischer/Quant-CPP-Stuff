#include "asian_option.h"
#include "path_generation.h"
#include <iostream>
#include <iomanip>
#include <print>

int main() {
	const std::size_t numSims = 10000;
	const std::size_t numSteps = 100;
	double S0 = 100.0, K = 100.0, r = 0.04, sigma = 0.2, T = 1.0;

	std::vector<double> assetPrices(numSteps, S0);
	auto payoff = std::make_unique<PayOffCall>(K);

	AsianOptionArithmetic asianOption(std::move(payoff));

	double sumPayoff = 0.0;
	for (std::size_t i = 0; i < numSims; i++) {
		assetPrices[0] = S0;
		calcPathAssetPricesExact(assetPrices, r, sigma, T);		// Use calcPathAssetPrices or calcPathAssetPricesExact
		sumPayoff += asianOption.payOffFloating(assetPrices);	// Use payOffFloating for floating strike payoff and payOffFixed for fixed strike payoff
	}

	double price = (sumPayoff / numSims) * std::exp(-r * T);
	std::println("Arithmetic Floating Average Asian Option Price: {:.6f}\n", price);
	return 0;
}
