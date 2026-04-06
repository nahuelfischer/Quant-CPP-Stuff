// Implied volatility calculation using the Newton-Raphson Method

#include <iostream>
#include <cmath>
#include <functional>
#include <stdexcept>
#include <boost/math/distributions/normal.hpp>
#include <boost/math/tools/roots.hpp>
#include <boost/version.hpp>
#include <limits>
#include <vector>
#include <fstream>
#include <iomanip>

// Black-Scholes call option
double blackScholesCall(double S, double K, double r, double sigma, double T) {
	if (sigma <= 0 || S <= 0 || K <= 0 || T <= 0)
		throw std::runtime_error("Invalid inputs");

	double d1 = (std::log(S / K) + (r + 0.5 * sigma * sigma) * T) / (sigma * std::sqrt(T));
	double d2 = d1 - sigma * std::sqrt(T);

	boost::math::normal dist(0.0, 1.0);
	return S * boost::math::cdf(dist, d1) - K * std::exp(-r * T) * boost::math::cdf(dist, d2);
}

// Vega
double blackScholesVega(double S, double K, double r, double sigma, double T) {
	double d1 = (std::log(S / K) + (r + 0.5 * sigma * sigma) * T) / (sigma * std::sqrt(T));
	boost::math::normal dist(0.0, 1.0);
	return S * boost::math::pdf(dist, d1) * std::sqrt(T);
}

// Functor returning both function value and derivative
struct ImpliedVolFunc {
	double S, K, r, T, marketPrice;
	ImpliedVolFunc(double S_, double K_, double r_, double T_, double marketPrice_) : S(S_), K(K_), r(r_), T(T_), marketPrice(marketPrice_) {
	}
	std::pair<double, double> operator()(double sigma) const {
		double price = blackScholesCall(S, K, r, sigma, T);
		double vega = blackScholesVega(S, K, r, sigma, T);
		return { price - marketPrice, vega }; // f(x) and f'(x)
	}
};

int main() {
	double S = 80.0;
	double K = 80.0;
	double r = 0.04;
	double T = 2.0;
	double marketPrice = 10.0;

	double guess = 0.2;
	double min = 0.01;
	double max = 1.0;
    
    double T_start = 0.3;
    double T_step = 0.1;
    double K_range = 5;
    double K_step = 1.0;

    // Containers for axes
    std::vector<double> T_values;
    std::vector<double> K_values;

    // Build maturity grid around original T
    for (double t = T_start; t <= T; t += T_step) {
        if (t > 0.0) // Only positive check
            T_values.push_back(t);
    }

    // Build strike grid around original K
    for (double k = K - K_range; k <= K + K_range; k += K_step) {
        if (k > 0.0) // Only positive check
            K_values.push_back(k);
    }

    // 2D surface: [T_index][K_index]
    std::vector<std::vector<double>> surface(
        T_values.size(),
        std::vector<double>(K_values.size(), 0.0)
    );

    // Compute implied vol surface
    for (size_t i = 0; i < T_values.size(); ++i) {
        for (size_t j = 0; j < K_values.size(); ++j) {
            double t_val = T_values[i];
            double k_val = K_values[j];
            try {
                ImpliedVolFunc func(S, k_val, r, t_val, marketPrice);

                // Newton-Raphson method
                double sigma = boost::math::tools::newton_raphson_iterate(
                func,                                       // Functor returning f(x) and f'(x)
                guess,                                      // Initial guess
                min,                                        // Lower bound
                max,                                        // Upper bound
                std::numeric_limits<double>::digits / 2     // Precision (binary digits)
                );

                surface[i][j] = sigma;
            }  
            catch (const std::exception& e) {
                std::cerr << "Error: " << e.what() << std::endl;
                surface[i][j] = std::numeric_limits<double>::quiet_NaN();
            }
        }
    }

    // Export to CSV
    std::ofstream file("implied_vol_surface.csv");
    file << "StrikePrice,Time,ImpliedVolatility\n";

    for (size_t i = 0; i < T_values.size(); ++i) {
        for (size_t j = 0; j < K_values.size(); ++j) {
            file    << K_values[j] << ","
                    << T_values[i] << ","
                    << surface[i][j] << "\n";
        }
    }

    file.close();

    std::cout << "Implied volatility surface exported to implied_vol_surface.csv\n";

    return 0;
}
