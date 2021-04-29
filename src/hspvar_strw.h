
//
//	hspvar_strw.cpp header
//
#ifndef __hspvar_strw_h
#define __hspvar_strw_h

#include "hspvar_core.h"

#ifdef __cplusplus
extern "C" {
#endif

void HspVarStrW_Init( HspVarProc *p );
int HspVarStrW_typeid( void );

#ifdef __cplusplus
}
#endif

void *HspVarStrW_Cnv(const void *buffer, int flag);

#endif


