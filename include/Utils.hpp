#pragma once

#include <iostream>

#ifdef DebugPrint
#define PrintField(M_FieldName)\
	std::cout << #M_FieldName ": " << M_FieldName << std::endl;
#endif

template<typename Type, size_t Size>
constexpr size_t getArraySize(const Type (&array)[Size])
{
	return Size;
}
