#define _USE_MATH_DEFINES

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>
#include <iomanip>
#include <algorithm>
#include <filesystem>

using namespace std;

// Function to compute log returns
vector<double> computeLogReturns(const vector<double>& prices) {
	vector<double> logReturns;
	if (prices.size() < 2) return logReturns;
	logReturns.resize(prices.size() - 1);
	for (size_t i = 1; i < prices.size(); ++i) {
		if (prices[i] > 0 && prices[i - 1] > 0) {
			logReturns[i - 1] = log(prices[i] / prices[i - 1]);
		}
		else {
			cerr << "Warning: Non-positive price encountered at index " << i << endl;
			return {};
		}
	}
	return logReturns;
}

// Function to read prices from a file
vector<double> readPrices(const string& filename) {
	ifstream file(filename);
	if (!file.is_open()) {
		cerr << "Error: Could not open file " << filename << endl;
		return {};
	}
	vector<double> prices;
	double price;
	while (file >> price) {
		if (price > 0) {
			prices.push_back(price);
		}
		else {
			cerr << "Warning: Non-positive price in file " << filename << endl;
		}
	}
	file.close();
	if (prices.size() < 2) {
		cerr << "Error: Insufficient data in file " << filename << endl;
		return {};
	}
	return prices;
}

// Vector mean
vector<double> VMean(const vector<vector<double>>& X) {
	if (X.empty()) return {};
	vector<double> meanX(X[0].size(), 0.0);
	for (const auto& row : X) {
		for (size_t j = 0; j < row.size(); ++j) {
			meanX[j] += row[j];
		}
	}
	for (double& val : meanX) {
		val /= X.size();
	}
	return meanX;
}

// Vector operations
vector<double> VAdd(const vector<double>& x, const vector<double>& y) {
	if (x.size() != y.size()) return {};
	vector<double> z(x.size());
	for (size_t j = 0; j < x.size(); ++j) {
		z[j] = x[j] + y[j];
	}
	return z;
}

vector<double> VSub(const vector<double>& x, const vector<double>& y) {
	if (x.size() != y.size()) return {};
	vector<double> z(x.size());
	for (size_t j = 0; j < x.size(); ++j) {
		z[j] = x[j] - y[j];
	}
	return z;
}

vector<double> VMult(const vector<double>& x, double a) {
	vector<double> z(x.size());
	for (size_t j = 0; j < x.size(); ++j) {
		z[j] = a * x[j];
	}
	return z;
}

double VecSum(const vector<double>& x) {
	double sum = 0.0;
	for (double val : x) {
		sum += val;
	}
	return sum;
}

double VecVar(const vector<double>& x) {
	if (x.empty()) return 0.0;
	double n = static_cast<double>(x.size());
	double mean = VecSum(x) / n;
	double sumV = 0.0;
	for (double val : x) {
		sumV += (val - mean) * (val - mean);
	}
	return sumV / (n - 1);
}

// Log Likelihood for EGARCH(1,1)
double LogLikelihood(const vector<double>& B, const vector<double>& prices) {
	// B = [beta0, beta1, beta2, theta, mu]
	double beta0 = B[0], beta1 = B[1], beta2 = B[2], theta = B[3], mu = B[4];

	// Stationarity constraint
	if (abs(beta1) >= 1.0) return 1e100;

	if (prices.size() < 2) return 1e100;

	vector<double> ret(prices.size() - 1);
	vector<double> logVar(prices.size() - 1);	// log(sigma_t^2)
	vector<double> var(prices.size() - 1);		// sigma_t^2
	vector<double> LogLike(prices.size() - 1);

	// Compute returns
	for (size_t i = 0; i < prices.size() - 1; ++i) {
		if (prices[i + 1] > 0 && prices[i] > 0) {
			ret[i] = log(prices[i + 1] / prices[i]);
		}
		else {
			return 1e100;
		}
	}

	// Initialize log-variance using sample variance of residuals
	vector<double> residuals(ret.size());
	for (size_t i = 0; i < ret.size(); ++i) {
		residuals[i] = ret[i] - mu;
	}
	double initVar = VecVar(residuals);
	if (initVar <= 0) return 1e100;
	logVar[0] = log(initVar);
	var[0] = exp(logVar[0]);
	LogLike[0] = -0.5 * (log(2 * M_PI) + logVar[0] + pow(residuals[0], 2) / var[0]);
	
	// Compute EGARCH log-variance and likelihood
	for (size_t i = 1; i < ret.size(); ++i) {
		double z_tm1 = residuals[i - 1] / sqrt(var[i - 1]); // epsilon_(t-1) / sigma_(t-1)
		logVar[i] = beta0 + beta1 * logVar[i - 1] + beta2 * (abs(z_tm1) - sqrt(2.0 / M_PI)) + theta * z_tm1;
		var[i] = exp(logVar[i]);
		residuals[i] = ret[i] - mu;
		LogLike[i] = -0.5 * (log(2 * M_PI) + logVar[i] + pow(residuals[i], 2) / var[i]);
	}
	return -VecSum(LogLike);
}

