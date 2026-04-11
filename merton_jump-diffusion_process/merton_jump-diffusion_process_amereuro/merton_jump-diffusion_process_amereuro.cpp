#include <random>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <chrono>
#include <iomanip>
#ifdef _OPENMP
#include <omp.h>
#endif

enum class OptionType { Call, Put };

struct OptionParams {
	double S0;
	double K;
	double T;
	double r;
	double sigma;
	OptionType type;
	int numPaths;

	// Jump params
	double lambda;		// intensity (jumps per year)
	double muJ;			// mean of ln(jump)
	double sigmaJ;		// stddev of ln(jump)
};

// Validate input parameters
void validateParams(const OptionParams& p) {
	if (p.S0 <= 0 || p.K <= 0 || p.T <= 0 || p.sigma < 0 || p.numPaths <= 0 || p.lambda < 0 || p.sigmaJ < 0) {
		throw std::invalid_argument("Invalid parameters: Non-positive or negative values detected.");
	}
}

// Cumulative standard normal distribution using erf
double normalCDF(double x) {
	return 0.5 * (1.0 + std::erf(x / std::sqrt(2.0)));
}

// Black-Scholes call price
double blackScholesCall(double S0, double K, double T, double r, double sigma) {
	if (sigma <= 0 || T <= 0) return std::max(S0 - K, 0.0);
	double d1 = (std::log(S0 / K) + (r + 0.5 * sigma * sigma) * T) / (sigma * std::sqrt(T));
	double d2 = d1 - sigma * std::sqrt(T);
	return S0 * normalCDF(d1) - K * std::exp(-r * T) * normalCDF(d2);
}

// Black-Scholes put price
double blackScholesPut(double S0, double K, double T, double r, double sigma) {
	if (sigma <= 0 || T <= 0) return std::max(K - S0, 0.0);
	double d1 = (std::log(S0 / K) + (r + 0.5 * sigma * sigma) * T) / (sigma * std::sqrt(T));
	double d2 = d1 - sigma * std::sqrt(T);
	return K * std::exp(-r * T) * normalCDF(-d2) - S0 * normalCDF(-d1);
}

// Analytical Merton Jump-Diffusion price for call and put
std::pair<double, double> mertonAnalyticalPrice(const OptionParams& p) {
	validateParams(p);

	const int nMax = 100;
	double lambdaT = p.lambda * p.T;
	double k = std::exp(p.muJ + 0.5 * p.sigmaJ * p.sigmaJ) - 1.0;
	double callPrice = 0.0;

	for (int n = 0; n <= nMax; ++n) {
		double poissonProb = std::exp(-lambdaT);
		for (int i = 1; i <= n; ++i) {
			poissonProb *= lambdaT / i;
		}
		double sigma_n = std::sqrt(p.sigma * p.sigma + n * p.sigmaJ * p.sigmaJ / p.T);
		double r_n = p.r - p.lambda * k + n * (p.muJ + 0.5 * p.sigmaJ * p.sigmaJ) / p.T;
		double bsPrice = blackScholesCall(p.S0, p.K, p.T, r_n, sigma_n);
		callPrice += poissonProb * bsPrice;
	}

	double putPrice = callPrice - p.S0 + p.K * std::exp(-p.r * p.T);
	return { callPrice, putPrice };
}

// Jump Diffusion (Merton) simulator with standard error and control variates
struct JumpDiffusionResult {
	double callPrice;
	double callSE;
	double putPrice;
	double putSE;
};

