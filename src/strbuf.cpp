
//
//	HSP3 string support
//	(おおらかなメモリ管理をするバッファマネージャー)
//	(sbAllocでSTRBUF_BLOCKSIZEのバッファを確保します)
//	(あとはsbCopy,sbAddで自動的にバッファの再確保を行ないます)
//	onion software/onitama 2004/6
//

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <atlstr.h>
#include "strbuf.h"

#include "hsp3plugin.h"
#include "hsp3debug.h"

#ifdef _DEBUG
#define TRACE(p) ::OutputDebugString(p); 
#else
#define TRACE(p)
#endif

/*------------------------------------------------------------*/
/*
		system data
*/
/*------------------------------------------------------------*/

#define GET_INTINF(buf) (&((buf)->inf.intptr->inf))

/*------------------------------------------------------------*/
/*
		interface
*/
/*------------------------------------------------------------*/

STRINF *sbGetSTRINF( char *ptr )
{
	return (STRINF *)( ptr - sizeof(STRINF) );
}


char *sbAlloc( int size )
{
	int sz;
	sz = size; if ( size < STRBUF_BLOCKSIZE_W) sz = STRBUF_BLOCKSIZE_W;
	return hspmalloc( sz );
}


char *sbAllocClear( int size )
{
	char *p;
	p = sbAlloc( size );
	memset( p, 0, size );
	return p;
}


void sbFree( void *ptr )
{
	hspfree(ptr);
}


char *sbExpand( char *ptr, int size )
{
	return hspexpand( ptr, size );
}


void sbCopy( char **pptr, char *data, int size )
{
	int sz;
	char *ptr;
	char *p;
	STRBUF *st;
	ptr = *pptr;
	st = (STRBUF *)( ptr - sizeof(STRINF) );
	sz = st->inf.size;
	p = st->inf.ptr;
	if ( size > sz ) { p = hspexpand( ptr, size ); *pptr = p; }
	memcpy( p, data, size );
}


void sbAdd( char **pptr, char *data, int size, int mode )
{
	//		mode:0=normal/1=string
	int sz,newsize;
	STRBUF *st;
	char *ptr;
	char *p;
	ptr = *pptr;
	st = (STRBUF *)( ptr - sizeof(STRINF) );
	p = st->inf.ptr;
	if ( mode ) {
		sz = (int)wcslen( (const wchar_t*)p ) * 2;					// 文字列データ
	} else {
		sz = st->inf.size;											// 通常データ
	}
	newsize = sz + size;
	if ( newsize > (st->inf.size) ) {
		newsize = ( newsize + 0xfff ) & 0xfffff000;						// 8K単位で確保
		//Alertf( "#Alloc%d",newsize );
		p = hspexpand( ptr, newsize );
		*pptr = p;
	}
	memcpy( p+sz, data, size );
}


void sbStrCopy( char **ptr, char *str )
{
	sbCopy( ptr, str, (int)wcslen((const wchar_t*)str) * 2 + 2 );
}


void sbStrAdd( char **ptr, char *str )
{
	sbAdd( ptr, str, (int)wcslen((const wchar_t*)str) * 2 + 2, 1 );
}


void *sbGetOption( char *ptr )
{
	STRBUF *st;
	st = (STRBUF *)( ptr - sizeof(STRINF) );
	return st->inf.opt;
}


void sbSetOption( char *ptr, void *option )
{
	STRBUF *st;
	STRINF *inf;
	st = (STRBUF *)( ptr - sizeof(STRINF) );
	st->inf.opt = option;
	inf = GET_INTINF(st);
	inf->opt = option;
}

/*
void sbInfo( char *ptr )
{
	STRBUF *st;
	st = (STRBUF *)( ptr - sizeof(STRINF) );
	Alertf( "size:%d (%x)",st->inf.size, st->inf.ptr );
}
*/

