#pragma once

#ifdef DebugPrint
#include <iostream>
#endif

#define _USE_MATH_DEFINES
#include <math.h>

#ifdef DebugPrint
#define PrintField(M_FieldName)\
	std::cout << #M_FieldName ": " << M_FieldName << std::endl;
#endif

template<typename Type, size_t Size>
constexpr size_t getArraySize(const Type (&array)[Size])
{
	return Size;
}

bool equals(const float value1, const float value2, const float precision = 0.0001f)
{
	return std::abs(value1 - value2) < precision;
}

