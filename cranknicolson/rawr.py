import pandas as pd
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
import numpy as np

# Read the CSV file
csv_file = '.../cranknicolson/cranknicolson/surface_crank.csv'
df = pd.read_csv(csv_file)

# Filter data: TimeStep 0-100, AssetPrice 0-100 (I recommend using a range from {K - 50} to {K + 50}), OptionPrice 0-50 (You may want to adjust this range based on your specific data, but it should be sufficient for most cases)
df = df[(df['TimeStep'] >= 0) & (df['TimeStep'] <= 100) &
        (df['AssetPrice'] >= 0) & (df['AssetPrice'] <= 100) &
        (df['OptionPrice'] >= 0) & (df['OptionPrice'] <= 50)]

# Pivot to create proper grid (rows = AssetPrice, columns = TimeStep, values = OptionPrice)
pivot_df = df.pivot_table(index='AssetPrice', columns='TimeStep', values='OptionPrice', aggfunc='first')

# Extract grid data
x_grid = pivot_df.columns.values  # TimeSteps (0 to 100)
y_grid = pivot_df.index.values    # AssetPrices
z_grid = pivot_df.values          # OptionPrices

# Create meshgrid for 3D plotting
x_mesh, y_mesh = np.meshgrid(x_grid, y_grid)

# Create 3D surface plot
fig = plt.figure(figsize=(12, 8))
ax = fig.add_subplot(111, projection='3d')

# Plot surface
surf = ax.plot_surface(x_mesh, y_mesh, z_grid, cmap='viridis', alpha=0.9)

ax.set_xlabel('Time Step')
ax.set_ylabel('Asset Price')
ax.set_zlabel('Option Price')
ax.set_title('Option Price Surface')

# Add colorbar
fig.colorbar(surf, ax=ax, label='Option Price')

plt.show()
