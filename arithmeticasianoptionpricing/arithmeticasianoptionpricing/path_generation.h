#pragma once

#include <vector>
#include <cmath>
#include <random>

// Generate random numbers
double random() noexcept;

// Generate GBM asset price path using the Euler-Maruyama method
void calcPathAssetPrices(std::vector<double>& assetPrices, double r, double sigma, double T) noexcept;

// Generate GBM asset price path using the Exact GBM solution
void calcPathAssetPricesExact(std::vector<double>& assetPrices, double r, double sigma, double T) noexcept;
