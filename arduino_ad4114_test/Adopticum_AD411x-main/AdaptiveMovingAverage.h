/*
AdaptiveMovingAverage.h
Calculates an adaptive weighted moving average (WMA) of sample values.
Use an adaptive WMA weight, that responds to how "wrong" the average is. 
This makes the average respond faster to large changes in the value
and still reduces the noise more when the value changes less. 

Created by Greger Burman, Adopticum, 2023.

Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
*/

#pragma once
#include <Arduino.h>


class AdaptiveMovingAverage
{
public:
	AdaptiveMovingAverage(double min_value, double max_value);
	
	void push(double value);
	
	double average() const;

protected:
	double lo, hi;
	double avg;
	double weight;
	double coeff;
};