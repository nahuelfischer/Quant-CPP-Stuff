# Option Payoff

A modern C++ library demonstrating option pricing payoff calculations using object-oriented design patterns and polymorphism.

## Overview

This project implements a flexible framework for calculating financial option payoffs. It demonstrates core C++ concepts including inheritance, polymorphism, smart pointers, and exception handling applied to quantitative finance scenarios.

## Features

- **Polymorphic PayOff Base Class**: Extensible architecture for implementing various option types
- **Call Option Implementation**: Standard European-style call option payoff calculation
- **Double Digital Option**: Binary option that pays 1 if the spot price is within a range, 0 otherwise
- **Modern C++ Design**: Uses `std::unique_ptr` for memory safety and RAII principles
- **Error Handling**: Comprehensive exception handling for invalid inputs
- **Input Validation**: Guards against invalid parameters like negative spot prices

## Project Structure

```
optionpayoff/
├── PayOff.h                 # Base abstract class for all option types
├── PayOffCall.h             # Call option implementation
├── PayOffDoubleDigital.h    # Double digital option implementation
├── optionpayoff.cpp         # Main program demonstrating usage
└── optionpayoff.vcxproj     # Visual Studio project file
```

## Building the Project

### Prerequisites
- Visual Studio 2022 or later (Community, Professional, or Enterprise)
- C++17 or later standard

### Compilation
1. Open `optionpayoff.vcxproj` in Visual Studio
2. Build the solution (Ctrl+Shift+B)
3. Run the executable (Ctrl+F5)

### Command Line (with vcvars)
```
cl /std:c++17 optionpayoff.cpp PayOffCall.cpp PayOffDoubleDigital.cpp -o optionpayoff.exe
```

## Usage

The main program demonstrates:

```cpp
// Create a call option with strike 100
std::unique_ptr<PayOff> callPayoff = std::make_unique<PayOffCall>(100.0);
double payoff = (*callPayoff)(spotPrice);  // Calculate payoff at spot price

// Create a double digital option with barriers
std::unique_ptr<PayOff> doubleDigitalPayoff = 
    std::make_unique<PayOffDoubleDigital>(90.0, 110.0);
double payoff = (*doubleDigitalPayoff)(spotPrice);
```

### Example Output
```
Call Payoff for spot 105: 5
Double Digital Payoff for spot 105: 1
```

## Extending the Framework

To add new option types:

1. Create a new header file inheriting from `PayOff`
2. Implement the `operator()` method with your payoff logic
3. Instantiate using `std::make_unique<YourOptionType>(...)`

## Key Concepts Demonstrated

- **Polymorphism**: Base class interface with derived implementations
- **Smart Pointers**: RAII-compliant memory management
- **Operator Overloading**: Functor pattern for payoff calculation
- **Exception Safety**: Input validation and error handling
- **Modern C++ Idioms**: Move semantics, unique_ptr, const correctness

