
//
//	strbuf.cpp header
//
#ifndef __strbuf_h
#define __strbuf_h

#define STRBUF_BLOCKSIZE 64
#define STRBUF_BLOCKSIZE_W 128

//	STRBUF structure
//

typedef struct STRBUF STRBUF;

typedef struct
{
	//	String Data structure
	//
	short flag;						// �g�p�t���O(0=none/other=busy)
	short exflag;					// �g���t���O(���g�p)
	STRBUF *intptr;					// ���g�̃A�h���X
	int size;						// �m�ۃT�C�Y
	char *ptr;						// �o�b�t�@�|�C���^
	STRBUF *extptr;					// �O���o�b�t�@�|�C���^(STRINF)
	void *opt;						// �I�v�V����(���[�U�[��`�p)
} STRINF;

struct STRBUF
{
	//	String Data structure
	//
	STRINF inf;						// �o�b�t�@���
	char data[STRBUF_BLOCKSIZE] ;	// �����o�b�t�@
};

char *sbAlloc( int size );
char *sbAllocClear( int size );
void sbFree( void *ptr );
char *sbExpand( char *ptr, int size );
STRINF *sbGetSTRINF( char *ptr );

void sbCopy( char **ptr, char *data, int size );
void sbStrCopy( char **ptr, char *str );
void sbAdd( char **ptr, char *data, int size, int offset );
void sbStrAdd( char **ptr, char *str );

void *sbGetOption( char *ptr );
void sbSetOption( char *ptr, void *option );

#endif
