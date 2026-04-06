# Implied Volatility Calculator (C++)

## Overview

This project computes the **implied volatility** of a **European call option** using the **Black-Scholes formula** and the **interval bisection method**.

It supports:

* Single implied volatility calculation
* Sensitivity analysis by varying:

  * Stock price
  * Strike price
  * Market option price

---

## Features

* Implementation of the **Black-Scholes pricing formula**
* Robust **bisection root-finding algorithm**
* Configurable tolerance and iteration limits
* Error handling for invalid intervals
* Ability to analyze implied volatility across parameter ranges

---

## Parameters

The main inputs are:

| Parameter     | Description                      |
| ------------- | -------------------------------- |
| `S`           | Stock price                      |
| `K`           | Strike price                     |
| `r`           | Risk-free interest rate          |
| `T`           | Time to maturity (in years)      |
| `marketPrice` | Observed market option price     |
| `sigma`       | Volatility (unknown, solved for) |

### Range Controls

| Variable               | Description                 |
| ---------------------- | --------------------------- |
| `boolSRange`           | Vary stock price if `true`  |
| `SRange`               | Range around `S`            |
| `boolKRange`           | Vary strike price if `true` |
| `KRange`               | Range around `K`            |
| `boolmarketPriceRange` | Vary market price if `true` |
| `marketPriceRange`     | Range around market price   |

⚠️ Only **one parameter range** can be varied at a time.

---

## How It Works

### 1. Black-Scholes Pricing

The European call price is computed using:

```id="6w1d0z"
C = S * N(d1) - K * exp(-rT) * N(d2)
```

where:

```id="z0g1cb"
d1 = [ln(S / K) + (r + 0.5 * σ^2) * T] / (σ * sqrt(T))
d2 = d1 - σ * sqrt(T)
```

* `N(x)` = cumulative distribution function of the standard normal distribution
* The normal CDF is approximated using `std::erf`

---

### 2. Implied Volatility

Implied volatility is found by solving:

```id="m5r2p1"
BlackScholesPrice(σ) - MarketPrice = 0
```

using the **bisection method**.

---

### 3. Bisection Method

* Starts with an interval `[lower, upper]` (default: `[0.01, 1.0]`)
* Repeatedly halves the interval
* Stops when:

  * Function value is close to zero (`tol`)
  * Interval becomes sufficiently small
* Throws an error if:

  * Root is not bracketed
  * Maximum iterations are exceeded

---

## How to Compile

Requires a C++17 (or newer) compiler.

### Using g++

```bash id="l2q8vm"
g++ -std=c++17 -O2 main.cpp -o implied_vol
./implied_vol
```

---

## Notes

* The bisection method is:

  * ✅ Stable
  * ❌ Slower than methods like Newton-Raphson

* The initial interval `[0.01, 1.0]` should bracket the true volatility

* If no sign change occurs, the solver will throw an error

* Only **European call options** are supported in this implementation
