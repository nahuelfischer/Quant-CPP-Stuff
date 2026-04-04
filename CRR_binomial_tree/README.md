# Binomial Option Pricing (C++)

## Overview

This project implements a **binomial tree model** in C++ to price options and compute their sensitivities (Greeks).

It supports:

* **European and American options**
* **Call and Put options**
* **Discrete dividends**
* Calculation of key Greeks:

  * Delta
  * Gamma
  * Theta
  * Vega (via finite differences)
  * Rho (via finite differences)

---

## Features

* Flexible binomial tree with configurable steps (`n`)
* Early exercise handling for **American options**
* Dividend-adjusted stock price tree
* Efficient backward induction for pricing
* Numerical estimation of Vega and Rho

---

## Parameters

The main inputs to the model are:

| Parameter   | Description                               |
| ----------- | ----------------------------------------- |
| `S0`        | Initial stock price                       |
| `K`         | Strike price                              |
| `r`         | Risk-free interest rate                   |
| `sigma`     | Volatility                                |
| `T`         | Time to maturity (in years)               |
| `n`         | Number of time steps                      |
| `PutCall`   | `"Call"` or `"Put"`                       |
| `EuroAmer`  | `"Euro"` or `"Amer"`                      |
| `Dividends` | List of dividends: `[time, amount, rate]` |

---

## How It Works

### 1. Binomial Tree Construction

The model builds a recombining tree using:

* Up factor: `u = exp(sigma * sqrt(dt))`
* Down factor: `d = 1 / u`
* Risk-neutral probability:

  ```
  p = (exp(r * dt) - d) / (u - d)
  ```

### 2. Dividend Adjustment

* Subtracts present value of dividends from the initial stock price
* Adds discounted dividends back at appropriate nodes

### 3. Option Pricing

* Computes payoff at maturity
* Uses **backward induction**:

  * European: discounted expectation
  * American: max(intrinsic, continuation value)

### 4. Greeks Calculation

* **Delta, Gamma, Theta**: extracted directly from the tree
* **Vega, Rho**: computed via finite differences

---

## Example Output

```
American Call Price :  X.XXXX
Delta               :  X.XXXX
Gamma               :  X.XXXX
Theta               :  X.XXXX
Vega                :  X.XXXX
Rho                 :  X.XXXX

American Put Price  :  X.XXXX
...
```

---

## How to Compile

Requires a C++20 (or newer) compiler due to `<format>` and `<ranges>`.

### Using g++

```bash
g++ -std=c++20 -O2 main.cpp -o binomial
./binomial
```

---

## Notes

* Large `n` (e.g., 1000) improves accuracy but increases runtime.
* American call options **without dividends** should match European call prices (no early exercise advantage).
* Finite difference step size (`1e-5`) affects Vega and Rho accuracy.

---

