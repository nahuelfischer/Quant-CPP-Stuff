#include <iostream>
#include <cmath>
#include <algorithm>
#include <vector>
#include <string>
#include <utility>

double fbinomial(double S0, double K, double r, double sigma, double T, int n, std::string PutCall, std::string EuroAmer) {
	double dt = T / n;
	double u = exp(sigma * sqrt(dt));
	double d = 1 / u;
	double p = (exp(r * dt) - d) / (u - d);
	double exp_rt = exp(-r * dt);

	std::vector<std::vector<double>> assetPrice(n + 1, std::vector<double>(n + 1, 0.0));
	std::vector<std::vector<double>> optionPrice(n + 1, std::vector<double>(n + 1, 0.0));

	// Asset price tree
	for (int i = 1; i <= n; ++i) {
		for (int j = i; j <= n; ++j) {
			assetPrice[i][j] = S0 * pow(u, j - i) * pow(d, i - 1);
		}
	}

    // Option prices at maturity
	for (int i = 1; i <= n; ++i) {
		if (PutCall == "Call") {
			optionPrice[i][n] = std::max(0.0, assetPrice[i][n] - K);
		} else if (PutCall == "Put") {
			optionPrice[i][n] = std::max(0.0, K - assetPrice[i][n]);
		}
	}

	// Remaining option prices
	for (int j = n - 1; j >= 1; --j) {
		for (int i = 1; i <= j; ++i) {
			if (EuroAmer == "Euro") {
				optionPrice[i][j] = exp_rt * (p * optionPrice[i][j + 1] + (1 - p) * optionPrice[i + 1][j + 1]);
			}
			else if (EuroAmer == "Amer") {
				if (PutCall == "Call") {
					optionPrice[i][j] = std::max(assetPrice[i][j] - K, exp_rt * (p * optionPrice[i][j + 1] + (1 - p) * optionPrice[i + 1][j + 1]));
				} else if (PutCall == "Put") {
					optionPrice[i][j] = std::max(K - assetPrice[i][j], exp_rt * (p * optionPrice[i][j + 1] + (1 - p) * optionPrice[i + 1][j + 1]));
				}
			}
		}
	}

   return optionPrice[1][1];
}

	double NewBarrier(double S0, double Bar, double sigma, double T, int M1, std::string M2) {
		int Sign = (Bar > S0) ? 1 : -1;

        if (M1 != 0) {
			Bar *= exp(Sign * 0.5826 * sigma * sqrt(T / M1));
		}
		else {
			if (M2 == "H") Bar *= exp(Sign * 0.5826 * sigma * sqrt(1.0 / (24.0 * 365.0)));
			else if (M2 == "D") Bar *= exp(Sign * 0.5826 * sigma * sqrt(1.0 / 365.0));
			else if (M2 == "W") Bar *= exp(Sign * 0.5826 * sigma * sqrt(1.0 / 52.0));
			else if (M2 == "M") Bar *= exp(Sign * 0.5826 * sigma * sqrt(1.0 / 12.0));
		}
		return Bar;
	}

