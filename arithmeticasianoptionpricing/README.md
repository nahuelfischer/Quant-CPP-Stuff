# Monte Carlo Pricing of Arithmetic Asian Options (C++)

## Overview

This project implements a **Monte Carlo pricer for Arithmetic Asian Options** under the **Geometric Brownian Motion (GBM)** model.

It combines:

* **C++ simulation** → stochastic path generation
* **Exact GBM solution** → unbiased asset evolution
* **Path-dependent payoff evaluation** → arithmetic averaging
* **Object-oriented design** → flexible option and payoff structure

---

## Features

### Monte Carlo Simulation

* Simulation of asset price paths under GBM
* Supports:

  * Exact log-normal solution (recommended)
  * Euler–Maruyama method (for comparison)

---

### Asian Option Pricing

* Arithmetic average options:

  * Fixed-strike Asian call
  * Floating-strike Asian call

* Path-dependent payoff computation

---

### Object-Oriented Design

* Abstract base classes:

  * `PayOff`
  * `AsianOption`

* Derived implementations:

  * `PayOffCall`
  * `AsianOptionArithmetic`

---

### Random Number Generation

* Standard normal random variables
* Uses:

  * `std::mt19937`
  * `std::normal_distribution`

---

## Model Specification

The underlying asset follows **Geometric Brownian Motion**:

```
dS_t = r * S_t * dt + sigma * S_t * dW_t
```

---

### Exact Simulation Scheme

```
S_{t+dt} = S_t * exp((r - 0.5 * sigma^2) * dt + sigma * sqrt(dt) * Z)
```

Where:

```
Z ~ N(0,1)
```

---

### Euler–Maruyama Scheme (Optional)

```
S_{t+dt} = S_t * (1 + r * dt + sigma * sqrt(dt) * Z)
```

---

## Option Payoffs

### Arithmetic Average

```
S_bar = (1 / N) * sum(S_i)
```

---

### Fixed-Strike Asian Call

```
max(S_bar - K, 0)
```

---

### Floating-Strike Asian Call

```
max(S_T - S_bar, 0)
```

---

## Parameters

| Parameter  | Description           |
| ---------- | --------------------- |
| `S0`       | Initial asset price   |
| `K`        | Strike price          |
| `r`        | Risk-free rate        |
| `sigma`    | Volatility            |
| `T`        | Time to maturity      |
| `numSteps` | Time discretization   |
| `numSims`  | Number of simulations |

---

## How It Works

### 1. Path Initialization

```
assetPrices[0] = S0
```

---

### 2. Path Simulation

At each time step:

```
S_{t+dt} = S_t * exp(...)
```

---

### 3. Arithmetic Averaging

```
mean = sum(S_i) / N
```

---

### 4. Payoff Evaluation

```
max(mean - K, 0)        // fixed strike
max(S_T - mean, 0)      // floating strike
```

---

### 5. Monte Carlo Estimation

```
price = exp(-r * T) * average(payoffs)
```

---

## Project Structure

```
.
├── main.cpp
├── asian_option.h / asian_option.cpp
├── payoff.h / payoff.cpp
├── path_generation.h / path_generation.cpp
```

---

## Notes

* Exact GBM simulation is **preferred**:

  * No discretization bias
  * Ensures positive asset prices

* Euler scheme:

  * Simpler but less accurate
  * Can produce negative values

* Monte Carlo convergence:

```
O(1 / sqrt(N))
```
