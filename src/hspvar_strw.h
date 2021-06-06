
//
//	hspvar_strw.cpp header
//
#ifndef __hspvar_strw_h
#define __hspvar_strw_h

#include "hspvar_core.h"

#define HSP_VAR_NAME_STRW "strw"

EXPORT void HspVarStrW_Init(HspVarProc *p);
EXPORT int HspVarStrW_typeid(void);

CStringA UTF16toUTF8(const CStringW& utf16);
CStringW UTF8toUTF16(const CStringA& utf8);

void *HspVarStrW_Cnv(const void *buffer, int flag);
void *HspVarStrW_CnvCustom(const void *buffer, int flag);

#endif


