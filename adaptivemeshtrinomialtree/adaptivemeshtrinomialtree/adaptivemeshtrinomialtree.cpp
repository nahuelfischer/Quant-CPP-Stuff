// Trinomial Tree with Adaptive Mesh Refinement (AMR) for Option Pricing

#include <iostream>
#include <vector>
#include <cmath>
#include <stdexcept>
#include <algorithm>
#include <print>

using namespace std;

enum class OptionType { Call, Put };
enum class ExerciseType { European, American };

struct OptionResults {
	double price = 0.0;
	double delta = 0.0;
	double gamma = 0.0;
	double theta = 0.0;
};

OptionResults AMM(double S0, double K, double T, double r, double sigma, int n, OptionType putCall, ExerciseType euroAmer, bool debug = false) {
	if (S0 <= 0 || K <= 0 || T <= 0 || sigma <= 0 || n <= 0) {
		throw invalid_argument("All input parameters must be positive.");
	}

	double dt = T / n;
	double u = exp(sigma * sqrt(3 * dt));
	double d = 1 / u;
	double drift = r - 0.5 * sigma * sigma;
	double pu = 1.0 / 6.0 + (drift * sqrt(dt)) / (2 * sigma * sqrt(3));
	double pd = 1.0 / 6.0 - (drift * sqrt(dt)) / (2 * sigma * sqrt(3));
	double pm = 2.0 / 3.0;
	double discount = exp(-r * dt);

	if (pu < 0 || pd < 0 || pm < 0 || abs(pu + pd + pm - 1) > 1e-6) {
		throw runtime_error("Invalid probabilities calculated. Check input parameters.");
	}

	int fine_steps = 8;
	double dt_fine = dt / fine_steps;
	double pu_fine = 1.0 / 6.0 + (drift * sqrt(dt_fine)) / (2 * sigma * sqrt(3));
	double pd_fine = 1.0 / 6.0 - (drift * sqrt(dt_fine)) / (2 * sigma * sqrt(3));
	double pm_fine = 2.0 / 3.0;
	double discount_fine = exp(-r * dt_fine);

	if (pu_fine < 0 || pd_fine < 0 || pm_fine < 0 || abs(pu_fine + pd_fine + pm_fine - 1) > 1e-6) {
		throw runtime_error("Invalid probabilities calculated for fine mesh. Check input parameters.");
	}

	vector<vector<double>> S(2 * n + 1, vector<double>(n + 1, 0.0));
	S[n][0] = S0;

	for (int j = 1; j <= n; ++j) {
		for (int i = n - j; i <= n + j; ++i) {
			if (i == n - j) {
				S[i][j] = S[i + 1][j - 1] * u;
			} else if (i == n + j) {
				S[i][j] = S[i - 1][j - 1] * d;
			} else {
				S[i][j] = S[i][j - 1];
			}
		}
	}

	vector<vector<double>> Op(2 * n + 1, vector<double>(n + 1, 0.0));
	for (int i = 0; i <= 2 * n; ++i) {
		if (putCall == OptionType::Call) {
			Op[i][n] = max(S[i][n] - K, 0.0);
		} else {
			Op[i][n] = max(K - S[i][n], 0.0);
		}
	}

	int locate = -1;
	for (int i = 1; i < 2 * n; ++i) {
		if (S[i][n - 1] >= K && K >= S[i + 1][n - 1]) {
			locate = i;
			break;
		}
	}
	if (locate == -1) {
		throw runtime_error("Failed to locate the strike price in the tree. Check input parameters.");
	}

	double h_fine = sigma * sqrt(3 * dt) / 3;
	vector<vector<double>> F(25, vector<double>(fine_steps + 1, 0.0));
	for (int i = 0; i < 25; ++i) {
		F[i][fine_steps] = log(S[locate][n - 1]) + (i - 12) * h_fine;
	}
	for (int j = fine_steps - 1; j >= 0; --j) {
		for (int i = 0; i < 25; ++i) {
			F[i][j] = F[i][j + 1];
		}
	}

	vector<vector<double>> expF(25, vector<double>(fine_steps + 1, 0.0));
	for (int j = 0; j <= fine_steps; ++j) {
		for (int i = 0; i < 25; ++i) {
			expF[i][j] = exp(F[i][j]);
		}
	}

	vector<vector<double>> FineOp(25, vector<double>(fine_steps + 1, 0.0));
	for (int i = 0; i < 25; ++i) {
		if (putCall == OptionType::Call) {
			FineOp[i][fine_steps] = max(expF[i][fine_steps] - K, 0.0);
		} else {
			FineOp[i][fine_steps] = max(K - expF[i][fine_steps], 0.0);
		}
	}

	for (int j = fine_steps - 1; j >= 0; --j) {
		for (int i = 1; i < 24; ++i) {
			double continuation = discount_fine * (pu_fine * FineOp[i - 1][j + 1] + pm_fine * FineOp[i][j + 1] + pd_fine * FineOp[i + 1][j + 1]);
			if (euroAmer == ExerciseType::European) {
				FineOp[i][j] = continuation;

			}
			else {
				FineOp[i][j] = (putCall == OptionType::Call) ? max(expF[i][j] - K, continuation) : max(K - expF[i][j], continuation);
			}
		}
		FineOp[0][j] = FineOp[1][j] * 0.5;
		FineOp[24][j] = FineOp[23][j] + (FineOp[23][j] - FineOp[22][j]) * 1.5;
	}

	for (int i = max(1, locate - 2); i <= min(2 * n - 1, locate + 2); ++i) {
		double s = S[i][n - 1];
		double op_val = 0.0;
		if (s < expF[0][0]) {
			op_val = FineOp[0][0];
		}
		else if (s > expF[24][0]) {
			op_val = FineOp[24][0];
		}
		else {
			for (int k = 0; k < 24; ++k) {
				if (expF[k][0] <= s && s <= expF[k + 1][0]) {
					double w = (s - expF[k][0]) / (expF[k + 1][0] - expF[k][0]);
					op_val = max((1 - w) * FineOp[k][0] + w * FineOp[k + 1][0], 0.0);
					break;
				}
					
			}
		}
		Op[i][n - 1] = op_val;
	}

	for (int i = 1; i < 2 * n; ++i) {
		if (i < locate - 2 || i > locate + 2) {
			double continuation = discount * (pu * Op[i - 1][n] + pm * Op[i][n] + pd * Op[i + 1][n]);
			Op[i][n - 1] = (euroAmer == ExerciseType::European) ? continuation : (putCall == OptionType::Call) ? max(S[i][n - 1] - K, continuation) : max(K - S[i][n - 1], continuation);
		}
	}

	for (int j = n - 2; j >= 0; --j) {
		for (int i = n - j - 1; i <= n + j + 1; ++i) {
			double continuation = discount * (pu * Op[i - 1][j + 1] + pm * Op[i][j + 1] + pd * Op[i + 1][j + 1]);
			Op[i][j] = (euroAmer == ExerciseType::European) ? continuation : (putCall == OptionType::Call) ? max(S[i][j] - K, continuation) : max(K - S[i][j], continuation);
		}
	}

	double delta = 0.0, gamma = 0.0, theta = 0.0;
	double ds = S[n - 1][1] - S[n + 1][1];
	if (abs(ds) > 1e-6) {
		delta = (Op[n - 1][1] - Op[n + 1][1]) / ds;
	}
	double ds1 = S[n - 1][1] - S[n][1];
	double ds2 = S[n][1] - S[n + 1][1];
	if (abs(ds1) > 1e-6 && abs(ds2) > 1e-6 && abs(ds) > 1e-6) {
		double delta_up = (Op[n - 1][1] - Op[n][1]) / ds1;
		double delta_down = (Op[n][1] - Op[n + 1][1]) / ds2;
		gamma = (delta_up - delta_down) / (ds / 2);
	}
	theta = (Op[n][2] - Op[n][0]) / (2 * dt);

	return { Op[n][0], delta, gamma, theta };
}

