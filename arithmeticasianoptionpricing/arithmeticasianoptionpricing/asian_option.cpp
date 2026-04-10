#include "asian_option.h"
#include <numeric>
#include <cmath>

AsianOption::AsianOption(std::unique_ptr<PayOff> _payoff) noexcept : payoff(std::move(_payoff)) {}

AsianOptionArithmetic::AsianOptionArithmetic(std::unique_ptr<PayOff> _payoff) noexcept : AsianOption(std::move(_payoff)) {}

// Compute Arithmetic Fixed-Strike Payoff
double AsianOptionArithmetic::payOffFixed(const std::vector<double>& assetPrices) const noexcept {
	if (assetPrices.empty()) return 0.0;

	double sum = std::accumulate(assetPrices.begin(), assetPrices.end(), 0.0);
	double arithMean = sum / assetPrices.size();

	return payoff->computeFixed(arithMean);
}

// Compute Arithmetic Floating-Strike Payoff
double AsianOptionArithmetic::payOffFloating(const std::vector<double>& assetPrices) const noexcept {
	if (assetPrices.empty()) return 0.0;

	double sum = std::accumulate(assetPrices.begin(), assetPrices.end(), 0.0);
	double arithMean = sum / assetPrices.size();

	return payoff->computeFloating(arithMean, assetPrices.back());
}