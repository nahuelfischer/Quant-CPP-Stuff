# Merton Jump-Diffusion & Monte Carlo Option Pricing (C++)

## Overview

This project implements option pricing under both the **Black-Scholes (GBM)** model and the **Merton Jump-Diffusion model** using analytical formulas and Monte Carlo simulation.

It combines:

* **Black-Scholes model (GBM)** → closed-form benchmark
* **Merton Jump-Diffusion model** → incorporates discontinuous price jumps
* **Monte Carlo simulation** → flexible numerical pricing
* **Variance reduction techniques** → improved efficiency and accuracy

The model supports:

* European Call and Put options
* Analytical and simulation-based pricing
* Confidence interval estimation

---

## Features

### Pricing Models

#### Black-Scholes (GBM)

* Analytical pricing for calls and puts
* Used as:

  * Benchmark
  * Control variate in Monte Carlo

#### Merton Jump-Diffusion

* Extends GBM by adding **Poisson-driven jumps**
* Captures:

  * Market shocks
  * Heavy tails
  * Skewness in returns

---

### Monte Carlo Simulation

#### Jump-Diffusion Simulation

* Simulates asset paths with:

  * Continuous diffusion (Brownian motion)
  * Discrete jumps (Poisson process)
* Uses:

  * **Antithetic variates** → reduces variance
  * **Control variates (Black-Scholes)** → improves convergence

✔️ Produces:

* Option price estimates
* Standard errors
* 95% confidence intervals

---

### GBM Monte Carlo

* Standard Monte Carlo under Black-Scholes assumptions
* Includes:

  * Antithetic variates
  * Standard error estimation

---

### Parallelization

* Optional **OpenMP support**
* Multi-threaded simulation for faster execution

---

### Validation

* Analytical Merton price used as reference
* Black-Scholes used for:

  * Benchmarking
  * Control variates

---

## Parameters

| Parameter  | Description                              |
| ---------- | ---------------------------------------- |
| `S0`       | Initial stock price                      |
| `K`        | Strike price                             |
| `T`        | Time to maturity (years)                 |
| `r`        | Risk-free interest rate                  |
| `sigma`    | Volatility                               |
| `numPaths` | Number of Monte Carlo simulations        |
| `lambda`   | Jump intensity (expected jumps per year) |
| `muJ`      | Mean of log jump size                    |
| `sigmaJ`   | Std dev of log jump size                 |

---

## How It Works

### 1. Black-Scholes Model

The standard GBM model assumes:

* Log-normal asset dynamics
* Continuous paths
* No jumps

Used for:

* Analytical pricing
* Control variate baseline

---

### 2. Merton Jump-Diffusion Model

The asset evolves as:

* GBM diffusion

* Plus random jumps:

* Jump arrivals follow a **Poisson process**

* Jump sizes follow a **log-normal distribution**

✔️ Captures real market behavior better than pure GBM

---

### 3. Analytical Merton Pricing

* Uses a **Poisson-weighted sum of Black-Scholes prices**
* Truncates at a finite number of jumps (e.g., 100)

Steps:

1. Compute probability of `n` jumps
2. Adjust volatility and drift
3. Apply Black-Scholes formula
4. Sum contributions

---

### 4. Monte Carlo Simulation

#### Jump-Diffusion Paths

For each simulation:

1. Sample number of jumps
2. Sample jump magnitudes
3. Simulate diffusion component
4. Combine into terminal price

---

### 5. Variance Reduction

#### Antithetic Variates

* Uses both `Z` and `-Z`
* Reduces variance of estimators

#### Control Variates

* Uses Black-Scholes payoff as control

* Adjusts estimator:

* Improves convergence significantly

* Reduces simulation noise

---

### 6. Statistical Output

Each simulation reports:

* Estimated price
* Standard error (SE)
* 95% confidence interval:

[
\text{Price} \pm 1.96 \times SE
]

---

## How to Compile

### Without OpenMP

```
g++ -std=c++20 -O2 main.cpp -o option_pricing
./option_pricing
```

### With OpenMP (recommended)

```
g++ -std=c++20 -O2 -fopenmp main.cpp -o option_pricing
./option_pricing
```

---

## Notes

* Monte Carlo accuracy depends on `numPaths`

  * Higher → more accurate but slower
* Control variates significantly reduce variance
* Jump-Diffusion is more realistic than GBM for:

  * Sudden market moves
  * Fat-tailed distributions

---

## Limitations

* Despite labels in output, pricing is **European-style**

  * No early exercise logic implemented
* Merton model assumes:

  * Constant jump intensity
  * Log-normal jump sizes
* Performance depends on:

  * Number of paths
  * Parallelization
