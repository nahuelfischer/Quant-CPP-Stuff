# Leisen-Reimer Binomial Model – Cash-or-Nothing Option Pricing (C++)

## Overview

This project implements a **Leisen-Reimer (LR) binomial tree model** for pricing **cash-or-nothing (digital) options**.

It combines:

* Leisen-Reimer binomial tree → improved convergence vs standard binomial
* Backward induction → recursive valuation
* Early exercise handling → supports American options

The model supports:

* European options
* American options
* Call and Put digital (cash-or-nothing) contracts

---

## Features

### Pricing Engine

#### Leisen-Reimer Binomial Tree

* Refined binomial model designed to:

  * Improve convergence speed
  * Reduce oscillations
* Uses probability transformation based on:

  * Modified normal approximation

✔️ More accurate than standard binomial models for fewer steps

---

### Digital (Cash-or-Nothing) Payoff

* Fixed payout (`PayOff`) if the option finishes **in-the-money**
* Otherwise payoff = 0

Examples:

* Call → pays if `S_T ≥ K`
* Put → pays if `S_T < K`

---

### American Option Support

* Supports early exercise via:

```
Value = max(exercise value, continuation value)
```

✔️ Important for American-style options

---

### Dividend Yield Support

* Includes continuous dividend yield (`dividends`)
* Adjusts drift in asset evolution

---

## Parameters

| Parameter   | Description                        |
| ----------- | ---------------------------------- |
| `S0`        | Initial stock price                |
| `K`         | Strike price                       |
| `r`         | Risk-free interest rate            |
| `dividends` | Continuous dividend yield          |
| `sigma`     | Volatility                         |
| `T`         | Time to maturity (years)           |
| `PayOff`    | Fixed payout of the digital option |
| `n`         | Number of time steps               |
| `PutCall`   | `"Call"` or `"Put"`                |
| `EuroAmer`  | `"Euro"` or `"Amer"`               |
| `Method`    | Variant for probability adjustment |

---

## How It Works

### 1. Leisen-Reimer Tree Construction

The LR model modifies the binomial tree by:

* Using transformed probabilities derived from:

  * `d1` and `d2` (Black-Scholes terms)
* Improving convergence to continuous-time pricing

Steps:

1. Compute `d1`, `d2`
2. Transform into probabilities `p` and `pp`
3. Compute:

   * Up factor `u`
   * Down factor `d`

---

### 2. Asset Price Lattice

The asset price tree is constructed as:

```
S(i,j) = S0 * u^(j-i) * d^i
```

Where:

* `i` = number of down moves
* `j` = time step

---

### 3. Terminal Payoff

At maturity:

* Call:

```
PayOff if S_T ≥ K else 0
```

* Put:

```
PayOff if S_T < K else 0
```

---

### 4. Backward Induction

#### European Options

```
V = exp(-r * dt) * (p * V_up + (1 - p) * V_down)
```

#### American Options

```
V = max(exercise value, continuation value)
```

---

### 5. Odd Step Adjustment

* Ensures `n` is **odd**
* Improves stability and convergence of the LR method

---

## How to Compile

```bash
g++ -std=c++20 -O2 main.cpp -o lr_cash_option
./lr_cash_option
```

---

## Notes

* Leisen-Reimer converges faster than standard binomial trees
* Particularly effective for:

  * Digital (discontinuous payoff) options
* American options:

  * Early exercise handled explicitly
