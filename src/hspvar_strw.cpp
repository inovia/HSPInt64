
//
//	HSPVAR core module
//	onion software/onitama 2003/4
//
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <atlstr.h>

#include "hsp3plugin.h"
#include "hspvar_core.h"
#include "hsp3debug.h"

#include "strbuf.h"
#include "hspvar_int64.h"
#include "hspvar_float.h"

/*------------------------------------------------------------*/
/*
		HSPVAR core interface (float)
*/
/*------------------------------------------------------------*/

#define GetPtr(pval) ((wchar_t *)pval)
#define sbAlloc hspmalloc
#define sbFree hspfree

static int mytype;
static CStringW conv;
static short *aftertype;
static CStringA customA;
static char custom[64];

// ------------------------------------------------------------------------------
// UTF8 <- -> UTF-16 �����R�[�h�ϊ�
// https://www.codeproject.com/Articles/26134/UTF16-to-UTF8-to-UTF16-simple-CString-based-conver
// ------------------------------------------------------------------------------
static CStringA UTF16toUTF8(const CStringW& utf16)
{
	CStringA utf8;
	int len = WideCharToMultiByte(CP_UTF8, 0, utf16, -1, NULL, 0, 0, 0);
	if (len > 1)
	{
		char *ptr = utf8.GetBuffer(len - 1);
		if (ptr) WideCharToMultiByte(CP_UTF8, 0, utf16, -1, ptr, len, 0, 0);
		utf8.ReleaseBuffer();
	}
	return utf8;
}

static CStringW UTF8toUTF16(const CStringA& utf8)
{
	CStringW utf16;
	int len = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, 0);
	if (len > 1)
	{
		wchar_t *ptr = utf16.GetBuffer(len - 1);
		if (ptr) MultiByteToWideChar(CP_UTF8, 0, utf8, -1, ptr, len);
		utf16.ReleaseBuffer();
	}
	return utf16;
}
// ------------------------------------------------------------------------------

static char **GetFlexBufPtr(PVal *pval, int num)
{
	//		�ϒ��o�b�t�@�̃|�C���^�𓾂�
	//
	char **pp;
	if (num == 0) return &(pval->pt);		// ID#0�́Apt���|�C���^�ƂȂ�
	pp = (char **)(pval->master);
	return &pp[num];
}

// Core
static PDAT *HspVarStrW_GetPtr(PVal *pval)
{
	char **pp;
	pp = GetFlexBufPtr(pval, pval->offset);
	return (PDAT *)(*pp);
}

void *HspVarStrW_Cnv( const void *buffer, int flag )
{
	//		���N�G�X�g���ꂽ�^ -> �����̌^�ւ̕ϊ����s�Ȃ�
	//		(�g�ݍ��݌^�ɂ̂ݑΉ���OK)
	//		(�Q�ƌ��̃f�[�^��j�󂵂Ȃ�����)
	//

	// ���M�̌^�̏ꍇ
	if ( flag == mytype) 
	{
		return (void *)buffer;
	}

	switch( flag ) {
	case HSPVAR_FLAG_STR:
		conv = UTF8toUTF16((const char *)buffer);
		return conv.GetBuffer();
	case HSPVAR_FLAG_INT:
		conv.Format( L"%d", *(int *)buffer);
		return conv.GetBuffer();
	case HSPVAR_FLAG_DOUBLE:
		conv.Format( L"%lf", *(double *)buffer);
		return conv.GetBuffer();

	default:
		if ( flag == HspVarInt64_typeid())
		{
			conv.Format( L"%lld", *(INT64 *)buffer);
			return conv.GetBuffer();
		}
		else if ( flag == HspVarFloat_typeid())
		{
			conv.Format( L"%f", *(float *)buffer);
			return conv.GetBuffer();
		}
		else;

		throw HSPVAR_ERROR_TYPEMISS;
	}
	return (void *)buffer;
}


