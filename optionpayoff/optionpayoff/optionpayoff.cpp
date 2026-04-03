#include <iostream>
#include <memory>
#include <stdexcept>
#include "PayOff.h"
#include "PayOffCall.h"
#include "PayOffDoubleDigital.h"

int main() {
	try {
		// Input parameters
		const double spotPrice = 105.0;
		const double strikePrice = 100.0;
		const double lowerBarrier = 90.0;
		const double upperBarrier = 110.0;

		// Validate spot price
		if (spotPrice < 0) {
			throw std::invalid_argument("Spot price cannot be negative.");
		}

		// Create a call option with strike 100
		std::unique_ptr<PayOff> callPayoff = std::make_unique<PayOffCall>(strikePrice);
		std::cout << "Call Payoff for spot " << spotPrice << ": "
			<< (*callPayoff)(spotPrice) << std::endl;

		// Create a double digital option with barriers 90 and 110
		std::unique_ptr<PayOff> doubleDigitalPayoff = std::make_unique<PayOffDoubleDigital>(lowerBarrier, upperBarrier);
		std::cout << "Double Digital Payoff for spot " << spotPrice << ": "
			<< (*doubleDigitalPayoff)(spotPrice) << std::endl;
	
	} catch (const std::invalid_argument& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	} catch (const std::exception& e) {
		std::cerr << "Unexpected error: " << e.what() << std::endl;
		return 1;
	} catch (...) {
		std::cerr << "An unknown error occurred." << std::endl;
		return 1;
	}

	return 0;
}