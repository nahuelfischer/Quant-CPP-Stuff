#include <iostream>
#include <cmath>
#include <vector>
#include <algorithm>

double normalCDF(double x) {
	return 0.5 * std::erf(-x / std::sqrt(2.0));
}

std::vector<double> LRCash(double S0, double K, double r, double dividends, double sigma, double T, double PayOff, int n, const std::string& PutCall, const std::string& EuroAmer, int Method) {
	if ((EuroAmer == "Amer" && PutCall == "Call" && S0 >= K) || (EuroAmer == "Amer" && PutCall == "Put" && S0 <= K)) {
		std::cerr << "Error: For American options, the initial spot price must be less than the strike price for calls and greater than the strike price for puts." << std::endl;
		exit(1);
	}

	if (n % 2 == 0) ++n;

	double dt = T / n;
	double exp_rt = std::exp(-r * dt);

	double d1 = (std::log(S0 / K) + (r - dividends + 0.5 * sigma * sigma) * T) / (sigma * std::sqrt(T));
	double d2 = (std::log(S0 / K) + (r - dividends - 0.5 * sigma * sigma) * T) / (sigma * std::sqrt(T));

	double Term1 = std::pow(d1 / (n + 1 / 3 - (1 - Method) * 0.1 / (n + 1)), 2) * (n + 1 / 6);
	double pp = 0.5 + std::copysign(0.5, d1) * std::sqrt(1 - std::exp(-Term1));

	Term1 = std::pow(d2 / (n + 1 / 3 - (1 - Method) * 0.1 / (n + 1)), 2) * (n + 1 / 6);
	double p = 0.5 + std::copysign(0.5, d2) * std::sqrt(1 - std::exp(-Term1));

	double u = std::exp((r - dividends) * dt) * pp / p;
	double d = (std::exp((r - dividends) * dt) - p * u) / (1 - p);

	std::vector<std::vector<double>> assetPrices(n + 1, std::vector<double>(n + 1, 0.0));
	assetPrices[0][0] = S0;

	for (int i = 0; i <= n; ++i) {
		for (int j = i; j <= n; ++j) {
			assetPrices[i][j] = S0 * std::pow(u, j - i) * std::pow(d, i);
		}
	}

	std::vector<std::vector<double>> optionValues(n + 1, std::vector<double>(n + 1, 0.0));

	for (int i = 0; i <= n; ++i) {
		optionValues[i][n] = (PutCall == "Call") ? (assetPrices[i][n] >= K ? PayOff : 0.0) : (assetPrices[i][n] < K ? PayOff : 0.0);
	}

	for (int j = n - 1; j >= 0; --j) {
		for (int i = 0; i <= j; ++i) {
			double expectedValue = exp_rt * (p * optionValues[i][j + 1] + (1 - p) * optionValues[i + 1][j + 1]);
			if (EuroAmer == "Amer") {
				double exerciseValue = (PutCall == "Call") ? (assetPrices[i][j] >= K ? PayOff : 0.0) : (assetPrices[i][j] < K ? PayOff : 0.0);
				optionValues[i][j] = std::max(expectedValue, exerciseValue);
			} else {
				optionValues[i][j] = expectedValue;
			}
		}
	}

	return { optionValues[0][0] };
}

int main() {
	double S0 = 30.0; // Initial stock price
	double K = 40.0; // Strike price
	double r = 0.04; // Risk-free rate
	double dividends = 0.01;
	double sigma = 0.30; // Volatility
	double T = 0.5; // Time to maturity
	double PayOff = 10.0; // Payoff for cash-or-nothing option
	int n = 250; // Number of steps in the binomial tree
	std::string PutCall = "Call"; // Option type: "Call" or "Put"
	std::string EuroAmer = "Amer"; // Option style: "Euro" or "Amer"
	int Method = 2;

	auto result = LRCash(S0, K, r, dividends, sigma, T, PayOff, n, PutCall, EuroAmer, Method);

	std::cout << "The price of the " << EuroAmer << " " << PutCall << " cash-or-nothing option is: " << result[0] << std::endl;

	return 0;
}