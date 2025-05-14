//
// Created by EmsiaetKadosh on 25-3-7.
//

#pragma once

#include "..\def.h"

class Animation {
public:
	enum Depend : unsigned char {
		AD_TIME, AD_GET
	};

	enum Style : unsigned short {
		AS_LINEAR = 0,
		AS_SIN_IN = 0x1, AS_SIN_OUT = 0x100, AS_SIN = 0x101,
		AS_QUADRATIC_IN = 0x2, AS_QUADRATIC_OUT = 0x200, AS_QUADRATIC = 0x202,
		AS_CUBIC_IN = 0x3, AS_CUBIC_OUT = 0x300, AS_CUBIC = 0x303,
	};

private:
	mutable QWORD tickReference = 0;
	mutable long long progress = 0;
	long long duration = 20;
	Style style = AS_LINEAR;
	Depend depend = AD_GET;
	bool positiveSuperAllowed = false;
	bool negativeSuperAllowed = false;
	bool doLoop = true;
	bool doReverse = true;

public:
	static double weight(double x) noexcept {
		if (x <= 0.0) return 0.0;
		if (x >= 1.0) return 1.0;
		x *= 2;
		return x < 1.0 ? pow(x, 5.0) * 0.5 : pow(x - 2.0, 5.0) * 0.5 + 1.0;
	}

	Animation() noexcept = default;

	Animation& allowPositiveSuper(const bool val = true) & noexcept { return positiveSuperAllowed = val, *this; }
	Animation&& allowPositiveSuper(const bool val = true) && noexcept { return positiveSuperAllowed = val, std::move(*this); }
	Animation& allowNegativeSuper(const bool val = true) & noexcept { return negativeSuperAllowed = val, *this; }
	Animation&& allowNegativeSuper(const bool val = true) && noexcept { return negativeSuperAllowed = val, std::move(*this); }
	Animation& depends(const Depend depend) & noexcept { return this->depend = depend, *this; }
	Animation&& depends(const Depend depend) && noexcept { return this->depend = depend, std::move(*this); }
	Animation& features(const Style style) & noexcept { return this->style = style, *this; }
	Animation&& features(const Style style) && noexcept { return this->style = style, std::move(*this); }
	Animation& loop(const bool val = true) & noexcept { return doLoop = val, *this; }
	Animation&& loop(const bool val = true) && noexcept { return doLoop = val, std::move(*this); }
	Animation& includeReverse(const bool val = true) & noexcept { return doReverse = val, *this; }
	Animation&& includeReverse(const bool val = true) && noexcept { return doReverse = val, std::move(*this); }
	Animation& setDuration(const long long val) & noexcept { return duration = val ? val : 1, *this; }
	Animation&& setDuration(const long long val) && noexcept { return duration = val ? val : 1, std::move(*this); }
	double getValue() const noexcept { return static_cast<double>(progress) / static_cast<double>(duration); }
	void reset() const noexcept;
	double calculateNext(double tickDelta) const noexcept;
	unsigned int adaptsColor(unsigned int from, unsigned int to, double tickDelta = 0) const noexcept;

	template <typename T>
	double adapts(T from, T to, const double tickDelta = 0) const noexcept {
		const double val = calculateNext(tickDelta);
		return (to - from) * val + from;
	}
};