JumpDiffusionResult simulateJumpDiffusionPrice(const OptionParams& p, double bsCallControl, double bsPutControl) {
	validateParams(p);

	std::vector<double> callPayoffs(p.numPaths, 0.0);
	std::vector<double> putPayoffs(p.numPaths, 0.0);
	std::vector<double> bsCallPayoffs(p.numPaths, 0.0);
	std::vector<double> bsPutPayoffs(p.numPaths, 0.0);
	double sumCallPayoffs = 0.0, sumCallPayoffsSquared = 0.0;
	double sumPutPayoffs = 0.0, sumPutPayoffsSquared = 0.0;
	double sumBsCallPayoffs = 0.0, sumBsPutPayoffs = 0.0;
	double covCallBs = 0.0, covPutBs = 0.0;
	double varBsCall = 0.0, varBsPut = 0.0;

	double k = std::exp(p.muJ + 0.5 * p.sigmaJ * p.sigmaJ) - 1.0;

#ifdef _OPENMP
#pragma omp parallel
	{
		unsigned thread_seed = 42 + static_cast<unsigned>(omp_get_thread_num());
		std::mt19937_64 rng(thread_seed);
		std::normal_distribution<> norm(0.0, 1.0);
		std::poisson_distribution<> pois(p.lambda * p.T);
		std::normal_distribution<> jumpDist(p.muJ, p.sigmaJ);

#pragma omp for
		for (int i = 0; i < p.numPaths; ++i) {
			int N = pois(rng);
			double Z = norm(rng);
			double jumpComponent = 0.0;

			for (int j = 0; j < N; ++j) {
				jumpComponent += jumpDist(rng);
			}

			double ST = p.S0 * std::exp((p.r - 0.5 * p.sigma * p.sigma - p.lambda * k) * p.T + p.sigma * std::sqrt(p.T) * Z + jumpComponent);
			double callPayoff = std::max(ST - p.K, 0.0);
			double putPayoff = std::max(p.K - ST, 0.0);

			ST = p.S0 * std::exp((p.r - 0.5 * p.sigma * p.sigma - p.lambda * k) * p.T + p.sigma * std::sqrt(p.T) * (-Z) + jumpComponent);
			double callPayoffAnti = std::max(ST - p.K, 0.0);
			double putPayoffAnti = std::max(p.K - ST, 0.0);

			ST = p.S0 * std::exp((p.r - 0.5 * p.sigma * p.sigma) * p.T + p.sigma * std::sqrt(p.T) * Z);
			double bsCallPayoff = std::max(ST - p.K, 0.0);
			double bsPutPayoff = std::max(p.K - ST, 0.0);

			ST = p.S0 * std::exp((p.r - 0.5 * p.sigma * p.sigma) * p.T + p.sigma * std::sqrt(p.T) * (-Z));
			double bsCallPayoffAnti = std::max(ST - p.K, 0.0);
			double bsPutPayoffAnti = std::max(p.K - ST, 0.0);

			double avgCallPayoff = (callPayoff + callPayoffAnti) / 2.0;
			double avgPutPayoff = (putPayoff + putPayoffAnti) / 2.0;
			double avgBsCallPayoff = (bsCallPayoff + bsCallPayoffAnti) / 2.0;
			double avgBsPutPayoff = (bsPutPayoff + bsPutPayoffAnti) / 2.0;

			callPayoffs[i] = std::exp(-p.r * p.T) * avgCallPayoff;
			putPayoffs[i] = std::exp(-p.r * p.T) * avgPutPayoff;
			bsCallPayoffs[i] = std::exp(-p.r * p.T) * avgBsCallPayoff;
			bsPutPayoffs[i] = std::exp(-p.r * p.T) * avgBsPutPayoff;
		}
	}
#else
	std::mt19937_64 rng(42);
	std::normal_distribution<> norm(0.0, 1.0);
	std::poisson_distribution<> pois(p.lambda * p.T);
	std::normal_distribution<> jumpDist(p.muJ, p.sigmaJ);

	for (int i = 0; i < p.numPaths; ++i) {
		int N = pois(rng);
		double Z = norm(rng);
		double jumpComponent = 0.0;

		for (int j = 0; j < N; ++j) {
			jumpComponent += jumpDist(rng);
		}

		double ST = p.S0 * std::exp((p.r - 0.5 * p.sigma * p.sigma - p.lambda * k) * p.T + p.sigma * std::sqrt(p.T) * Z + jumpComponent);
		double callPayoff = std::max(ST - p.K, 0.0);
		double putPayoff = std::max(p.K - ST, 0.0);

		ST = p.S0 * std::exp((p.r - 0.5 * p.sigma * p.sigma - p.lambda * k) * p.T + p.sigma * std::sqrt(p.T) * (-Z) + jumpComponent);
		double callPayoffAnti = std::max(ST - p.K, 0.0);
		double putPayoffAnti = std::max(p.K - ST, 0.0);

		ST = p.S0 * std::exp((p.r - 0.5 * p.sigma * p.sigma) * p.T + p.sigma * std::sqrt(p.T) * Z);
		double bsCallPayoff = std::max(ST - p.K, 0.0);
		double bsPutPayoff = std::max(p.K - ST, 0.0);

		ST = p.S0 * std::exp((p.r - 0.5 * p.sigma * p.sigma) * p.T + p.sigma * std::sqrt(p.T) * (-Z));
		double bsCallPayoffAnti = std::max(ST - p.K, 0.0);
		double bsPutPayoffAnti = std::max(p.K - ST, 0.0);

		double avgCallPayoff = (callPayoff + callPayoffAnti) / 2.0;
		double avgPutPayoff = (putPayoff + putPayoffAnti) / 2.0;
		double avgBsCallPayoff = (bsCallPayoff + bsCallPayoffAnti) / 2.0;
		double avgBsPutPayoff = (bsPutPayoff + bsPutPayoffAnti) / 2.0;

		callPayoffs[i] = std::exp(-p.r * p.T) * avgCallPayoff;
		putPayoffs[i] = std::exp(-p.r * p.T) * avgPutPayoff;
		bsCallPayoffs[i] = std::exp(-p.r * p.T) * avgBsCallPayoff;
		bsPutPayoffs[i] = std::exp(-p.r * p.T) * avgBsPutPayoff;
	}
#endif

	// Aggregate results
	for (int i = 0; i < p.numPaths; ++i) {
		sumCallPayoffs += callPayoffs[i];
		sumCallPayoffsSquared += callPayoffs[i] * callPayoffs[i];
		sumPutPayoffs += putPayoffs[i];
		sumPutPayoffsSquared += putPayoffs[i] * putPayoffs[i];
		sumBsCallPayoffs += bsCallPayoffs[i];
		sumBsPutPayoffs += bsPutPayoffs[i];
		covCallBs += callPayoffs[i] * bsCallPayoffs[i];
		covPutBs += putPayoffs[i] * bsPutPayoffs[i];
		varBsCall += bsCallPayoffs[i] * bsCallPayoffs[i];
		varBsPut += bsPutPayoffs[i] * bsPutPayoffs[i];
	}

	double meanCall = sumCallPayoffs / p.numPaths;
	double meanPut = sumPutPayoffs / p.numPaths;
	double meanBsCall = sumBsCallPayoffs / p.numPaths;
	double meanBsPut = sumBsPutPayoffs / p.numPaths;

	// Original variance and standard error
	double varianceCall = (sumCallPayoffsSquared / p.numPaths - meanCall * meanCall) / (p.numPaths - 1);
	double variancePut = (sumPutPayoffsSquared / p.numPaths - meanPut * meanPut) / (p.numPaths - 1);
	double seCall = std::sqrt(varianceCall / p.numPaths);
	double sePut = std::sqrt(variancePut / p.numPaths);

	// Control variates
	covCallBs = covCallBs / p.numPaths - meanCall * meanBsCall;
	covPutBs = covPutBs / p.numPaths - meanPut * meanBsPut;
	varBsCall = varBsCall / p.numPaths - meanBsCall * meanBsCall;
	varBsPut = varBsPut / p.numPaths - meanBsPut * meanBsPut;

	double betaCall = varBsCall > 1e-10 ? covCallBs / varBsCall : 1.0;
	double betaPut = varBsPut > 1e-10 ? covPutBs / varBsPut : 1.0;

	double adjustedCallPrice = meanCall + betaCall * (bsCallControl - meanBsCall);
	double adjustedPutPrice = meanPut + betaPut * (bsPutControl - meanBsPut);

	// Use orignal SE if adjusted variance is unreliable
	return { adjustedCallPrice, seCall, adjustedPutPrice, sePut };
}

