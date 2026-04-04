// This code calculates the implied volatility of a European call option using the Black-Scholes formula and the interval bisection method.

#include <iostream>
#include <cmath>
#include <functional>
#include <stdexcept>

// Black-Scholes formula for European call option
double blackScholesCall(double S, double K, double r, double sigma, double T) {
	double d1 = (std::log(S / K) + (r + 0.5 * sigma * sigma) * T) / (sigma * std::sqrt(T));
	double d2 = d1 - sigma * std::sqrt(T);
	return S * 0.5 * (1 + std::erf(d1 / std::sqrt(2))) - K * std::exp(-r * T) * 0.5 * (1 + std::erf(d2 / std::sqrt(2)));
}

// Function to find implied volatility using interval bisection method
template <typename F>
double bisection(F&& func, double lower, double upper, double tol = 1e-6, int maxIter = 100) {
	// Ensure the interval brackets a root
	double fLower = func(lower);
	double fUpper = func(upper);
	if (fLower * fUpper >= 0) {
		throw std::invalid_argument("The function does not change signs in the given interval.");
	}

	// Bisection method
	for (int i = 0; i < maxIter; ++i) {
		double mid = (lower + upper) / 2.0;
		double fMid = func(mid);

		// Check if we found the root or the interval is sufficiently small
		if (std::abs(fMid) < tol || (upper - lower) / 2 < tol) {
			return mid;
		}

		// Update the interval
		if (fMid * fLower > 0) {
			lower = mid;
		}
		else {
			upper = mid;
		}
	}
	throw std::runtime_error("Bisection method did not converge.");
}

// Example usage
int main() {
	double S = 80.0; // Stock price
	bool boolSRange = true; // Set to true to calculate implied volatility for a range of stock prices
	double SRange = 10.0; // Range for stock price variation
	double K = 80.0; // Strike price
	bool boolKRange = false; // Set to true to calculate implied volatility for a range of strike prices
	double KRange = 10.0; // Range for strike price variation
	double r = 0.04; // Risk-free rate
	double T = 1.0; // Time to maturity
	double marketPrice = 10.0; // Market price of the option
	bool boolmarketPriceRange = false; // Set to true to calculate implied volatility for a range of market prices
	double marketPriceRange = 5.0; // Range for market price variation


	// Define the function to find the root of
	if (boolSRange == true && boolKRange == true) {
		std::cerr << "Error: Cannot vary both stock price and strike price simultaneously." << std::endl;
		return 1;
	}
	else if (boolSRange == true && boolmarketPriceRange == true) {
		std::cerr << "Error: Cannot vary both stock price and market price simultaneously." << std::endl;
		return 1;
	}
	else if (boolKRange == true && boolmarketPriceRange == true) {
		std::cerr << "Error: Cannot vary both strike price and market price simultaneously." << std::endl;
		return 1;
	}
	
	if (boolSRange == true) {
		for (double stockPrice = S - SRange; stockPrice <= S + SRange; stockPrice += 5.0) {
			auto f = [stockPrice, K, r, T, marketPrice](double sigma) {
				return blackScholesCall(stockPrice, K, r, sigma, T) - marketPrice;
			};
			try {
				double sigma = bisection(f, 0.01, 1.0); // Search for implied volatility in the range [0.01, 1.0]
				std::cout << "Stock Price: " << stockPrice << ", Implied Volatility: " << sigma << std::endl;
			}
			catch (const std::exception& e) {
				std::cerr << "Error: " << e.what() << std::endl;
			}
		}
	}
	else if (boolKRange == true) {
		for (double strikePrice = K - KRange; strikePrice <= K + KRange; strikePrice += 5.0) {
			auto f = [S, strikePrice, r, T, marketPrice](double sigma) {
				return blackScholesCall(S, strikePrice, r, sigma, T) - marketPrice;
				};
			try {
				double sigma = bisection(f, 0.01, 1.0); // Search for implied volatility in the range [0.01, 1.0]
				std::cout << "Strike Price: " << strikePrice << ", Implied Volatility: " << sigma << std::endl;
			}
			catch (const std::exception& e) {
				std::cerr << "Error: " << e.what() << std::endl;
			}
		}
	}
	else if (boolmarketPriceRange == true) {
		for (double price = marketPrice - marketPriceRange; price <= marketPrice + marketPriceRange; price += 0.5) {
			auto f = [S, K, r, T, price](double sigma) {
				return blackScholesCall(S, K, r, sigma, T) - price;
			};
			try {
				double sigma = bisection(f, 0.01, 1.0); // Search for implied volatility in the range [0.01, 1.0]
				std::cout << "Market Price: " << price << ", Implied Volatility: " << sigma << std::endl;
			}
			catch (const std::exception& e) {
				std::cerr << "Error: " << e.what() << std::endl;
			}
		}
	}
	else {
		auto f = [S, K, r, T, marketPrice](double sigma) {
			return blackScholesCall(S, K, r, sigma, T) - marketPrice;
			};

		try {
			double sigma = bisection(f, 0.01, 1.0); // Search for implied volatility in the range [0.01, 1.0]
			std::cout << "Implied Volatility: " << sigma << std::endl;
		}
		catch (const std::exception& e) {
			std::cerr << "Error: " << e.what() << std::endl;
		}
	}

	return 0;
}