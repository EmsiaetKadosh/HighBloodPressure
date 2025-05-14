//
// Created by EmsiaetKadosh on 25-5-12.
//

#include "Animation.h"
#include "Game.h"


void Animation::reset() const noexcept { progress = 0, tickReference = game.getTick(); }

double Animation::calculateNext(const double tickDelta) const noexcept {
	if (depend == AD_TIME) { // AD_TIME
		const long long delta = static_cast<long long>(game.getTick() - tickReference);
		if (doLoop) {
			if (doReverse) {
				progress = delta % (duration << 1);
				if (progress > duration) progress = progress - (duration << 1);
			} else progress = delta % duration;
		} else {
			if (doReverse) {
				if (delta > duration << 1) progress = duration;
				else if (delta > duration) progress = delta - duration;
				else progress = delta;
			} else progress = delta > duration ? duration : delta;
		}
	} else if (++progress > duration) progress = doReverse ? 1 - duration : 0; // AD_GET; original code

	double p = static_cast<double>(progress < 0 ? -progress : progress) / static_cast<double>(duration);
	p += tickDelta * static_cast<double>(duration);
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

unsigned int Animation::adaptsColor(const unsigned int from, const unsigned int to, const double tickDelta) const noexcept {
	const double val = calculateNext(tickDelta);
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
