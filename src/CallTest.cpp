#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "hsp3plugin.h"

// DllCall Test function

EXPORT int WINAPI Test_Int_Ret_Int(int p1)
{
	return p1 * 2;
}

EXPORT float WINAPI Test_Float_Ret_Float(float p1)
{
	return p1 * 2;
}

EXPORT double WINAPI Test_Double_Ret_Double(double p1)
{
	return p1 * 2;
}

EXPORT INT64 WINAPI Test_Int64_Ret_Int64(INT64 p1)
{
	return p1 * 2;
}

EXPORT int WINAPI Test_Int_Double_Float_Ret_Int(int p1, double p2, float p3)
{
	return p1 + p2 + p3;
}

EXPORT float WINAPI Test_Int_Double_Float_Ret_Float(int p1, double p2, float p3)
{
	return p1 + p2 + p3;
}

EXPORT double WINAPI Test_Int_Double_Float_Ret_Double(int p1, double p2, float p3)
{
	return p1 + p2 + p3;
}

EXPORT INT64 WINAPI Test_Int_Double_Float_Int64_Ret_Int64(int p1, double p2, float p3, INT64 p4)
{
	return p1 + p2 + p3 + p4;
}

EXPORT void WINAPI Test_Int_IntP_Ret_Void(int p1, int* p2)
{
	if ( p2 == nullptr)
	{
		return;
	}

	*p2 = p1 * 2;
}

EXPORT void WINAPI Test_Float_FloatP_Ret_Void(float p1, float* p2)
{
	if ( p2 == nullptr)
	{
		return;
	}

	*p2 = p1 * 2;
}

EXPORT void WINAPI Test_Double_DoubleP_Ret_Void(double p1, double* p2)
{
	if ( p2 == nullptr)
	{
		return;
	}

	*p2 = p1 * 2;
}

EXPORT void WINAPI Test_Int64_Int64P_Ret_Void(INT64 p1, INT64* p2)
{
	if (p2 == nullptr)
	{
		return;
	}

	*p2 = p1 * 2;
}

EXPORT void WINAPI Test_Int_Double_Float_Int_Double_Float_DoubleP_Ret_Void(int p1, double p2, float p3, int p4, double p5, float p6, double* p7)
{
	if ( p7 == nullptr)
	{
		return;
	}

	*p7 = p1 + p2 + p3 + p4 + p5 + p6;
}

EXPORT void WINAPI Test_Int64_Int64_Int64_Int64_Int64_double_double_double_double_double_Int64P_Ret_Void(
	INT64 p1, INT64 p2, INT64 p3, INT64 p4, INT64 p5, double p6, double p7, double p8, double p9, double p10, INT64* p11)
{
	if ( p11 == nullptr)
	{
		return;
	}

	*p11 = p1 + p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9 + p10;
}

EXPORT const char* WINAPI Test_Ret_Str()
{
	static const char val[] = "Test!!!!";
	return val;
}


EXPORT const wchar_t* WINAPI Test_Ret_StrW()
{
	static const wchar_t val[] = L"Test!!!!12345";
	return val;
}

EXPORT int WINAPI Test_POINT_Ret_Int(POINT pt)
{
	return pt.x + pt.y;
}

EXPORT int WINAPI Test_POINT_Ret_Int2(int ptX, int ptY)
{
	return ptX + ptY;
}

EXPORT int WINAPI Test_POINT_Ret_Int3(INT64 pt)
{
	return (int)pt;
}
