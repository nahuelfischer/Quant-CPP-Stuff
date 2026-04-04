# Implied Volatility Surface & Visualization (C++ & Python)

## Overview

This project computes and visualizes an **implied volatility surface** for European call options.

It combines:

* **C++** → numerical computation (Black-Scholes + bisection)
* **Python** → visualization (heatmap scatter, 3D surface and smile)

---

## Features

### C++ (Computation)

* Black-Scholes pricing for European call options
* Implied volatility via **bisection method**
* Generates a **3D volatility dataset**:

  * Stock price (S)
  * Strike price (K)
  * Market price (P)
* Stores results in a **3D matrix (`std::vector`)**
* Exports to CSV

### Python (Visualization)

* 3D scatter plot:

  * Axes: (Strike, Stock, Market Price)
  * Color: Implied volatility (heatmap-style)
* 3D surface plot (fixed market price slice)
* 2D volatility smile

---

## Data Structure

The generated CSV file:

```id="t3x8pl"
StockPrice,StrikePrice,MarketPrice,ImpliedVolatility
```

Each row represents one point in the 4D space:

[
(S, K, P) \rightarrow \sigma
]

---

## How It Works

### 1. Black-Scholes Pricing

The option price is computed using:

[
C = S \cdot N(d_1) - K e^{-rT} \cdot N(d_2)
]

---

### 2. Implied Volatility

Volatility is obtained by solving:

[
\text{BS}(S,K,\sigma) - P_{market} = 0
]

using the **bisection method**.

---

### 3. Volatility Surface Construction

The program evaluates implied volatility over a grid:

* Stock prices (S range)
* Strike prices (K range)
* Market prices (P range)

Result:

* A **3D grid of inputs**
* A **4th dimension (volatility)** stored as values

---

## Visualization

### 1. 3D Heatmap Scatter (Full Surface)

* X → Strike price
* Y → Stock price
* Z → Market price
* Color → Implied volatility

This preserves the full **4D structure** of the data.

---

### 2. 3D Surface (Slice)

* Fixes market price
* Plots:

  * X → Strike
  * Y → Stock
  * Z → Volatility

---

### 3. Volatility Smile

* Fixes stock price and market price
* Plots:

  * X → Strike
  * Y → Volatility

---

## Example Output

### CSV

```id="2wq9ka"
X,X,X,X
X,X,X,X
...
```

### Visualizations

* 3D scatter heatmap
* 3D surface (slice)
* 2D smile curve

---

## How to Compile (C++)

```bash id="b8d9sx"
g++ -std=c++17 -O2 main.cpp -o implied_vol_surface
./implied_vol_surface
```

This generates:

```id="7q3mzt"
implied_vol_surface.csv
```

---

## How to Run (Python)

### Install dependencies

```bash id="l0a3vx"
pip install pandas numpy matplotlib plotly
```

### Run visualization

```bash id="m4r2kn"
python plot_surface.py
```

---

## Notes

* The dataset is **4-dimensional**, so:

  * Surface plots require slicing
  * Scatter plots show full structure
* `NaN` values indicate:

  * No valid implied volatility
  * Root-finding failure
* Bisection is:

  * ✅ Stable
  * ❌ Slower than Newton methods


