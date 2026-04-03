// Crank Nicolson Method to compute Option Prices

#include <iostream>
#include <cmath>
#include <vector>
#include <algorithm>
#include <fstream>
#include <print>

class CrankNicolson {
private:
	std::vector<std::vector<double>> S;
	std::vector<std::vector<double>> P;

public:
	CrankNicolson(int N, int M) {
		S.resize(N + 1, std::vector<double>(2 * M + 1, 0.0));
		P.resize(N + 1, std::vector<double>(2 * M + 1, 0.0));
	}

	double solveCrankNicolson(double S0, double K, double r, double dividends, double sigma, double T, int N, int M, char type, std::vector<std::vector<double>>* P_out = nullptr, std::vector<double>* S_out = nullptr) {
		S.resize(N + 1, std::vector<double>(2 * M + 1, 0.0));
		P.resize(N + 1, std::vector<double>(2 * M + 1, 0.0));

		double dt = T / N;
		double dx = sigma * std::sqrt(3 * dt / 2);

		// Build asset price grid
		for (int i = 0; i <= N; ++i) {
			for (int j = -M; j <= M; ++j) {
				S[i][j + M] = S0 * std::exp(j * dx);
			}
		}

		// Payoff at maturity
		for (int j = -M; j <= M; ++j) {
			if (type == 'C') {
				P[N][j + M] = std::max(S[N][j + M] - K, 0.0);
			}
			else {
				P[N][j + M] = std::max(K - S[N][j + M], 0.0);
			}
		}

		// Coefficients
		double sigma2 = sigma * sigma;
		double a = 0.25 * dt * (sigma2 / (dx * dx) - (r - dividends - 0.5 * sigma2) / dx);
		double b = -0.5 * dt * (sigma2 / (dx * dx) + r);
		double c = 0.25 * dt * (sigma2 / (dx * dx) + (r - dividends - 0.5 * sigma2) / dx);

		// Time-stepping 
		for (int i = N - 1; i >= 0; --i) {
			std::vector<double> lower(2 * M + 1), diag(2 * M + 1), upper(2 * M + 1), rhs(2 * M + 1);
			for (int j = -M + 1; j < M; ++j) {
				int idx = j + M;
				lower[idx] = -a;
				diag[idx] = 1 - b;
				upper[idx] = -c;
				rhs[idx] = a * P[i + 1][idx - 1] + (1 + b) * P[i + 1][idx] + c * P[i + 1][idx + 1];
			}

			// Boundary conditions (European options)
			if (type == 'C') {
				P[i][0] = 0.0;
				P[i][2 * M] = S[i][2 * M] - K * std::exp(-r * (N - i) * dt);
			}
			else if (type == 'P') {
				P[i][0] = K * std::exp(-r * (N - i) * dt);
				P[i][2 * M] = 0.0;
			}
			rhs[-M + 1 + M] -= lower[-M + 1 + M] * P[i][0];
			rhs[M - 1 + M] -= upper[M - 1 + M] * P[i][2 * M];

			// Thomas algorithm for tridiagonal system
			std::vector<double> c_star(2 * M + 1), d_star(2 * M + 1);
			int j_start = -M + 1 + M;
			int j_end = M - 1 + M;

			diag[j_start] = diag[j_start];
			c_star[j_start] = upper[j_start] / diag[j_start];
			d_star[j_start] = rhs[j_start] / diag[j_start];

			for (int j = j_start + 1; j <= j_end; ++j) {
				double m = diag[j] - lower[j] * c_star[j - 1];
				c_star[j] = upper[j] / m;
				d_star[j] = (rhs[j] - lower[j] * d_star[j - 1]) / m;
			}

			P[i][j_end] = d_star[j_end];
			for (int j = j_end - 1; j >= j_start; --j) {
				P[i][j] = d_star[j] - c_star[j] * P[i][j + 1];
			}
		}

		if (P_out && S_out) {
			*P_out = P;
			*S_out = S[0];
		}

		return P[0][M];
	}
};

int main() {
	double S0 = 50.0, K = 50.0, r = 0.06, dividends = 0.03, sigma = 0.2, T = 1.0;

	// Convergence plot data
	std::ofstream conv_file("convergence_crank.txt");
	conv_file << "Steps,Price\n";

	for (int steps = 10; steps <= 100; steps += 10) {
		CrankNicolson cn(steps, steps);
		double price = cn.solveCrankNicolson(S0, K, r, dividends, sigma, T, steps, steps, 'C'); // C for European Calls, P for European Puts
		conv_file << steps << "," << price << "\n";
	}
	conv_file.close();
	std::println("Convergence data written to convergence_crank.txt");

	// Surface plot data
	int N = 100, M = 100;
	CrankNicolson cn(N, M);
	std::vector<std::vector<double>> P_grid;
	std::vector<double> S_grid;

	double price = cn.solveCrankNicolson(S0, K, r, dividends, sigma, T, N, M, 'C', &P_grid, &S_grid); // C for European Calls, P for European Puts

	std::ofstream surface_file("surface_crank.csv");
	surface_file << "TimeStep,AssetPrice,OptionPrice\n";
	for (int i = 0; i <= N; ++i) {
		for (int j = 0; j <= 2 * M; ++j) {
			surface_file << i << "," << S_grid[j] << "," << P_grid[i][j] << "\n";
		}
	}
	surface_file.close();
  surface_file.close();
	std::println("Surface data written to surface_crank.csv");

	std::println("Option Price at S0: {}", price);

	return 0;
}