// Nelder-Mead Algorithm
vector<double> NelderMead(double (*f)(const vector<double>&, const vector<double>&), const vector<double>& prices, int N, double MaxIters, double Tolerance, vector<vector<double>> vertices) {
	size_t NumIters = 0;

	while (NumIters < static_cast<size_t>(MaxIters)) {
		++NumIters;
		
		// Step 0: Ordering
		vector<pair<double, size_t>> F(N + 1);
		for (size_t j = 0; j <= static_cast<size_t>(N); ++j) {
			F[j] = { f(vertices[j], prices), j };
		}
		sort(F.begin(), F.end());

		// Reorder vertices
		vector<vector<double>> y(N + 1, vector<double>(N));
		for (size_t j = 0; j <= static_cast<size_t>(N); ++j) {
			y[j] = vertices[F[j].second];
		}
		vertices = y;

		// Best and worst point
		vector<double> best = y[0];
		double f_best = F[0].first;
		double f_worst = F[N].first;

		// Centroid of N best points
		vector<double> xm = VMean(vector<vector<double>>(y.begin(), y.end() - 1));
		double fm = f(xm, prices);

		// Reflection
		vector<double> xr = VAdd(xm, VSub(xm, y[N]));
		double fr = f(xr, prices);

		if (fr < f_best) {
			// Expansion
			vector<double> xe = VAdd(xm, VMult(VSub(xr, xm), 2.0));
			double fe = f(xe, prices);
			y[N] = (fe < fr) ? xe : xr;
		}
		else if (fr < F[N - 1].first) {
			y[N] = xr;
		}
		else if (fr < f_worst) {
			// Outside contraction
			vector<double> xc = VAdd(xm, VMult(VSub(xr, xm), 0.5));
			double fc = f(xc, prices);
			if (fc <= fr) {
				y[N] = xc;
			}
			else {
				// Shrink
				for (size_t j = 1; j <= static_cast<size_t>(N); ++j) {
					y[j] = VAdd(y[0], VMult(VSub(y[j], y[0]), 0.5));
				}
			}
		}
		else {
			// Inside contraction
			vector<double> xc = VAdd(xm, VMult(VSub(y[N], xm), 0.5));
			double fc = f(xc, prices);
			if (fc < f_worst) {
				y[N] = xc;
			}
			else {
				// Shrink
				for (size_t j = 1; j <= static_cast<size_t>(N); ++j) {
					y[j] = VAdd(y[0], VMult(VSub(y[j], y[0]), 0.5));
				}
			}
		}
		
		vertices = y;

		// Convergence check
		double f_variance = 0.0;
		for (const auto& vertex : vertices) {
			double fv = f(vertex, prices);
			f_variance += (fv - fm) * (fv - fm);
		}
		f_variance /= (N + 1);
		if (sqrt(f_variance) < Tolerance) {
			break;
		}
	}

	// Return best parameters, function value, and iteration count
	vector<double> result(N + 2);
	for (int i = 0; i < N; ++i) {
		result[i] = vertices[0][i];
	}
	result[N] = f(vertices[0], prices);
	result[N + 1] = static_cast<double>(NumIters);
	return result;
}

// Function to compute EGARCH variances and volatilities
vector<pair<double, double>> computeEGARCHVarianceVolatility(const vector<double>& prices, const vector<double>& params // [beta0, beta1, beta2, theta, mu] 
) {
	vector<pair<double, double>> varVol; // Store (variance, volatility) pairs
	if (prices.size() < 2) return varVol;

	// Compute returns
	vector<double> ret(prices.size() - 1);
	for (size_t i = 0; i < prices.size() - 1; ++i) {
		if (prices[i + 1] > 0 && prices[i] > 0) {
			ret[i] = log(prices[i + 1] / prices[i]);
		}
		else {
			return varVol; // Invalid data
		}
	}

	// Parameters
	double beta0 = params[0], beta1 = params[1], beta2 = params[2], theta = params[3], mu = params[4];

	// Initialize log-variance using sample variance of residuals
	vector<double> residuals(ret.size());
	for (size_t i = 0; i < ret.size(); ++i) {
		residuals[i] = ret[i] - mu;
	}
	double initVar = VecVar(residuals);
	if (initVar <= 0) return varVol;

	vector<double> logVar(ret.size());
	vector<double> var(ret.size());
	logVar[0] = log(initVar);
	var[0] = exp(logVar[0]);
	varVol.emplace_back(var[0], sqrt(var[0]));

	// Compute EGARCH variances
	for (size_t i = 1; i < ret.size(); ++i) {
		double z_tm1 = residuals[i - 1] / sqrt(var[i - 1]);
		logVar[i] = beta0 + beta1 * logVar[i - 1] + beta2 * (abs(z_tm1) - sqrt(2.0 / M_PI)) + theta * z_tm1;
		var[i] = exp(logVar[i]);
		residuals[i] = ret[i] - mu;
		varVol.emplace_back(var[i], sqrt(var[i]));
	}

	return varVol;
}

