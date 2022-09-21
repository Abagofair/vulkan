//
// Created by Anders on 20/09/2022.
//

#include "Timer.h"

#include <windows.h>
#include <stdint.h>

static double freq = 0.0;
static uint64_t start = 0;

double Timer_StartTime()
{
	return start;
}

double Timer_Now()
{
	LARGE_INTEGER largeInteger;
	QueryPerformanceCounter(&largeInteger);

	return (double)(largeInteger.QuadPart - start) / freq;
}

void Timer_Start()
{
	LARGE_INTEGER largeInteger;
	QueryPerformanceFrequency(&largeInteger);

	freq = (double)largeInteger.QuadPart;

	QueryPerformanceCounter(&largeInteger);
	start = largeInteger.QuadPart;
}