#pragma once

#include <algorithm>
#include <memory>

class PayOff {
protected:
	double K; // strike price

public:
	PayOff() = default;
	virtual ~PayOff() = default;

	// Pure virtual function to calculate the payoff
	[[nodiscard]] virtual double computeFixed(double mean) const noexcept = 0;
	[[nodiscard]] virtual double computeFloating(double mean, double S) const noexcept = 0;
};

// PayOff class for a call option
class PayOffCall : public PayOff {
public:
	explicit PayOffCall(double E) noexcept;
	~PayOffCall() override = default;

	[[nodiscard]] double computeFixed(double mean) const noexcept override;
	[[nodiscard]] double computeFloating(double mean, double S) const noexcept override;
};