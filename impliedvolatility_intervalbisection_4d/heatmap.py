
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D

# Load CSV
file_path = "xxx/impliedvolatility_intervalbisection/implied_vol_surface.csv"
df = pd.read_csv(file_path)

# Drop NaN values (failed bisection cases)
df = df.dropna()

# =========================
# 3D SCATTER (Color = Implied Volatility)
# =========================

# Extract axes
X = df["StrikePrice"]
Y = df["StockPrice"]
Z = df["MarketPrice"]
V = df["ImpliedVolatility"]  # color

# Create 3D scatter plot
fig = plt.figure()
ax = fig.add_subplot(111, projection='3d')

sc = ax.scatter(X, Y, Z, c=V)

# Labels
ax.set_xlabel("Strike Price")
ax.set_ylabel("Stock Price")
ax.set_zlabel("Market Price")
ax.set_title("Implied Volatility Surface (Color = Volatility)")

# Colorbar (heatmap legend)
cbar = plt.colorbar(sc)
cbar.set_label("Implied Volatility")

plt.show()

# =========================
# 3D SURFACE (Fix Market Price)
# =========================

# Choose a fixed market price slice
fixed_price = df["MarketPrice"].median()
slice_df = df[np.isclose(df["MarketPrice"], fixed_price)]

# Pivot for surface
pivot = slice_df.pivot(index="StockPrice", columns="StrikePrice", values="ImpliedVolatility")

X, Y = np.meshgrid(pivot.columns, pivot.index)
Z = pivot.values

# Plot 3D surface
fig = plt.figure()
ax = fig.add_subplot(111, projection='3d')

ax.plot_surface(X, Y, Z)

ax.set_xlabel("Strike Price")
ax.set_ylabel("Stock Price")
ax.set_zlabel("Implied Volatility")
ax.set_title(f"Implied Vol Surface (Market Price ≈ {fixed_price:.2f})")

plt.show()

# =========================
# 2D SMILE (Fix Stock Price)
# =========================

fixed_stock = df["StockPrice"].median()
smile_df = df[np.isclose(df["StockPrice"], fixed_stock) &
              np.isclose(df["MarketPrice"], fixed_price)]

plt.figure()
plt.plot(smile_df["StrikePrice"], smile_df["ImpliedVolatility"])

plt.xlabel("Strike Price")
plt.ylabel("Implied Volatility")
plt.title(f"Volatility Smile (S ≈ {fixed_stock:.2f}, P ≈ {fixed_price:.2f})")

plt.grid()
plt.show()