double Vega(double S0, double K, double T, double r, double sigma, int n, OptionType putCall, ExerciseType euroAmer) {
	const double change = 1e-6;
	OptionResults f = AMM(S0, K, T, r, sigma, n, putCall, euroAmer, false);
	OptionResults fs = AMM(S0, K, T, r, sigma + change, n, putCall, euroAmer, false);
	return (fs.price - f.price) / change;
}

double Rho(double S0, double K, double T, double r, double sigma, int n, OptionType putCall, ExerciseType euroAmer) {
	const double change = 1e-6;
	OptionResults f = AMM(S0, K, T, r, sigma, n, putCall, euroAmer, false);
	OptionResults fs = AMM(S0, K, T, r + change, sigma, n, putCall, euroAmer, false);
	return (fs.price - f.price) / change;
}

double Gauss(double x) {
	return 0.5 * (1.0 + erf(x / sqrt(2.0)));
}

double BlackScholes(double S0, double K, double T, double r, double sigma, int n, OptionType putCall, ExerciseType euroAmer) {
	if (euroAmer == ExerciseType::American) {
		throw invalid_argument("Black-Scholes formula is only applicable for European options.");
	}
	double d1 = (log(S0 / K) + (r + 0.5 * sigma * sigma) * T) / (sigma * sqrt(T));
	double d2 = d1 - sigma * sqrt(T);
	if (putCall == OptionType::Call) {
		return S0 * Gauss(d1) - K * exp(-r * T) * Gauss(d2);
	} else {
		return K * exp(-r * T) * Gauss(-d2) - S0 * Gauss(-d1);
	}
}

int main() {
	try {
		double S0 = 80.0;
		double K = 80.0;
		double T = 1.0;
		double r = 0.04;
		double sigma = 0.2;
		int n = 1000;
		OptionType putCall = OptionType::Call;
		ExerciseType euroAmer = ExerciseType::European;

		OptionResults results = AMM(S0, K, T, r, sigma, n, putCall, euroAmer, false);

		std::println("Option Price: {:.4f}", results.price);
		std::println("Delta: {:.4f}", results.delta);
		std::println("Gamma: {:.4f}", results.gamma);
		std::println("Theta: {:.4f}", results.theta);
		std::println("Vega: {:.4f}", Vega(S0, K, T, r, sigma, n, putCall, euroAmer));
		std::println("Rho: {:.4f}", Rho(S0, K, T, r, sigma, n, putCall, euroAmer));
		std::println("Black-Scholes Price: {:.4f}", BlackScholes(S0, K, T, r, sigma, n, putCall, euroAmer));
	}
	catch (const exception& e) {
		cerr << "Error: " << e.what() << endl;
		return 1;
	}
	return 0;
}