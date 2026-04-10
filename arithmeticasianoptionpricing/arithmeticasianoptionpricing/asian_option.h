#pragma once

#include "payoff.h"
#include <vector>
#include <memory>

class AsianOption {
protected:
	std::unique_ptr<PayOff> payoff;

public:
	explicit AsianOption(std::unique_ptr<PayOff> _payoff) noexcept;
	virtual ~AsianOption() = default;

	[[nodiscard]] virtual double payOffFixed(const std::vector<double>& assetPrices) const noexcept = 0;
	[[nodiscard]] virtual double payOffFloating(const std::vector<double>& assetPrices) const noexcept = 0;
};

// Derived class for Arithmetic Asian Option
class AsianOptionArithmetic : public AsianOption {
public:
	explicit AsianOptionArithmetic(std::unique_ptr<PayOff> _payoff) noexcept;
	~AsianOptionArithmetic() override = default;

	[[nodiscard]] double payOffFixed(const std::vector<double>& assetPrices) const noexcept override;
	[[nodiscard]] double payOffFloating(const std::vector<double>& assetPrices) const noexcept override;
};