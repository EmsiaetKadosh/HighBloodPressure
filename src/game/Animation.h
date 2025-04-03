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
	mutable int progress = 0;
	int duration = 20;
	Style style = AS_LINEAR;
	Depend depend = AD_GET;
	bool positiveSuperAllowed = false;
	bool negativeSuperAllowed = false;
	bool doLoop = false;
	bool doReverse = true;

public:
	Animation() noexcept = default;

	Animation& allowPositiveSuper(const bool val = true) noexcept {
		positiveSuperAllowed = val;
		return *this;
	}

	Animation& allowNegativeSuper(const bool val = true) noexcept {
		negativeSuperAllowed = val;
		return *this;
	}

	Animation& depends(const Depend depend) noexcept {
		this->depend = depend;
		return *this;
	}

	Animation& features(const Style style) noexcept {
		this->style = style;
		return *this;
	}

	Animation& loop(const bool val = true) noexcept {
		doLoop = val;
		return *this;
	}

	Animation& includeReverse(const bool val = true) noexcept {
		doReverse = val;
		return *this;
	}

	Animation& setDuration(const int val) noexcept {
		if (duration == 0) duration = 1;
		duration = val;
		return *this;
	}

	void reset() const noexcept { progress = 0; }

	static double weight(double x) noexcept {
		if (x <= 0.0) return 0.0;
		if (x >= 1.0) return 1.0;
		x *= 2;
		return x < 1.0 ? pow(x, 5.0) * 0.5 : pow(x - 2.0, 5.0) * 0.5 + 1.0;
	}

	double calculateNext() const noexcept {
		if (++progress > duration) progress = doReverse ? 1 - duration : 0;
		double p = static_cast<double>(progress < 0 ? -progress : progress) / duration;
		const double w = weight(p);
		double val1 = 0, val2 = 0;
		switch (style & 0xff) {
			case AS_SIN_IN:
				val1 = 1 - cos(p * 1.5707963267948966192313216916398);
				break;
			case AS_QUADRATIC_IN:
				val1 = p * p;
				break;
			case AS_CUBIC_IN:
				val1 = p * p * p;
				break;
			case AS_LINEAR:
			default:
				val1 = p;
				break;
		}
		switch (style & 0xff00) {
			case AS_SIN_OUT:
				val2 = sin(p * 1.5707963267948966192313216916398);
				break;
			case AS_QUADRATIC_OUT:
				p = 1 - p;
				val2 = 1 - p * p;
				break;
			case AS_CUBIC_OUT:
				p = 1 - p;
				val2 = 1 + p * p * p;
				break;
			case AS_LINEAR:
			default:
				val2 = p;
				break;
		}
		return val1 * (1 - w) + val2 * w;
	}

	double getValue() const noexcept { return static_cast<double>(progress) / duration; }

	template <typename T>
	double adapts(T from, T to) const noexcept {
		const double val = calculateNext();
		return (to - from) * val + from;
	}

	unsigned int adaptsColor(const unsigned int from, const unsigned int to) const noexcept {
		const double val = calculateNext();
		const long long fr = from;
		const long long t = to;
		unsigned int ret = 0;
		long long temp = 0;
		temp = (t & 0xff000000) - (fr & 0xff000000);
		temp = static_cast<long long>(static_cast<double>(temp) * val);
		temp += fr & 0xff000000;
		if (temp > 0xffffffffLL) ret = 0xff000000;
		else ret = temp & 0xff000000;
		temp = (t & 0x00ff0000) - (fr & 0x00ff0000);
		temp = static_cast<long long>(static_cast<double>(temp) * val);
		temp += fr & 0x00ff0000;
		if (temp > 0xffffffLL) ret |= 0x00ff0000;
		else ret |= temp & 0x00ff0000;
		temp = (t & 0x0000ff00) - (fr & 0x0000ff00);
		temp = static_cast<long long>(static_cast<double>(temp) * val);
		temp += fr & 0x0000ff00;
		if (temp > 0xffffLL) ret |= 0x0000ff00;
		else ret |= temp & 0x0000ff00;
		temp = (t & 0x000000ff) - (fr & 0x000000ff);
		temp = static_cast<long long>(static_cast<double>(temp) * val);
		temp += fr & 0x000000ff;
		if (temp > 0xffLL) ret |= 0x000000ff;
		else ret |= temp & 0x000000ff;
		return ret;
	}
};
