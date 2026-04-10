#include "path_generation.h"

// Generate random numbers
double random() noexcept {
	static std::random_device rd;
	static std::mt19937 gen(rd());
	static std::normal_distribution<double> dist(0.0, 1.0);
	return dist(gen);
}

// Generate GBM asset price path using the Euler-Maruyama method
void calcPathAssetPrices(std::vector<double>& assetPrices, double r, double sigma, double T) noexcept {
	std::size_t numSteps = assetPrices.size();
	double dt = T / static_cast<double>(numSteps - 1);
	for (std::size_t i = 1; i < numSteps; ++i) {
		double Z = random();
		assetPrices[i] = assetPrices[i - 1] * (1 + r * dt + sigma * std::sqrt(dt) * Z);
	}
}

// Generate GBM asset price path using the Exact GBM solution
void calcPathAssetPricesExact(std::vector<double>& assetPrices, double r, double sigma, double T) noexcept {
	std::size_t numSteps = assetPrices.size();
	double dt = T / static_cast<double>(numSteps - 1);
	for (std::size_t i = 1; i < numSteps; ++i) {
		double Z = random();
		assetPrices[i] = assetPrices[i - 1] * std::exp((r - 0.5 * sigma * sigma) * dt + sigma * Z * std::sqrt(dt));
	}
}