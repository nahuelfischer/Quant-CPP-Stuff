// PayOffDOubleDigital.h
#ifndef PAYOFFDOUBLEDIGITAL_H
#define PAYOFFDOUBLEDIGITAL_H

#include <stdexcept>
#include "PayOff.h"

// Represents a double digital option payoff, returning 1 if the spot price is between lowerBarrier and upperBarrier (exclusive), and 0 otherwise.
class PayOffDoubleDigital : public PayOff {
private:
	double lowerBarrier, upperBarrier;
public:
	// Constructs a double digital payoff with specified barriers.
	// Throws std::invalid_argument if lowerBarrier > upperBarrier.
	PayOffDoubleDigital(double lower, double upper)
		: lowerBarrier(lower), upperBarrier(upper) {
		if (lower > upper) {
			throw std::invalid_argument("Lower barrier must not exceed upper barrier.");
		}
	}

	// Calculates the payoff based on the spot price.
	virtual double operator()(double spot) const noexcept override {
		return (spot > lowerBarrier && spot < upperBarrier) ? 1.0 : 0.0;
	}
};

#endif