std::pair<double, int> BarrierBin(double S0, double K, double& Bar, double r, double sigma, double T, int old_n, std::string PutCall, std::string EuroAmer, std::string BarType, int M1, std::string M2) {

		Bar = NewBarrier(S0, Bar, sigma, T, M1, M2);

		if (((BarType == "DO" || BarType == "DI") && Bar > S0) || ((BarType == "UO" || BarType == "UI") && Bar < S0)) {
			std::cerr << "Error: Invalid barrier level for the given option type." << std::endl;
			return { 0, 0 };
		}

		int n = old_n;
		double F[100];
		for (int m = 1; m <= 100; ++m) {
			F[m - 1] = pow(m, 2) * sigma * sigma * T / pow(log(S0 / Bar), 2);
		}

		if (old_n < F[0]) {
			std::cerr << "Increase number of steps to at least " << static_cast<int>(std::floor(F[0] + 1)) << std::endl;
		}
		else {
			for (int i = 0; i < 99; ++i) {
				if (F[i] <= old_n && old_n < F[i + 1]) {
					n = static_cast<int>(std::floor(F[i + 1]));
					break;
				}
			}
		}

		double dt = T / n;
		double u = exp(sigma * sqrt(dt));
		double d = 1 / u;
		double p = (exp(r * dt) - d) / (u - d);
		double exp_rt = exp(-r * dt);

		std::vector<std::vector<double>> optionPrice(n + 1, std::vector<double>(n + 1, 0.0));

		// Option prices at maturity
		for (int i = 1; i <= n; ++i) {
			double AssetPrice = S0 * pow(u, n + 1 - i) * pow(d, i - 1);
			if (((BarType == "DO" || BarType == "DI") && AssetPrice <= Bar) || ((BarType == "UO" || BarType == "UI") && AssetPrice >= Bar)) {
				optionPrice[i][n] = 0.0;
			}
			else if (PutCall == "Call") {
				optionPrice[i][n] = std::max(0.0, AssetPrice - K);
			}
			else if (PutCall == "Put") {
				optionPrice[i][n] = std::max(0.0, K - AssetPrice);
			}
		}

		for (int j = n - 1; j >= 1; --j) {
			for (int i = 1; i <= j; ++i) {
				double AssetPrice = S0 * pow(u, j - i) * pow(d, i - 1);
				if (((BarType == "DO" || BarType == "DI") && AssetPrice <= Bar) || ((BarType == "UO" || BarType == "UI") && AssetPrice >= Bar)) {
					optionPrice[i][j] = 0.0;
				}
				else if (EuroAmer == "Euro") {
					optionPrice[i][j] = exp_rt * (p * optionPrice[i][j + 1] + (1 - p) * optionPrice[i + 1][j + 1]);
				}
				else if (EuroAmer == "Amer") {
					if (PutCall == "Call") {
						optionPrice[i][j] = std::max(AssetPrice - K, exp_rt * (p * optionPrice[i][j + 1] + (1 - p) * optionPrice[i + 1][j + 1]));
					}
					else if (PutCall == "Put") {
						optionPrice[i][j] = std::max(K - AssetPrice, exp_rt * (p * optionPrice[i][j + 1] + (1 - p) * optionPrice[i + 1][j + 1]));
					}
				}
			}
		}
		double result = 0;
		if (BarType == "DO" || BarType == "UO") {
			result = optionPrice[1][1];
		}
		else if (EuroAmer == "Euro") {
			result = fbinomial(S0, K, r, sigma, T, n, PutCall, "Euro") - optionPrice[1][1];
		}
		else if (EuroAmer == "Amer" && PutCall == "Call" && BarType == "DI") {
			result = pow(S0 / Bar, 1 - 2 * r / (sigma * sigma)) * fbinomial(pow(Bar, 2) / S0, K, r, sigma, T, n, "Call", "Amer");
		}

		return { result, n };
}

int main() {
	double S0 = 100.0; // Initial stock price
	double K = 140.0; // Strike price
	double Bar = 95.0; // Barrier level
	double r = 0.0431; // Risk-free rate
	double sigma = 0.35; // Volatility
	double T = 1.0; // Time to maturity
	int old_n = 500; // Initial number of steps
	std::string PutCall = "Call"; // Option type (put or call)
	std::string EuroAmer = "Amer"; // Exercise style (Euro or Amer)
	std::string BarType = "DI"; // Barrier type (DO = down-and-out, DI = down-and-in, UO = up-and-out, UI = up-and-in)
	int M1 = 0; // Time interval for barrier monitoring
	std::string M2 = "D"; // Time unit for barrier monitoring (H = hourly, D = daily, W = weekly, M = monthly)

	auto result = BarrierBin(S0, K, Bar, r, sigma, T, old_n, PutCall, EuroAmer, BarType, M1, M2);

	std::cout << "Adjusted Barrier: " << Bar << std::endl;
	std::cout << "Option Price: " << result.first << std::endl;
	std::cout << "Optimal Number of Steps: " << result.second << std::endl;

	return 0;
}