// Function to compute long-run variance and volatility
pair<double, double> computeLongRunVarVolFromSeries(
	const vector<pair<double, double>>& varVol) {

	double sum = 0.0;
	for (const auto& vv : varVol) {
		sum += vv.first; // variance
	}

	double meanVar = sum / varVol.size();
	return { meanVar, sqrt(meanVar) };
}

int main() {
	cout << fixed << setprecision(10);

	// Read prices
	vector<double> prices = readPrices("SP500.txt");			// Change filename here
	if (prices.empty()) {
		cerr << "Error: No data found in the file." << endl;
		return 1;
	}

	// Compute and display log returns
	vector<double> logReturns = computeLogReturns(prices);
	if (logReturns.empty()) {
		cerr << "Error: Invalid log returns." << endl;
		return 1;
	}
	cout << "First 10 Log Returns:" << endl;
	for (size_t i = 0; i < min(logReturns.size(), size_t(10)); ++i) {
		cout << logReturns[i] << endl;
	}
	cout << "--------------------------------" << endl;

	// EGARCH(1,1) parameter estimation
	int N = 5; // beta0, beta1, beta2, theta, mu
	double MaxIters = 1000;
	double Tolerance = 1e-8;

	// Initial simplex: each row is a vertex [beta0, beta1, beta2, theta, mu]
	double lambda = 0.04340000;
	vector<vector<double>> s(N + 1, vector<double>(N));
	double mu0 = VecSum(logReturns) / logReturns.size();
	double var0 = VecVar(logReturns);
	double logVar0 = log(var0);

	double beta1_0 = 0.95;
	double beta0_0 = (1.0 - beta1_0) * logVar0;

	vector<double> base = {
		beta0_0,
		beta1_0,
		0.1,      // beta2
		-0.1,     // theta (negative for leverage)
		mu0
	};
	// Starting values
	double step_beta0 = 0.05 * max(1.0, abs(beta0_0));
	double step_beta1 = 0.02;
	double step_beta2 = 0.02;
	double step_theta = 0.02;
	double step_mu = 0.1 * sqrt(var0) / sqrt(logReturns.size());

	s[0] = base;

	s[1] = base; s[1][0] += step_beta0;
	s[2] = base; s[2][1] += step_beta1;
	s[3] = base; s[3][2] += step_beta2;
	s[4] = base; s[4][3] += step_theta;
	s[5] = base; s[5][4] += step_mu;

	// Run Nelder-Mead
	auto NM = NelderMead(LogLikelihood, prices, N, MaxIters, Tolerance, s);

	// Output EGARCH parameters
	cout << "EGARCH(1,1) Parameters" << endl;
	cout << "--------------------------------" << endl;
	cout << "Beta0			= " << NM[0] << endl;
	cout << "Beta1			= " << NM[1] << endl;
	cout << "Beta2			= " << NM[2] << endl;
	cout << "Theta			= " << NM[3] << endl;
	cout << "Mu			= " << NM[4] << endl;
	cout << "Persistence		= " << NM[1] << endl;
	cout << "--------------------------------" << endl;
	cout << "Log Likelihood value = " << -NM[5] << endl;
	cout << "Number of iterations = " << static_cast<int>(NM[6]) << endl;
	cout << "--------------------------------" << endl;

	// Compute and display EGARCH variances and volatilities
	vector<double> params = { NM[0], NM[1], NM[2], NM[3], NM[4] };
	auto varVol = computeEGARCHVarianceVolatility(prices, params);
	if (!varVol.empty()) {
		cout << "EGARCH Variance and Volatility Estimates (First 10):" << endl;
		cout << "--------------------------------" << endl;
		for (size_t i = 0; i < min(varVol.size(), size_t(10)); ++i) {
			cout << "t=" << i + 1 << ": Variance = " << varVol[i].first << ", Volatility = " << varVol[i].second << endl;
		}

		// Save all variance and volatility estimates to CSV
		ofstream csvOut("egarch_var_vol.csv");
		if (csvOut.is_open()) {
			csvOut << "Time,Variance,Volatility\n";
			csvOut << fixed << setprecision(10);
			for (size_t i = 0; i < varVol.size(); ++i) {
				csvOut << i + 1 << "," << varVol[i].first << "," << varVol[i].second << "\n";
			}
		}
		else {
			cerr << "Error: Could not open egarch_var_vol.csv for writing." << endl;
		}
	}
	else {
		cout << "Error: Could not compute variances." << endl;
	}

	// Compute and display long-run variance and volatility
	auto longRun = computeLongRunVarVolFromSeries(varVol);
	cout << "--------------------------------" << endl;
	cout << "Long-Run Daily Variance		= " << longRun.first << endl;
	cout << "Long-Run Daily Volatility	= " << longRun.second << endl;
	cout << "Long-Run Annualized Volatility	= " << longRun.second * sqrt(252) << endl;

	return 0;
}