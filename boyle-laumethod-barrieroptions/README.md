# Binomial Barrier Option Pricing with Boyle–Lau Adjustment (C++)

## Overview

This project implements a **binomial tree model (Cox-Ross-Rubinstein)** for pricing **barrier options**, enhanced with:

* **Boyle–Lau step adjustment** → improved barrier alignment
* **Barrier shifting correction** → reduces discrete monitoring bias
* **American and European option support**

The model handles:

* Vanilla options (for benchmarking)
* Barrier options:

  * Down-and-Out (DO)
  * Down-and-In (DI)
  * Up-and-Out (UO)
  * Up-and-In (UI)

---

## Features

### Pricing Engine

#### Binomial Tree (CRR Model)

* Standard Cox-Ross-Rubinstein tree:

  * Up factor: `u = exp(sigma * sqrt(dt))`
  * Down factor: `d = 1 / u`
  * Risk-neutral probability `p`

* Supports:

  * European options
  * American options (early exercise)

---

### Barrier Option Handling

* At each node:

  * If barrier is breached → option value = 0
* Otherwise:

  * Standard backward induction

✔️ Works for both **knock-out** and **knock-in (via parity/transformations)**

---

### Barrier Adjustment (Discrete Monitoring Correction)

* Applies correction:

```text
Barrier ← Barrier * exp(±0.5826 * sigma * sqrt(dt))
```

✔️ Purpose:

* Accounts for difference between:

  * Continuous barrier monitoring
  * Discrete tree observation

---

### Boyle–Lau Step Adjustment

* Adjusts number of time steps `n` to better align barrier with tree nodes

Core idea:

```text
n ≈ (m² * sigma² * T) / (log(S0 / Barrier))²
```

✔️ Improves:

* Accuracy of barrier detection
* Stability of pricing

---

### Vanilla Option Benchmark

* Includes standard binomial pricing (`fbinomial`)
* Used for:

  * Validation
  * In-out parity for barrier options

---

## Parameters

| Parameter  | Description                                  |
| ---------- | -------------------------------------------- |
| `S0`       | Initial stock price                          |
| `K`        | Strike price                                 |
| `Bar`      | Barrier level                                |
| `r`        | Risk-free interest rate                      |
| `sigma`    | Volatility                                   |
| `T`        | Time to maturity (years)                     |
| `old_n`    | Initial number of time steps                 |
| `PutCall`  | `"Call"` or `"Put"`                          |
| `EuroAmer` | `"Euro"` or `"Amer"`                         |
| `BarType`  | `"DO"`, `"DI"`, `"UO"`, `"UI"`               |
| `M1`       | Monitoring frequency (integer steps)         |
| `M2`       | Monitoring unit (`"H"`, `"D"`, `"W"`, `"M"`) |

---

## How It Works

### 1. Step Adjustment (Boyle–Lau)

* Computes candidate values for `n`
* Selects an adjusted `n` to improve barrier alignment

---

### 2. Barrier Correction

* Adjusts barrier level using:

  * Volatility
  * Monitoring frequency

---

### 3. Tree Construction

* Builds binomial lattice:

```text
S(i,j) = S0 * u^(j-i) * d^i
```

---

### 4. Terminal Payoff

If barrier is breached:

```text
Value = 0
```

Otherwise:

* Call: `max(S - K, 0)`
* Put: `max(K - S, 0)`

---

### 5. Backward Induction

#### European Options

```text
V = exp(-r * dt) * (p * V_up + (1 - p) * V_down)
```

#### American Options

```text
V = max(intrinsic value, continuation value)
```

---

### 6. Knock-In Handling

Uses:

```text
Knock-In = Vanilla − Knock-Out   (European only)
```

Additional transformation used for:

* American down-and-in calls

---

## How to Compile

```bash
g++ -std=c++20 -O2 main.cpp -o barrier_option
./barrier_option
```

---

## Notes

* Accuracy depends on:

  * Number of steps `n`
  * Barrier proximity to spot
* Boyle–Lau adjustment improves:

  * Node alignment
  * Pricing stability
* Barrier shifting reduces:

  * Discrete monitoring bias