static void *HspVarStrW_CnvCustom( const void *buffer, int flag )
{
	//		(�J�X�^���^�C�v�̂�)
	//		�����̌^ -> ���N�G�X�g���ꂽ�^ �ւ̕ϊ����s�Ȃ�
	//		(�g�ݍ��݌^�ɑΉ�������)
	//		(�Q�ƌ��̃f�[�^��j�󂵂Ȃ�����)
	//

	const wchar_t * p = (const wchar_t *)buffer;

	switch( flag ) {
	case HSPVAR_FLAG_STR:
		customA = UTF16toUTF8(p);
		return customA.GetBuffer();

	case HSPVAR_FLAG_INT:
		*(int *)custom = _wtoi(p);
		break;

	case HSPVAR_FLAG_DOUBLE:
		*(double *)custom = _wtof(p);
		break;

	default:
		if ( flag == HspVarInt64_typeid())
		{
			*(INT64 *)custom = _wtoi64(p);
			break;
		}
		else if (flag == HspVarFloat_typeid())
		{
			*(float *)custom = (float)_wtof(p);
			break;
		}
		else;

		throw HSPVAR_ERROR_TYPEMISS;
	}
	return custom;
}


static int GetVarSize(PVal *pval)
{
	//		PVAL�|�C���^�̕ϐ����K�v�Ƃ���T�C�Y���擾����
	//		(size�t�B�[���h�ɐݒ肳���)
	//
	int size;
	size = pval->len[1];
	if (pval->len[2]) size *= pval->len[2];
	if (pval->len[3]) size *= pval->len[3];
	if (pval->len[4]) size *= pval->len[4];
	size *= sizeof(wchar_t *);
	pval->size = size;
	return size;
}


static void HspVarStrW_Free(PVal *pval)
{
	//		PVAL�|�C���^�̕ϐ����������������
	//
	char **pp;
	int i, size;
	if (pval->mode == HSPVAR_MODE_MALLOC) {
		size = GetVarSize(pval);
		for (i = 0; i < (int)(size / sizeof(wchar_t *)); i++) {
			pp = GetFlexBufPtr(pval, i);
			sbFree(*pp);
		}
		free(pval->master);
	}
	pval->mode = HSPVAR_MODE_NONE;
}

static void HspVarStrW_Alloc(PVal *pval, const PVal *pval2)
{
	//		pval�ϐ����K�v�Ƃ���T�C�Y���m�ۂ���B
	//		(pval�����łɊm�ۂ���Ă��郁��������͌Ăяo�������s�Ȃ�)
	//		(pval2��NULL�̏ꍇ�́A�V�K�f�[�^�Blen[0]�Ɋm�ۃo�C�g������������)
	//		(pval2���w�肳��Ă���ꍇ�́Apval2�̓��e���p�����čĊm��)
	//
	char **pp;
	int i, i2, size, bsize;
	PVal oldvar;
	if (pval->len[1] < 1) pval->len[1] = 1;		// �z����Œ�1�͊m�ۂ���
	if (pval2 != NULL) oldvar = *pval2;			// �g�����͈ȑO�̏���ۑ�����

	size = GetVarSize(pval);
	pval->mode = HSPVAR_MODE_MALLOC;
	pval->master = (char *)calloc(size, 1);
	if (pval->master == NULL) throw HSPERR_OUT_OF_MEMORY;

	if (pval2 == NULL) {							// �z��g���Ȃ�
		bsize = pval->len[0];
		if (bsize < STRBUF_BLOCKSIZE) { bsize = STRBUF_BLOCKSIZE; }
		for (i = 0; i < (int)(size / sizeof(char *)); i++) {
			pp = GetFlexBufPtr(pval, i);
			*pp = sbAllocClear(bsize);
			sbSetOption(*pp, (void *)pp);
		}
		return;
	}

	i2 = oldvar.size / sizeof(wchar_t *);
	for (i = 0; i < (int)(size / sizeof(wchar_t *)); i++) {
		pp = GetFlexBufPtr(pval, i);
		if (i >= i2) {
			*pp = sbAllocClear(STRBUF_BLOCKSIZE);	// �V�K�m�ە�
		}
		else {
			*pp = *GetFlexBufPtr(&oldvar, i);		// �m�ۍς݃o�b�t�@
		}
		sbSetOption(*pp, (void *)pp);
	}
	free(oldvar.master);
}
/*
static void *HspVarFloat_ArrayObject( PVal *pval, int *mptype )
{
	//		�z��v�f�̎w�� (������/�A�z�z��p)
	//
	throw HSPERR_UNSUPPORTED_FUNCTION;
	return NULL;
}
*/

// Size
static int HspVarStrW_GetSize(const PDAT *pdat)
{
	return (int)(wcslen((wchar_t *)pdat) * 2 + 2);
}

