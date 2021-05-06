
//
//		HSP3.0 plugin sample
//		onion software/onitama 2004/9
//

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <atlstr.h>

#include "strbuf.h"

#include "hsp3plugin.h"
#include "hsp3struct.h"
#include "hspvar_int64.h"
#include "hspvar_float.h"
#include "hspvar_strw.h"

// TODO: �����Ⴒ���Ⴕ�Ă����̂ŁA�����\��

enum class RET_TYPE : int
{
	Void		= -1,		// void�^�p
	Int			= 0,		// int�^�ϐ��ŕԂ��܂�
	Int64		= 1,		// int64�^�ϐ��ŕԂ��܂�
	Double		= 2,		// double�^�ϐ��ŕԂ��܂�
	Float		= 3,		// float�^�ϐ��ŕԂ��܂�
	FloatInt	= 4,		// float�l��int�^�ϐ��ŕԂ��܂�
	Str			= 5,		// ������^�ŕԂ��܂�
	StrW		= 6,		// Unicode������^(UTF-16)�ŕԂ��܂�
};

// func�`���ŌĂԏꍇ�icmd �̍ŏ�ʃr�b�g�����邾���j
#define CALL_FUNC_TYPE 0x80000000

// 64bit dll call
extern "C" INT64 CallFunc64(INT64 *, FARPROC, int);
extern "C" double CallFunc64d(INT64 *, FARPROC, int);
extern "C" float CallFunc64f(INT64 *, FARPROC, int);

#define call_extfunc64(externalFunction, arguments, numberOfArguments)	CallFunc64((INT64 *)arguments, (FARPROC)externalFunction, numberOfArguments)
#define call_extfunc64d(externalFunction, arguments, numberOfArguments)	CallFunc64d((INT64 *)arguments, (FARPROC)externalFunction, numberOfArguments)
#define call_extfunc64f(externalFunction, arguments, numberOfArguments)	CallFunc64f((INT64 *)arguments, (FARPROC)externalFunction, numberOfArguments)

/*------------------------------------------------------------*/
/*
		controller
*/
/*------------------------------------------------------------*/