// GBM (Black-Scholes) simulator with standard error
struct GBMResult {
	double callPrice;
	double callSE;
	double putPrice;
	double putSE;
};

GBMResult simulateGBMPrice(const OptionParams& p) {
	validateParams(p);

	std::vector<double> callPayoffs(p.numPaths, 0.0);
	std::vector<double> putPayoffs(p.numPaths, 0.0);
	double sumCallPayoffs = 0.0, sumCallPayoffsSquared = 0.0;
	double sumPutPayoffs = 0.0, sumPutPayoffsSquared = 0.0;

#ifdef _OPENMP
#pragma omp parallel
	{
		unsigned thread_seed = 42 + static_cast<unsigned>(omp_get_thread_num());
		std::mt19937_64 rng(thread_seed);
		std::normal_distribution<> norm(0.0, 1.0);

#pragma omp for
		for (int i = 0; i < p.numPaths; ++i) {
			double Z = norm(rng);
			
			double ST = p.S0 * std::exp((p.r - 0.5 * p.sigma * p.sigma) * p.T + p.sigma * std::sqrt(p.T) * Z);
			double callPayoff = std::max(ST - p.K, 0.0);
			double putPayoff = std::max(p.K - ST, 0.0);

			double ST = p.S0 * std::exp((p.r - 0.5 * p.sigma * p.sigma) * p.T + p.sigma * std::sqrt(p.T) * (-Z));
			double callPayoffAnti = std::max(ST - p.K, 0.0);
			double putPayoffAnti = std::max(p.K - ST, 0.0);

			double avgCallPayoff = (callPayoff + callPayoffAnti) / 2.0;
			double avgPutPayoff = (putPayoff + putPayoffAnti) / 2.0;

			callPayoffs[i] = std::exp(-p.r * p.T) * avgCallPayoff;
			putPayoffs[i] = std::exp(-p.r * p.T) * avgPutPayoff;
		}
	}
#else
	std::mt19937_64 rng(42);
	std::normal_distribution<> norm(0.0, 1.0);

	for (int i = 0; i < p.numPaths; ++i) {
		double Z = norm(rng);

		double ST = p.S0 * std::exp((p.r - 0.5 * p.sigma * p.sigma) * p.T + p.sigma * std::sqrt(p.T) * Z);
		double callPayoff = std::max(ST - p.K, 0.0);
		double putPayoff = std::max(p.K - ST, 0.0);

		ST = p.S0 * std::exp((p.r - 0.5 * p.sigma * p.sigma) * p.T + p.sigma * std::sqrt(p.T) * (-Z));
		double callPayoffAnti = std::max(ST - p.K, 0.0);
		double putPayoffAnti = std::max(p.K - ST, 0.0);

		double avgCallPayoff = (callPayoff + callPayoffAnti) / 2.0;
		double avgPutPayoff = (putPayoff + putPayoffAnti) / 2.0;

		callPayoffs[i] = std::exp(-p.r * p.T) * avgCallPayoff;
		putPayoffs[i] = std::exp(-p.r * p.T) * avgPutPayoff;
	}
#endif

	for (int i = 0; i < p.numPaths; ++i) {
		sumCallPayoffs += callPayoffs[i];
		sumCallPayoffsSquared += callPayoffs[i] * callPayoffs[i];
		sumPutPayoffs += putPayoffs[i];
		sumPutPayoffsSquared += putPayoffs[i] * putPayoffs[i];
	}

	double meanCall = sumCallPayoffs / p.numPaths;
	double meanPut = sumPutPayoffs / p.numPaths;
	double varianceCall = (sumCallPayoffsSquared / p.numPaths - meanCall * meanCall) / (p.numPaths - 1);
	double variancePut = (sumPutPayoffsSquared / p.numPaths - meanPut * meanPut) / (p.numPaths - 1);
	double seCall = std::sqrt(varianceCall / p.numPaths);
	double sePut = std::sqrt(variancePut / p.numPaths);

	return { meanCall, seCall, meanPut, sePut };
}