// Set
static void HspVarStrW_Set(PVal *pval, PDAT *pdat, const void *in)
{
	char **pp;
	if (pval->mode == HSPVAR_MODE_CLONE) {
		wcsncpy((wchar_t *)pdat, (wchar_t *)in, pval->size / 2);		// byte -> ������
		return;
	}
	pp = (char **)sbGetOption((char *)pdat);
	sbStrCopy(pp, (char *)in);
	//strcpy( GetPtr(pval), (char *)in );
}

// Add
static void HspVarStrW_AddI(PDAT *pval, const void *val)
{
	char **pp;
	pp = (char **)sbGetOption((char *)pval);
	sbStrAdd(pp, (char *)val);
	//strcat( GetPtr(pval), (char *)val );
	*aftertype = mytype;
}


// Eq
static void HspVarStrW_EqI(PDAT *pdat, const void *val)
{
	if (wcscmp((wchar_t *)pdat, (wchar_t *)val)) {
		*(int *)pdat = 0;
	}
	else {
		*(int *)pdat = 1;
	}
	*aftertype = HSPVAR_FLAG_INT;
}

// Ne
static void HspVarStrW_NeI(PDAT *pdat, const void *val)
{
	int i;
	i = wcscmp((wchar_t *)pdat, (wchar_t *)val);
	if (i < 0) i = -1;
	if (i > 0) i = 1;
	*(int *)pdat = i;
	*aftertype = HSPVAR_FLAG_INT;
}

/*
// INVALID
static void HspVarFloat_Invalid( PDAT *pval, const void *val )
{
	throw( HSPVAR_ERROR_INVALID );
}
*/

static void *GetBlockSize(PVal *pval, PDAT *pdat, int *size)
{
	STRINF *inf;
	if (pval->mode == HSPVAR_MODE_CLONE) {
		*size = pval->size;
		return pdat;
	}
	inf = sbGetSTRINF((char *)pdat);
	*size = inf->size;
	return pdat;
}

static void AllocBlock(PVal *pval, PDAT *pdat, int size)
{
	char **pp;
	if (pval->mode == HSPVAR_MODE_CLONE) return;
	pp = (char **)sbGetOption((char *)pdat);
	*pp = sbExpand(*pp, size);
}


/*------------------------------------------------------------*/

EXPORT int HspVarStrW_typeid( void )
{
	return mytype;
}


EXPORT void HspVarStrW_Init( HspVarProc *p )
{
	aftertype = &p->aftertype;

	p->Set = HspVarStrW_Set;
	p->Cnv = HspVarStrW_Cnv;
	p->GetPtr = HspVarStrW_GetPtr;
	p->CnvCustom = HspVarStrW_CnvCustom;
	p->GetSize = HspVarStrW_GetSize;
	p->GetBlockSize = GetBlockSize;
	p->AllocBlock = AllocBlock;

	//	p->ArrayObject = HspVarStrW_ArrayObject;
	p->Alloc = HspVarStrW_Alloc;
	p->Free = HspVarStrW_Free;

	p->AddI = HspVarStrW_AddI;
	//	p->SubI = HspVarStrW_Invalid;
	//	p->MulI = HspVarStrW_Invalid;
	//	p->DivI = HspVarStrW_Invalid;
	//	p->ModI = HspVarStrW_Invalid;

	//	p->AndI = HspVarStrW_Invalid;
	//	p->OrI  = HspVarStrW_Invalid;
	//	p->XorI = HspVarStrW_Invalid;

	p->EqI = HspVarStrW_EqI;
	p->NeI = HspVarStrW_NeI;
	//	p->GtI = HspVarStrW_Invalid;
	//	p->LtI = HspVarStrW_Invalid;
	//	p->GtEqI = HspVarStrW_Invalid;
	//	p->LtEqI = HspVarStrW_Invalid;

	//	p->RrI = HspVarStrW_Invalid;
	//	p->LrI = HspVarStrW_Invalid;

	p->vartype_name = "strw";			// �^�C�v��
	p->version = 0x001;					// �^�^�C�v�����^�C���o�[�W����(0x100 = 1.0)
	p->support = HSPVAR_SUPPORT_FLEXSTORAGE | HSPVAR_SUPPORT_FLEXARRAY;
	// �T�|�[�g�󋵃t���O(HSPVAR_SUPPORT_*)
	p->basesize = -1;					// �P�̃f�[�^���g�p����T�C�Y(byte) / �ϒ��̎���-1

	mytype = p->flag;
}

/*------------------------------------------------------------*/