static void *reffunc( int *type_res, int cmd )
{
	// �ŏ�ʃr�b�g�������Ă���Ƃ��́AFunc(�֐��`��)����
	bool bFunc = (cmd & CALL_FUNC_TYPE) != 0;
	cmd &= ~(CALL_FUNC_TYPE);

	// �߂�l�̕ϐ�
	static INT64 ref_int64val;						
	static double ref_doubleval;
	static float ref_floatval;
	static CStringA ref_strval;
	static CStringW ref_strwval;

	// �߂�l�̌^
	RET_TYPE ref_type;

	// �֐��`���̏ꍇ
	if (!bFunc)
	{
		//		�֐��E�V�X�e���ϐ��̎��s���� (�l�̎Q�Ǝ��ɌĂ΂�܂�)
		//
		//			'('�Ŏn�܂邩�𒲂ׂ�
		//
		if (*type != TYPE_MARK)
		{
			puterror(HSPERR_INVALID_FUNCPARAM);
		}

		if (*val != '(')
		{
			puterror(HSPERR_INVALID_FUNCPARAM);
		}

		code_next();
	}

	switch( cmd ) {							// �T�u�R�}���h���Ƃ̕���

	case 0x00:								// int64�֐�
	{
		int prm;
		prm = code_getprm();					// �����l���擾(�f�t�H���g�Ȃ�)
		if ( prm <= PARAM_END)
		{
			puterror(HSPERR_NO_DEFAULT);
		}
		ref_int64val = *(INT64 *)HspVarInt64_Cnv(mpval->pt, mpval->flag);
		ref_type = RET_TYPE::Int64;
		break;
	}
	case 0x01:								// qpeek�֐�
	{
		PVal *pval;
		APTR aptr;

		aptr = code_getva(&pval);

		HspVarProc *phvp;
		phvp = exinfo->HspFunc_getproc(pval->flag);
		PDAT* pdat = phvp->GetPtr(pval);

		int size;
		phvp->GetBlockSize(pval, pdat, &size);

		int p1 = code_getdi(0);

		if ( p1 < 0)
		{
			puterror(HSPERR_ILLEGAL_FUNCTION);
		}

		if ( p1 + 8 > size)
		{
			puterror(HSPERR_ILLEGAL_FUNCTION);
		}

		ref_int64val = *(INT64 *)(pdat + p1);
		ref_type = RET_TYPE::Int64;
		break;
	}
	case 0x03:								// varptr64
	{
		PVal *pval;
		APTR aptr;
		STRUCTDAT *st;
		if (*type == TYPE_DLLFUNC) 
		{
			st = &(ctx->mem_finfo[*val]);
			ref_int64val = (INT64)(st->proc);
			ref_type = RET_TYPE::Int64;
			code_next();
			break;
		}
		aptr = code_getva(&pval);

		HspVarProc *phvp;
		phvp = exinfo->HspFunc_getproc(pval->flag);
		ref_int64val = (INT64)phvp->GetPtr(pval);
		ref_type = RET_TYPE::Int64;

		break;
	}
	case 0x05:	// callfunc64i
	case 0x06:	// callfunc64d
	case 0x07:	// callfunc64f
	{
		// 1�p��= �p�����^(INT64�ϐ�), 2�p�� = �A�h���X(INT64), 3�p�� = �p�����^��(INT)
		PVal *pval;
		APTR aptr;

		// ��1����
		aptr = code_getva(&pval);
		if ( pval->flag != HspVarInt64_typeid())	// INT64�ł͂Ȃ�
		{
			puterror(HSPERR_ILLEGAL_FUNCTION);
		}
			
		HspVarProc *phvp;
		phvp = exinfo->HspFunc_getproc(pval->flag);
		INT64 prm1 = (INT64)phvp->GetPtr(pval);

		// ��2����
		int prm = code_getprm();					// �����l���擾(�f�t�H���g�Ȃ�)
		if ( prm <= PARAM_END)
		{
			puterror(HSPERR_NO_DEFAULT);
		}
		INT64 prm2 = *(INT64 *)HspVarInt64_Cnv(mpval->pt, mpval->flag);

		// ��3����
		int prm3 = code_geti();

		if ( 0x05 == cmd)
		{
			ref_int64val = call_extfunc64((void *)prm2, (INT64 *)prm1, prm3);
			ref_type = RET_TYPE::Int64;
		}
		else if ( 0x06 == cmd)
		{
			ref_doubleval = call_extfunc64d((void *)prm2, (INT64 *)prm1, prm3);
			ref_type = RET_TYPE::Double;
		}
		else if ( 0x07 == cmd)
		{
			ref_floatval = call_extfunc64f((void *)prm2, (INT64 *)prm1, prm3);
			ref_type = RET_TYPE::FloatInt;
		}

		break;
	}

	case 0x08:	// cfunc64
	case 0x100:	// cfunc64v
	case 0x101:	// cfunc64i
	case 0x102:	// cfunc64i64
	case 0x103:	// cfunc64d
	case 0x104:	// cfunc64f
	case 0x105:	// cfunc64fi
	case 0x106:	// cfunc64s
	case 0x107:	// cfunc64sw
	{
		int prm;

		// 1�p�� = �߂�l�̌^ 0(int), 1(int64), 2(dobule), 3(float), 4(float->int) 5(char*), 6(wchar*),
		// 2�p�� = �A�h���X(INT64) OR #func #cfunc ��DLL�֐���
		// 3�p���ȍ~ �ϒ�����

		if ( cmd == 0x08)
		{
			// ��1���� 
			int prm1 = code_getdi((int)RET_TYPE::Int64);

			if ( prm1 < (int)RET_TYPE::Void || (int)RET_TYPE::StrW < prm1)
			{
				puterror(HSPERR_ILLEGAL_FUNCTION);
			}
			else;

			ref_type = static_cast<RET_TYPE>( prm1);
		}
		else 
		{
			// 0x100 - 0x107
			// RET_TYPE �� -1 �n�܂�
			ref_type = static_cast<RET_TYPE>( cmd - 0x100 - 1);
		}

		// ��2����
		INT64 prm2;

		STRUCTDAT *st;
		if (*type == TYPE_DLLFUNC)					// DLL�̊֐������w�肵���ꍇ�̃P�[�X
		{
			st = &(ctx->mem_finfo[*val]);
			prm2 = (INT64)(st->proc);
			code_next();
			prm = code_getprm();					// �����l���擾(�f�t�H���g�Ȃ�)
		}
		else 
		{
			prm = code_getprm();					// �����l���擾(�f�t�H���g�Ȃ�)

			if ( prm <= PARAM_END)
			{
				puterror(HSPERR_NO_DEFAULT);
			}

			if ( mpval->flag != HspVarInt64_typeid())		// INT64�݂̂����s����
			{
				puterror(HSPERR_INVALID_PARAMETER);
			}
			prm2 = *(INT64 *)HspVarInt64_Cnv(mpval->pt, mpval->flag);
		}

		// ��3�����ȍ~�A�ϒ�
		std::vector<INT64> vParams;
		std::vector<byte*> vStringData;

		// TODO: ����ł�����H
		INT64 liTmp;
		
		do 
		{
			prm = code_getprm();

			if ( prm == PARAM_OK || prm == PARAM_SPLIT)
			{
				switch (mpval->flag)
				{
				case HSPVAR_FLAG_STR:
				{
					// ������T�C�Y���擾
					// (�o�C�i�������Ă���P�[�X��z�肵�ďI�[NULL����ł͂Ȃ��AHSP�����Ǘ����Ă���T�C�Y���擾�j
					int nSrcSize;
					HspVarProc* phvp = exinfo->HspFunc_getproc(mpval->flag);
					PDAT* pDat = phvp->GetPtr(mpval);
					phvp->GetBlockSize(mpval, pDat, &nSrcSize);

					// �R�s�[��̃T�C�Y�����߂�
					int nDestSize = (nSrcSize <= 0) ? 64 : nSrcSize;

					byte* pDest = new byte[nDestSize];
					::memcpy_s( pDest, nDestSize, pDat, nSrcSize);

					vStringData.push_back( pDest);
					liTmp = (INT64)pDest;
					
					//// ����
					//CStringA* push = new CStringA(mpval->pt);
					//vStrings.push_back(push);
					//liTmp = (INT64)push->GetBuffer();
					break;
				}
					
				case HSPVAR_FLAG_INT:
					liTmp = (INT64)*(int*) mpval->pt;
					break;
				case HSPVAR_FLAG_DOUBLE:
					liTmp = *(INT64*)(double*)mpval->pt;
					break;
				default:

					// INT64
					if ( mpval->flag == HspVarInt64_typeid())
					{
						liTmp = *(INT64*)mpval->pt;
					}
					else if ( mpval->flag == HspVarFloat_typeid())
					{
						liTmp = *(INT64*)(float*)mpval->pt;
					}
					else if (mpval->flag == HspVarStrW_typeid())
					{
						// ������T�C�Y���擾
						// (�o�C�i�������Ă���P�[�X��z�肵�ďI�[NULL����ł͂Ȃ��AHSP�����Ǘ����Ă���T�C�Y���擾�j
						int nSrcSize;
						HspVarProc* phvp = exinfo->HspFunc_getproc(mpval->flag);
						PDAT* pDat = phvp->GetPtr(mpval);
						phvp->GetBlockSize(mpval, pDat, &nSrcSize);

						// �R�s�[��̃T�C�Y�����߂�
						int nDestSize = (nSrcSize <= 0) ? 128 : nSrcSize;

						byte* pDest = new byte[nDestSize];
						::memcpy_s(pDest, nDestSize, pDat, nSrcSize);

						vStringData.push_back(pDest);
						liTmp = (INT64)pDest;
					}
					else;

					break;
				}

				vParams.push_back(liTmp);
			}
		} while (PARAM_END < prm);

		// ���s
		switch (ref_type)
		{
		case RET_TYPE::Void:
		case RET_TYPE::Int:
		case RET_TYPE::Int64:
			ref_int64val = call_extfunc64((void *)prm2, (INT64 *)vParams.data(), (int)vParams.size());
			break;
		case RET_TYPE::Double:
			ref_doubleval = call_extfunc64d((void *)prm2, (INT64 *)vParams.data(), (int)vParams.size());
			break;
		case RET_TYPE::Float:
		case RET_TYPE::FloatInt:
			ref_floatval = call_extfunc64f((void *)prm2, (INT64 *)vParams.data(), (int)vParams.size());
			break;
		
		case RET_TYPE::Str:
		{
			char* pStr = (char*)call_extfunc64((void *)prm2, (INT64 *)vParams.data(), (int)vParams.size());
			ref_strval = CStringA(pStr);
			break;
		}
			
		case RET_TYPE::StrW:
		{
			wchar_t* pStr = (wchar_t*)call_extfunc64((void *)prm2, (INT64 *)vParams.data(), (int)vParams.size());
			ref_strwval = CStringW(pStr);
			break;
		}
			
		default:
			break;
		}

		// �����p������폜
		for ( auto v : vStringData)
		{
			delete[] v;
		}

		break;
	}
	case 0x0A:								// float�֐�
	{
		int prm;
		prm = code_getprm();
		if (prm <= PARAM_END)
		{
			puterror(HSPERR_NO_DEFAULT);
		}
		ref_floatval = *(float *)HspVarFloat_Cnv(mpval->pt, mpval->flag);
		ref_type = RET_TYPE::Float;
		break;
	}
	case 0x0C:								// strw�֐�
	{
		int prm;
		prm = code_getprm();
		if (prm <= PARAM_END)
		{
			puterror(HSPERR_NO_DEFAULT);
		}
		ref_strwval = (wchar_t *)HspVarStrW_Cnv(mpval->pt, mpval->flag);
		ref_type = RET_TYPE::StrW;
		break;
	}
	case 0x0E:								// strwlen�֐�
	{
		int prm;
		prm = code_getprm();
		if (prm <= PARAM_END)
		{
			puterror(HSPERR_NO_DEFAULT);
		}

		// �^�`�F�b�N�͂��Ȃ�
		ref_int64val = wcslen((const wchar_t*) mpval->pt);
		ref_type = RET_TYPE::Int;
		break;
	}
	case 0x10:								// instrw�֐�
	{
		int nTypeid_StrW = HspVarStrW_typeid();

		PVal *pval;
		APTR aptr;

		aptr = code_getva(&pval);
		if (pval->flag != nTypeid_StrW) throw HSPERR_TYPE_MISMATCH;

		HspVarProc *phvp;
		phvp = exinfo->HspFunc_getproc(pval->flag);
		CStringW buf = (wchar_t *)phvp->GetPtr(pval);

		int p1 = code_geti();					// �����l���擾(�f�t�H���g�Ȃ�)

		int prm = code_getprm();						// �����l���擾(�f�t�H���g�Ȃ�)
		if (prm <= PARAM_END)
		{
			puterror(HSPERR_NO_DEFAULT);
		}
		const CStringW p2 = (const wchar_t*)HspVarStrW_Cnv(mpval->pt, mpval->flag);

		ref_int64val = buf.Find(p2, p1);
		ref_type = RET_TYPE::Int;

		break;
	}
	case 0x11:								// strwmid�֐�
	{
		int nTypeid_StrW = HspVarStrW_typeid();

		PVal *pval;
		APTR aptr;

		aptr = code_getva(&pval);
		if (pval->flag != nTypeid_StrW) throw HSPERR_TYPE_MISMATCH;

		HspVarProc *phvp;
		phvp = exinfo->HspFunc_getproc(pval->flag);
		CStringW buf = (wchar_t *)phvp->GetPtr(pval);

		int p1 = code_geti();					// �����l���擾(�f�t�H���g�Ȃ�)
		int p2 = code_geti();					// �����l���擾(�f�t�H���g�Ȃ�)

		if ( p1 < 0)
		{
			ref_strwval = buf.Right(p2);
		}
		else 
		{
			ref_strwval = buf.Mid(p1, p2);
		}

		ref_type = RET_TYPE::StrW;

		break;
	}
	case 0x12:								// strwtrim�֐�
	{
		int nTypeid_StrW = HspVarStrW_typeid();

		PVal *pval;
		APTR aptr;

		aptr = code_getva(&pval);
		if (pval->flag != nTypeid_StrW) throw HSPERR_TYPE_MISMATCH;

		HspVarProc *phvp;
		phvp = exinfo->HspFunc_getproc(pval->flag);
		CStringW buf = CStringW((wchar_t *)phvp->GetPtr(pval));

		int p1 = code_geti();					// �����l���擾(�f�t�H���g�Ȃ�)

		int prm = code_getprm();				// �����l���擾(�f�t�H���g�Ȃ�)
		if (prm <= PARAM_END)
		{
			puterror(HSPERR_NO_DEFAULT);
		}

		CStringW p2;

		if ( mpval->flag == HSPVAR_FLAG_INT)
		{
			p2 = (const wchar_t)*(const wchar_t*)mpval->pt;
		}
		else 
		{
			p2 = (const wchar_t*)HspVarStrW_Cnv(mpval->pt, mpval->flag);
		}

		switch (p1)
		{
			case 1:
			{
				ref_strwval = buf.TrimLeft( p2);
				break;
			}
			case 2:
			{
				ref_strwval = buf.TrimRight( p2);
				break;
			}
			case 3:
			{
				buf.Replace( p2, L"");
				ref_strwval = buf;
				break;
			}
			default:
			{
				ref_strwval = buf.Trim( p2);
				break;
			}
		}

		ref_type = RET_TYPE::StrW;
		break;
	}
	case 0x13:								// strwupper�֐�
	{
		int prm = code_getprm();				// �����l���擾(�f�t�H���g�Ȃ�)
		if (prm <= PARAM_END)
		{
			puterror(HSPERR_NO_DEFAULT);
		}
		CStringW buf = (const wchar_t*)HspVarStrW_Cnv(mpval->pt, mpval->flag);

		ref_strwval = buf.MakeUpper();
		ref_type = RET_TYPE::StrW;
		break;
	}
	case 0x14:								// strwlower�֐�
	{
		int prm = code_getprm();				// �����l���擾(�f�t�H���g�Ȃ�)
		if (prm <= PARAM_END)
		{
			puterror(HSPERR_NO_DEFAULT);
		}
		CStringW buf = (const wchar_t*)HspVarStrW_Cnv(mpval->pt, mpval->flag);

		ref_strwval = buf.MakeLower();
		ref_type = RET_TYPE::StrW;
		break;
	}
	case 0x15:								// strwinsert�֐�
	{
		int prm = code_getprm();				// �����l���擾(�f�t�H���g�Ȃ�)
		if (prm <= PARAM_END)
		{
			puterror(HSPERR_NO_DEFAULT);
		}
		CStringW buf = CString((const wchar_t*)HspVarStrW_Cnv(mpval->pt, mpval->flag));

		int p1 = code_geti();					// �����l���擾(�f�t�H���g�Ȃ�)

		prm = code_getprm();				// �����l���擾(�f�t�H���g�Ȃ�)
		if (prm <= PARAM_END)
		{
			puterror(HSPERR_NO_DEFAULT);
		}
		const CStringW p2 = (const wchar_t*)HspVarStrW_Cnv(mpval->pt, mpval->flag);

		buf.Insert(p1, p2);
		ref_strwval = buf;
		ref_type = RET_TYPE::StrW;
		break;
	}
	case 0x16:								// strwcomp�֐�
	{
		int prm = code_getprm();				// �����l���擾(�f�t�H���g�Ȃ�)
		if (prm <= PARAM_END)
		{
			puterror(HSPERR_NO_DEFAULT);
		}
		CStringW buf = CString((const wchar_t*)HspVarStrW_Cnv(mpval->pt, mpval->flag));

		prm = code_getprm();				// �����l���擾(�f�t�H���g�Ȃ�)
		if (prm <= PARAM_END)
		{
			puterror(HSPERR_NO_DEFAULT);
		}
		const CStringW p1 = (const wchar_t*)HspVarStrW_Cnv(mpval->pt, mpval->flag);

		int p2 = code_geti();					// �����l���擾(�f�t�H���g�Ȃ�)

		ref_int64val = (p2 == 1) ? buf.CompareNoCase(p1) : buf.Compare(p1);
		ref_type = RET_TYPE::Int;
		break;
	}
	default:
		puterror( HSPERR_UNSUPPORTED_FUNCTION );
	}

	// �֐��`���̏ꍇ
	if (!bFunc)
	{
		//			')'�ŏI��邩�𒲂ׂ�
		//
		if (*type != TYPE_MARK)
		{
			puterror(HSPERR_INVALID_FUNCPARAM);
		}

		if (*val != ')')
		{
			puterror(HSPERR_INVALID_FUNCPARAM);
		}

		code_next();

		switch (ref_type)
		{
		case RET_TYPE::Void:
		case RET_TYPE::Int:
			*type_res = HSPVAR_FLAG_INT;
			return (void *)&ref_int64val;

		case RET_TYPE::Int64:
			*type_res = HspVarInt64_typeid();
			return (void *)&ref_int64val;

		case RET_TYPE::Double:
			*type_res = HSPVAR_FLAG_DOUBLE;
			return (void *)&ref_doubleval;

		case RET_TYPE::Float:
			*type_res = HspVarFloat_typeid();
			return (void *)&ref_floatval;

		case RET_TYPE::FloatInt:
			*type_res = HSPVAR_FLAG_INT;
			return (void *)&ref_floatval;

		case RET_TYPE::Str:
			*type_res = HSPVAR_FLAG_STR;
			return (void *)ref_strval.GetBuffer();

		case RET_TYPE::StrW:
			*type_res = HspVarStrW_typeid();
			return (void *)ref_strwval.GetBuffer();

		default:
			break;
		}
	}

	// ���Ȃ��z��
	return (void *)&ref_int64val;
}

