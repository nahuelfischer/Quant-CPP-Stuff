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

[
C = S \cdot N(d_1) - K e^{-rT} \cdot N(d_2)
]

where:

* ( d_1 = \frac{\ln(S/K) + (r + \frac{1}{2}\sigma^2)T}{\sigma \sqrt{T}} )
* ( d_2 = d_1 - \sigma \sqrt{T} )

The normal CDF is approximated using `std::erf`.

---

### 2. Implied Volatility

Implied volatility is found by solving:

[
\text{BlackScholesPrice}(\sigma) - \text{MarketPrice} = 0
]

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

## Example Output

### Single Calculation

```id="g7s8kd"
Implied Volatility: X.XXXX
```

### Varying Stock Price

```id="p9x2lm"
Stock Price: 70, Implied Volatility: X.XXXX
Stock Price: 75, Implied Volatility: X.XXXX
Stock Price: 80, Implied Volatility: X.XXXX
...
```

---

## How to Compile

Requires a C++17 (or newer) compiler.

### Using g++

```bash id="k2m4vn"
g++ -std=c++17 -O2 main.cpp -o implied_vol
./implied_vol
```

---

## Notes

* The bisection method is **stable but slower** than methods like Newton-Raphson.
* The initial interval `[0.01, 1.0]` should bracket the true volatility.
* If no sign change occurs, the solver will throw an error.
* Only **European call options** are supported in this implementation.
