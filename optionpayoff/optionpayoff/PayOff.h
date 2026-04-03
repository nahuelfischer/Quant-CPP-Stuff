//Payoff.h
#ifndef PAYOFF_H
#define PAYOFF_H

class PayOff {
public:
	virtual ~PayOff() {}
	virtual double operator()(double spot) const = 0;
};

#endif