static int cmdfunc(int cmd)
{
	code_next();

	switch (cmd) {
	case 0x02:								// qpoke
	{
		PVal *pval;
		APTR aptr = code_getva(&pval);
		int size = pval->size;
		int p1 = code_geti();

		if (p1<0) throw HSPERR_BUFFER_OVERFLOW;

		if ((p1 + 8)>size) throw HSPERR_BUFFER_OVERFLOW;

		int prm;
		prm = code_getprm();					// �����l���擾(�f�t�H���g�Ȃ�)
		if (prm <= PARAM_END) puterror(HSPERR_NO_DEFAULT);

		// Debug:
		// INT64 val = *(INT64 *)HspVarInt64_Cnv(mpval->pt, mpval->flag);

		*(INT64 *)((pval->pt) + p1) = *(INT64 *)HspVarInt64_Cnv(mpval->pt, mpval->flag);
		break;
	}
	case 0x04:								// dupptr64
	{
		PVal *pval_m;
		pval_m = code_getpval();
		int prm = code_getprm();
		if (prm <= PARAM_END) puterror(HSPERR_NO_DEFAULT);
		INT64 ptr = *(INT64 *)HspVarInt64_Cnv(mpval->pt, mpval->flag);
		if (NULL == ptr) throw HSPERR_ILLEGAL_FUNCTION;
		int p2 = code_geti();
		int p3 = code_getdi(HSPVAR_FLAG_INT);
		if (p2 <= 0) throw HSPERR_ILLEGAL_FUNCTION;

		if (exinfo->HspFunc_getproc(p3)->flag == 0) throw HSPERR_ILLEGAL_FUNCTION;
		HspVarCoreDupPtr(pval_m, p3, (void *)ptr, p2);
		break;
	}
	case 0x09:		// dim64
	case 0x0B:		// fdim
	case 0x0D:		// swdim
	{
		PVal *pval = code_getpval();

		int p1 = code_getdi(0);
		int p2 = code_getdi(0);
		int p3 = code_getdi(0);
		int p4 = code_getdi(0);
		if ( cmd == 0x0D)
		{
			int p5 = code_getdi(0);
			exinfo->HspFunc_dim(pval, HspVarStrW_typeid(), p1, p2, p3, p4, p5);
			break;
		}

		int fl;

		if (cmd == 0x09)
		{
			fl = HspVarInt64_typeid();
		}
		else if (cmd == 0x0B)
		{
			fl = HspVarFloat_typeid();
		}

		exinfo->HspFunc_dim(pval, fl, 1, p1, p2, p3, p4);		// ����ł����́H
		break;
	}
	case 0x0F:								// strwrep�֐�
	{
		int nTypeid_StrW = HspVarStrW_typeid();

		PVal *pval;
		APTR aptr;

		aptr = code_getva(&pval);
		if (pval->flag != nTypeid_StrW) throw HSPERR_TYPE_MISMATCH;

		HspVarProc *phvp;
		phvp = exinfo->HspFunc_getproc(pval->flag);
		CStringW buf = (wchar_t *)phvp->GetPtr(pval);

		int prm = code_getprm();					// �����l���擾(�f�t�H���g�Ȃ�)
		if (prm <= PARAM_END)
		{
			puterror(HSPERR_NO_DEFAULT);
		}
		const CStringW p1 = (const wchar_t*)HspVarStrW_Cnv(mpval->pt, mpval->flag);

		prm = code_getprm();						// �����l���擾(�f�t�H���g�Ȃ�)
		if (prm <= PARAM_END)
		{
			puterror(HSPERR_NO_DEFAULT);
		}
		const CStringW p2 = (const wchar_t*)HspVarStrW_Cnv(mpval->pt, mpval->flag);

		stat = buf.Replace(p1, p2);

		code_setva(pval, aptr, nTypeid_StrW, buf.GetBuffer());
		buf.ReleaseBuffer();

		break;
	}
	case 0x100:		// cfunc64v() �֐����Ɏ����A��
	{
		int type;
		reffunc( &type, CALL_FUNC_TYPE | cmd);
		break;
	}

	default:
		puterror(HSPERR_UNSUPPORTED_FUNCTION);
	}
	return RUNMODE_RUN;
}

