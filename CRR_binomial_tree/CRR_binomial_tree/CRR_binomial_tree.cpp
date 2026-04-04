#include <iostream>
#include <vector>
#include <string_view>
#include <cmath>
#include <algorithm>
#include <format>
#include <ranges>

// Struct to hold the option results
struct OptionResult {
	double price;
	double delta;
	double gamma;
	double theta;
};

// Function for binomial option pricing with Greeks
OptionResult Binomial(double S0, double K, double r, double sigma, double T, int n, std::string_view PutCall, std::string_view EuroAmer, const std::vector<std::vector<double>>& Dividends) {
	const double dt = T / n;
	const double u = std::exp(sigma * std::sqrt(dt));
	const double d = 1.0 / u;
	const double p = (std::exp(r * dt) - d) / (u - d);
	const double discount = std::exp(-r * dt);

	// Process dividends
	double divsum = 0.0;
	std::vector<double> Div, Tau, Rate;

	for (const auto& dividend : Dividends) {
		Tau.push_back(dividend[0]);
		Div.push_back(dividend[1]);
		Rate.push_back(dividend[2]);
		divsum += dividend[1] * std::exp(-dividend[2] * dividend[0]);
	}

	// Stock price tree
	std::vector<std::vector<double>> S(n + 1, std::vector<double>(n + 1, 0.0));
	S[0][0] = S0 - divsum;

	for (int j = 0; j <= n; ++j) {
		for (int i = 0; i <= j; ++i) {
			S[i][j] = S[0][0] * std::pow(u, j - i) * std::pow(d, i);
		}
	}

	// Add present value (PV) of dividends at each node
	for (size_t z = 0; z < Div.size(); ++z) {
		for (int j = 1; j <= n; ++j) {
			for (int i = 0; i <= j; ++i) {
				if (Tau[z] >= (j - 1) * dt) {
					S[i][j] += Div[z] * std::exp(-Rate[z] * (Tau[z] - (j - 1) * dt));
				}
			}
		}
	}

	// Option value tree
	std::vector<std::vector<double>> Op(n + 1, std::vector<double>(n + 1, 0.0));

	// Compute option payoffs at expiration
	for (int i = 0; i <= n; ++i) {
		Op[i][n] = (PutCall == "Call") ? std::max(S[i][n] - K, 0.0) : std::max(K - S[i][n], 0.0);
	}

	// Backward induction for option pricing
	for (int j = n - 1; j >= 0; --j) {
		for (int i = 0; i <= j; ++i) {
			double expected = discount * (p * Op[i][j + 1] + (1 - p) * Op[i + 1][j + 1]);
			double intrinsic = (PutCall == "Call") ? std::max(S[i][j] - K, 0.0) : std::max(K - S[i][j], 0.0);
			Op[i][j] = (EuroAmer == "Amer") ? std::max(intrinsic, expected) : expected;
		}
	}

	// Compute Greeks
	double delta = (Op[0][1] - Op[1][1]) / (S[0][1] - S[1][1]);
	double gamma = ((Op[0][2] - Op[1][2]) / (S[0][2] - S[1][2]) - (Op[1][2] - Op[2][2]) / (S[1][2] - S[2][2])) / ((S[0][1] - S[1][1]) / 2);
	double theta = (Op[1][2] - Op[0][0]) / (2 * dt);

	return { Op[0][0], delta, gamma, theta };
}

// Function to compute Vega
double Vega(double S0, double K, double r, double sigma, double T, int n, std::string_view PutCall, std::string_view EuroAmer, const std::vector<std::vector<double>>& Dividends) {
	constexpr double change = 1e-5;
	OptionResult f = Binomial(S0, K, r, sigma, T, n, PutCall, EuroAmer, Dividends);
	OptionResult fs = Binomial(S0, K, r, sigma + change, T, n, PutCall, EuroAmer, Dividends);
	return (fs.price - f.price) / change;
}

// Function to compute Rho
double Rho(double S0, double K, double r, double sigma, double T, int n, std::string_view PutCall, std::string_view EuroAmer, const std::vector<std::vector<double>>& Dividends) {
	constexpr double change = 1e-5;
	OptionResult f = Binomial(S0, K, r, sigma, T, n, PutCall, EuroAmer, Dividends);
	OptionResult fs = Binomial(S0, K, r + change, sigma, T, n, PutCall, EuroAmer, Dividends);
	return (fs.price - f.price) / change;
}

// Main function
int main() {
	// Option parameters
	constexpr double S0 = 100.0;
	constexpr double K = 140.0;
	constexpr double r = 0.0431;
	constexpr double sigma = 0.35;
	constexpr double T = 1.0;
	constexpr int n = 1000;

	// No dividends in this case
	std::vector<std::vector<double>> Dividends;

	// Compute American Call and Put Prices & Greeks
	OptionResult callResult = Binomial(S0, K, r, sigma, T, n, "Call", "Amer", Dividends);
	double callVega = Vega(S0, K, r, sigma, T, n, "Call", "Amer", Dividends);
	double callRho = Rho(S0, K, r, sigma, T, n, "Call", "Amer", Dividends);

	OptionResult putResult = Binomial(S0, K, r, sigma, T, n, "Put", "Amer", Dividends);
	double putVega = Vega(S0, K, r, sigma, T, n, "Put", "Amer", Dividends);
	double putRho = Rho(S0, K, r, sigma, T, n, "Put", "Amer", Dividends);

	// Output results

	std::cout << std::format("\n{:<15}: {:.4f}", "American Call Price", callResult.price) << '\n';
	std::cout << std::format("{:<15}: {:.4f}", "Delta", callResult.delta) << '\n';
	std::cout << std::format("{:<15}: {:.4f}", "Gamma", callResult.gamma) << '\n';
	std::cout << std::format("{:<15}: {:.4f}", "Theta", callResult.theta) << '\n';
	std::cout << std::format("{:<15}: {:.4f}", "Vega", callVega) << '\n';
	std::cout << std::format("{:<15}: {:.4f}", "Rho", callRho) << '\n\n';

	std::cout << std::format("\n{:<15}: {:.4f}", "American Put Price", putResult.price) << '\n';
	std::cout << std::format("{:<15}: {:.4f}", "Delta", putResult.delta) << '\n';
	std::cout << std::format("{:<15}: {:.4f}", "Gamma", putResult.gamma) << '\n';
	std::cout << std::format("{:<15}: {:.4f}", "Theta", putResult.theta) << '\n';
	std::cout << std::format("{:<15}: {:.4f}", "Vega", putVega) << '\n';
	std::cout << std::format("{:<15}: {:.4f}", "Rho", putRho) << '\n\n';

	return 0;
}