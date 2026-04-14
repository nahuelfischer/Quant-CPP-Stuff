# Tool to convert the historical data of an asset from a .csv file into a .txt file
# It is made for data sets downloaded from nasdaq.com

input_file = "historical_data.csv"
output_file = "closing_prices.txt"

with open(input_file, "r") as infile, open(output_file, "w") as outfile:
    for line in infile:
        # Remove whitespace/newline and split by comma
        values = line.strip().split(",")
        
        # Check if line has at least 2 values
        if len(values) >= 2:
            closing_price = values[1]
            
            # Write closing price to output file
            outfile.write(closing_price + "\n")

print("Closing prices extracted to closing_prices.txt")