int main() {
	try {
		OptionParams params{
			.S0 = 50.0,
			.K = 50.0,
			.T = 1.0,
			.r = 0.043,
			.sigma = 0.2,
			.type = OptionType::Call,
			.numPaths = 100000,
			.lambda = 0.75,
			.muJ = -0.1,
			.sigmaJ = 0.3
		};

		std::cout << std::fixed << std::setprecision(6);

		// Analytical Merton price (call and put)
		auto start = std::chrono::high_resolution_clock::now();
		auto [mertonCall, mertonPut] = mertonAnalyticalPrice(params);
		auto end = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> mertonDiff = end - start;
		std::cout << "Merton Analytical American Call Price: " << mertonCall << " (Time: " << mertonDiff.count() << " seconds)\n";
		std::cout << "Merton Analytical American Put Price: " << mertonPut << " (Time: " << mertonDiff.count() << " seconds)\n";

		// Compute Black-Scholes analytical prices for control variates
		double bsCallControl = blackScholesCall(params.S0, params.K, params.T, params.r, params.sigma);
		double bsPutControl = blackScholesPut(params.S0, params.K, params.T, params.r, params.sigma);

		// Jump-Diffusion Monte Carlo with control variates
		start = std::chrono::high_resolution_clock::now();
		auto jdResult = simulateJumpDiffusionPrice(params, bsCallControl, bsPutControl);
		end = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> jumpDiff = end - start;
		double jdCallCI_lower = jdResult.callPrice - 1.96 * jdResult.callSE;
		double jdCallCI_upper = jdResult.callPrice + 1.96 * jdResult.callSE;
		double jdPutCI_lower = jdResult.putPrice - 1.96 * jdResult.putSE;
		double jdPutCI_upper = jdResult.putPrice + 1.96 * jdResult.putSE;
		std::cout << "Jump-Diffusion MC American Call Price: " << jdResult.callPrice << " (SE: " << jdResult.callSE << ", 95% CI: [" << jdCallCI_lower << ", " << jdCallCI_upper << "], Time: " << jumpDiff.count() << " seconds)\n";
		std::cout << "Jump-Diffusion MC American Put Price: " << jdResult.putPrice << " (SE: " << jdResult.putSE << ", 95% CI: [" << jdPutCI_lower << ", " << jdPutCI_upper << "], Time: " << jumpDiff.count() << " seconds)\n";

		// GBM Monte Carlo
		start = std::chrono::high_resolution_clock::now();
		auto gbmResult = simulateGBMPrice(params);
		end = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> gbmDiff = end - start;
		double gbmCallCI_lower = gbmResult.callPrice - 1.96 * gbmResult.callSE;
		double gbmCallCI_upper = gbmResult.callPrice + 1.96 * gbmResult.callSE;
		double gbmPutCI_lower = gbmResult.putPrice - 1.96 * gbmResult.putSE;
		double gbmPutCI_upper = gbmResult.putPrice + 1.96 * gbmResult.putSE;
		std::cout << "GBM MC American Call Price: " << gbmResult.callPrice << " (SE: " << gbmResult.callSE << ", 95% CI: [" << gbmCallCI_lower << ", " << gbmCallCI_upper << "], Time: " << gbmDiff.count() << " seconds)\n";
		std::cout << "GBM MC American Put Price: " << gbmResult.putPrice << " (SE: " << gbmResult.putSE << ", 95% CI: [" << gbmPutCI_lower << ", " << gbmPutCI_upper << "], Time: " << gbmDiff.count() << " seconds)\n";

		// Validation note
		std::cout << "// Note: Prices are European (American call = European call; American put >= European put).\n";
		std::cout << "// Jump-Diffusion MC uses Control Variates with GBM prices for improved precision.\n";
	}
	catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << '\n';
		return 1;
	}
	return 0;
}