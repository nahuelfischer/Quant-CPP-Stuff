import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D

# Load CSV
file_path = "xxx/impliedvolatility_newton-raphson/impliedvolatility_newton-raphson/implied_vol_surface.csv"
df = pd.read_csv(file_path)

# Clean data
df = df.dropna()

# =========================
# 3D SURFACE (Strike vs Time)
# =========================

# Pivot table for surface
# Create pivot table (handles duplicates safely)
pivot = df.pivot_table(
    index="Time",
    columns="StrikePrice",
    values="ImpliedVolatility",
    aggfunc="mean"   # safe fallback
)

# Convert to grid
X, Y = np.meshgrid(pivot.columns, pivot.index)
Z = pivot.values

# Plot surface
fig = plt.figure()
ax = fig.add_subplot(111, projection='3d')

ax.plot_surface(X, Y, Z)

ax.set_xlabel("Strike Price")
ax.set_ylabel("Time to Maturity")
ax.set_zlabel("Implied Volatility")
ax.set_title("Implied Volatility Surface (Newton-Raphson)")

plt.show()

# =========================
# 2D SMILE (Fixed Maturity)
# =========================

# Get unique maturities (sorted)
times = np.sort(df["Time"].unique())

# Select a few representative maturities (e.g. 6 evenly spaced)
num_plots = 6
selected_times = np.linspace(0, len(times) - 1, num_plots, dtype=int)
selected_times = times[selected_times]

# Create subplots
fig, axes = plt.subplots(2, 3, figsize=(12, 8))
axes = axes.flatten()

for i, t in enumerate(selected_times):
    ax = axes[i]
    
    # Extract smile
    smile_df = df[df["Time"] == t].sort_values("StrikePrice")
    
    ax.plot(smile_df["StrikePrice"], smile_df["ImpliedVolatility"])
    
    ax.set_title(f"T = {t:.2f}")
    ax.set_xlabel("Strike")
    ax.set_ylabel("Vol")
    ax.grid()

# Adjust layout
plt.suptitle("Volatility Smiles Across Maturities")
plt.tight_layout()

plt.show()

# =========================
# HEATMAP (2D color plot)
# =========================

plt.figure()

plt.imshow(
    Z,
    aspect='auto',
    origin='lower',
    extent=[pivot.columns.min(), pivot.columns.max(),
            pivot.index.min(), pivot.index.max()]
)

plt.xlabel("Strike Price")
plt.ylabel("Time to Maturity")
plt.title("Implied Volatility Heatmap")

cbar = plt.colorbar()
cbar.set_label("Implied Volatility")

plt.show()
