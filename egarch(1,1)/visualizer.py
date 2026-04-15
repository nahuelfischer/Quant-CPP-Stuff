# Tool to visualize the outputted .csv file
# The time axis represents the days

import pandas as pd
import matplotlib.pyplot as plt

# Load CSV file
df = pd.read_csv("egarch_var_vol.csv")

# Display first few rows
print(df.head())

# Plot Variance
plt.figure()
plt.plot(df["Time"], df["Variance"])
plt.title("EGARCH Conditional Variance")
plt.xlabel("Time")
plt.ylabel("Variance")
plt.grid(True)
plt.show()

# Plot Volatility
plt.figure()
plt.plot(df["Time"], df["Volatility"])
plt.title("EGARCH Conditional Volatility")
plt.xlabel("Time")
plt.ylabel("Volatility")
plt.grid(True)
plt.show()

# Annualized Volatility
df["AnnualizedVol"] = df["Volatility"] * (252 ** 0.5)

plt.figure()
plt.plot(df["Time"], df["AnnualizedVol"])
plt.title("Annualized Volatility")
plt.xlabel("Time")
plt.ylabel("Volatility")
plt.grid(True)
plt.show()