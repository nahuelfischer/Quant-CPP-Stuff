// PayOffCall.h
#ifndef PAYOFFCALL_H
#define PAYOFFCALL_H

#include "PayOff.h"

class PayOffCall : public PayOff {
private:
	double strike;
public:
	PayOffCall(double K) : strike(K) {}

	virtual double operator()(double spot) const override {
		return (spot > strike) ? (spot - strike) : 0.0;
	}
};

#endif