/*------------------------------------------------------------*/

static int termfunc( int option )
{
	//		�I������ (�A�v���P�[�V�����I�����ɌĂ΂�܂�)
	//

	sbBye();

	return 0;
}


/*------------------------------------------------------------*/
/*
		interface
*/
/*------------------------------------------------------------*/

int WINAPI DllMain (HINSTANCE hInstance, DWORD fdwReason, PVOID pvReserved)
{
	//		DLL�G���g���[ (��������K�v�͂���܂���)
	//
	return TRUE;
}


EXPORT void WINAPI hsp3cmdinit( HSP3TYPEINFO *info )
{
	//		�v���O�C�������� (���s�E�I��������o�^���܂�)
	//
	hsp3sdk_init( info );		// SDK�̏�����(�ŏ��ɍs�Ȃ��ĉ�����)

	info->reffunc = reffunc;		// �Q�Ɗ֐�(reffunc)�̓o�^
	info->cmdfunc = cmdfunc;		// ���߂̓o�^
	info->termfunc = termfunc;		// �I���֐�(termfunc)�̓o�^

	sbInit();					// �σ������o�b�t�@������

	registvar( -1, HspVarInt64_Init );		// �V�����^�̒ǉ�
	registvar( -1, HspVarFloat_Init );		// �V�����^�̒ǉ�
	registvar( -1, HspVarStrW_Init);		// �V�����^�̒ǉ�
}

/*------------------------------------------------------------*/

