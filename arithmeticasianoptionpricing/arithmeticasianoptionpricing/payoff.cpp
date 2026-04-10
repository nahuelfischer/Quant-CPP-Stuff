#include "payoff.h"

PayOffCall::PayOffCall(double E) noexcept { K = E; }

// Compute the payoff for a fixed strike Asian call option
double PayOffCall::computeFixed(double mean) const noexcept {
	return std::max(mean - K, 0.0);
}

// Compute the payoff for a floating strike Asian call option
double PayOffCall::computeFloating(double mean, double S) const noexcept {
	return std::max(S - mean, 0.0);
}

