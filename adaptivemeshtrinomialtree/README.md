# Trinomial Tree with Adaptive Mesh Refinement (AMR) – Option Pricing (C++)

## Overview

This project implements a **trinomial tree model with Adaptive Mesh Refinement (AMR)** for pricing options and computing Greeks.

It combines:

* **Trinomial tree** → stable lattice-based pricing
* **Adaptive Mesh Refinement (AMR)** → improved accuracy near the strike
* **Finite differences** → numerical Greeks

The model supports:

* European options
* American options
* Call and Put contracts

---

## Features

### Pricing Engine

* Trinomial tree (three branches per step: up, middle, down)
* Risk-neutral valuation
* Early exercise handling (American options)

---

### Adaptive Mesh Refinement (AMR)

* Refines the grid **locally near the strike price**
* Uses a **fine sub-grid** for improved accuracy
* Interpolates refined values back into the main tree

✔️ Significantly improves precision where payoff curvature is highest

---

### Greeks Computation

* Delta
* Gamma
* Theta
* Vega (finite difference)
* Rho (finite difference)

---

### Validation

* Includes **Black-Scholes formula** for European options
* Useful for benchmarking the trinomial model

---

## Parameters

| Parameter  | Description              |
| ---------- | ------------------------ |
| `S0`       | Initial stock price      |
| `K`        | Strike price             |
| `T`        | Time to maturity (years) |
| `r`        | Risk-free interest rate  |
| `sigma`    | Volatility               |
| `n`        | Number of time steps     |
| `putCall`  | Call or Put              |
| `euroAmer` | European or American     |

---

## How It Works

### 1. Trinomial Tree Construction

At each time step, the asset can move:

* Up: factor `u = exp(sigma * sqrt(3 * dt))`
* Down: factor `d = 1 / u`
* Middle: unchanged

Transition probabilities:

```id="t9x8za"
pu = 1/6 + (drift * sqrt(dt)) / (2 * sigma * sqrt(3))
pm = 2/3
pd = 1/6 - (drift * sqrt(dt)) / (2 * sigma * sqrt(3))
```

---

### 2. Adaptive Mesh Refinement (AMR)

* Identifies region near the strike price
* Builds a **fine grid in log-space**
* Performs local backward induction
* Interpolates refined values into the main tree

This improves accuracy for:

* At-the-money options
* Gamma estimation
* Early exercise boundary

---

### 3. Backward Induction

Option values are computed recursively:

* European:

```id="q8p2jm"
Value = discounted expected payoff
```

* American:

```id="3r7xnk"
Value = max(intrinsic value, continuation value)
```

---

### 4. Greeks

* **Delta**: first derivative w.r.t. stock price
* **Gamma**: second derivative
* **Theta**: time decay
* **Vega**: sensitivity to volatility (finite difference)
* **Rho**: sensitivity to interest rate (finite difference)

---

### 5. Black-Scholes Benchmark

For European options:

```id="s1z9dm"
C = S * N(d1) - K * exp(-rT) * N(d2)
```

Used to validate numerical results.

---

## How to Compile

Requires a modern C++ compiler (C++20+ recommended due to `<print>`).

```bash id="y6m2bt"
g++ -std=c++20 -O2 main.cpp -o amm_option
./amm_option
```

---

## Notes

* AMR improves accuracy but increases computational cost

* The method is particularly useful near:

  * Strike price
  * Early exercise boundary

* Finite difference Greeks depend on step size:

  * Too large → inaccurate
  * Too small → numerical instability

* Black-Scholes is only valid for:

  * European options
  * No dividends
