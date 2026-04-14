# EGARCH(1,1) Volatility Estimation with Maximum Likelihood (C++)

## Overview

This project implements an **EGARCH(1,1)** (Exponential GARCH) model for estimating **financial time series volatility** using **maximum likelihood estimation**.

The implementation includes:

* Log-return computation from price data
* EGARCH volatility dynamics
* Parameter estimation via **Nelder–Mead optimization**
* Conditional variance and volatility extraction
* Empirical long-run volatility estimation

---

## Features

### Volatility Model

#### EGARCH(1,1) Specification

The model evolves log-variance as:

```text
log(sigma_t^2) =
    beta0
  + beta1 * log(sigma_{t-1}^2)
  + beta2 * (|z_{t-1}| - sqrt(2/pi))
  + theta * z_{t-1}
```

where:

```text
z_t = (r_t - mu) / sigma_t
r_t = log return
```

✔️ Captures:

* Volatility clustering
* Asymmetric response to shocks (leverage effect)
* Nonlinear variance dynamics

---

### Maximum Likelihood Estimation

* Parameters estimated by maximizing log-likelihood:

```text
L = -0.5 * [ log(2*pi) + log(sigma_t^2) + (epsilon_t^2 / sigma_t^2) ]
```

* Optimization performed using:

  * **Nelder–Mead simplex algorithm**

✔️ No gradients required
✔️ Robust to nonlinearity

---

### Conditional Volatility Extraction

* Computes:

```text
Variance  = sigma_t^2
Volatility = sqrt(sigma_t^2)
```

* Outputs full time series of:

  * Conditional variance
  * Conditional volatility

---

### Long-Run Volatility Estimation

Long-run variance is computed empirically from the estimated variance series:

```text
long_run_variance = average(sigma_t^2)
````

The corresponding annualized volatility is:

```text
sqrt(long_run_variance) * sqrt(252)
```

---

### Data Handling

* Reads price data from file
* Computes log returns:

```text
r_t = log(P_t / P_{t-1})
```

* Validates:

  * Positive prices
  * Sufficient data length

---

## Parameters

| Parameter | Description                              |
| --------- | ---------------------------------------- |
| `beta0`   | Constant term in log-variance equation   |
| `beta1`   | Persistence parameter                    |
| `beta2`   | Magnitude (volatility clustering) effect |
| `theta`   | Asymmetry / leverage effect              |
| `mu`      | Mean return                              |

---

## How It Works

### 1. Data Input

* Reads price series from:

```text
SP500.txt
```

---

### 2. Log Return Computation

```text
r_t = log(P_t / P_{t-1})
```

---

### 3. Initialization

* Mean (`mu`) estimated from data
* Initial variance from sample variance
* Starting parameters chosen based on typical financial data

---

### 4. Likelihood Evaluation

For each time step:

```text
epsilon_t = r_t - mu
z_t = epsilon_t / sigma_t
```

Update log-variance:

```text
log(sigma_t^2) = model equation
```

Compute log-likelihood contribution:

```text
L_t = -0.5 * [ log(2*pi) + log(sigma_t^2) + (epsilon_t^2 / sigma_t^2) ]
```

---

### 5. Optimization (Nelder–Mead)

* Builds initial simplex around parameter guess
* Iteratively updates vertices using:

  * Reflection
  * Expansion
  * Contraction
  * Shrink

✔️ Converges when function values stabilize

---

### 6. Variance & Volatility Output

* Computes full series:

```text
sigma_t^2  → variance
sqrt(sigma_t^2) → volatility
```

---

### 7. Long-Run Volatility

* Computed from average conditional variance
* Annualized using 252 trading days

---

## How to Compile

```bash
g++ -std=c++17 -O2 main.cpp -o egarch
./egarch
```

---

## Notes

* Model assumes **daily financial data**
* EGARCH avoids non-negativity constraints by modeling log-variance
* Results are sensitive to:

  * Data quality
  * Outliers
  * Parameter initialization
