// This code calculates the implied volatility of a European call option using the Black-Scholes formula and the interval bisection method.

#include <iostream>
#include <cmath>
#include <functional>
#include <stdexcept>
#include <vector>
#include <fstream>

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
    // Base parameters
    double S0 = 80.0;
    double K0 = 80.0;
    double r = 0.04;
    double T = 1.0;
    double marketPrice0 = 10.0;

    // Ranges
    double SRange = 10.0;
    double KRange = 10.0;
    double priceRange = 5.0;

    double S_step = 5.0;
    double K_step = 5.0;
    double P_step = 1.0;

    // Containers for axes
    std::vector<double> S_values, K_values, P_values;

    for (double S = S0 - SRange; S <= S0 + SRange; S += S_step)
        S_values.push_back(S);

    for (double K = K0 - KRange; K <= K0 + KRange; K += K_step)
        K_values.push_back(K);

    for (double P = marketPrice0 - priceRange; P <= marketPrice0 + priceRange; P += P_step)
        P_values.push_back(P);

    // 3D surface: [S][K][P]
    std::vector<std::vector<std::vector<double>>> surface(
        S_values.size(),
        std::vector<std::vector<double>>(
            K_values.size(),
            std::vector<double>(P_values.size(), 0.0)
        )
    );

    // Compute implied vol surface
    for (size_t i = 0; i < S_values.size(); ++i) {
        for (size_t j = 0; j < K_values.size(); ++j) {
            for (size_t k = 0; k < P_values.size(); ++k) {

                double S = S_values[i];
                double K = K_values[j];
                double marketPrice = P_values[k];

                auto f = [=](double sigma) {
                    return blackScholesCall(S, K, r, sigma, T) - marketPrice;
                    };

                try {
                    surface[i][j][k] = bisection(f, 0.01, 1.0);
                }
                catch (...) {
                    surface[i][j][k] = NAN; // mark failure
                }
            }
        }
    }

    // Export to CSV
    std::ofstream file("implied_vol_surface.csv");
    file << "StockPrice,StrikePrice,MarketPrice,ImpliedVolatility\n";

    for (size_t i = 0; i < S_values.size(); ++i) {
        for (size_t j = 0; j < K_values.size(); ++j) {
            for (size_t k = 0; k < P_values.size(); ++k) {
                file << S_values[i] << ","
                    << K_values[j] << ","
                    << P_values[k] << ","
                    << surface[i][j][k] << "\n";
            }
        }
    }

    file.close();

    std::cout << "Implied volatility surface exported to implied_vol_surface.csv\n";

    return 0;
}