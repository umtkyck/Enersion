/*
AdaptiveMovingAverage.cpp
Calculates an adaptive weighted moving average (WMA) of sample values.
Uses an adaptive WMA weight, that responds to how "wrong" the average is. 
This makes the average respond faster to large changes in the value
and still reduces the noise more when the value changes less. 

Created by Greger Burman, Adopticum, 2023.

Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
*/

#include "AdaptiveMovingAverage.h"
#include <math.h>

AdaptiveMovingAverage::AdaptiveMovingAverage(double min_value, double max_value)
{
	this->lo = min_value;
	this->hi = max_value;
	this->avg = 0.0;
	this->weight = 0.10;
	this->coeff = 1000.0 / (max_value - min_value);
}

void AdaptiveMovingAverage::push(double value)
{
	// Clamp the value inside the allowed range.
	auto safe_value = (value < this->lo) ? this->lo : (value > this->hi) ? this->hi : value;
	// Calculate the adaptive weight.
	auto diff = safe_value - this->avg;
	this->weight = this->coeff * log10(1 + diff * diff);
	// Calculate the WMA using the new weight.
	this->avg = this->weight * safe_value + (1.0 - this->weight) * this->avg;
}

double AdaptiveMovingAverage::average() const
{
	return this->avg;
}
