#pragma once
//#include "stdlib.h"
//#include "stdio.h"
//#include "string.h"

const int LINE_READ_FILE_MAX_FILENAME = 256;
const int LINE_READ_FILE_MAX_LINECOUNT = 240960;
class CLineReadFile
{
public:
	CLineReadFile(char *strFileName);
	~CLineReadFile(void);

	int		readline();
	char	*ltrim(char *str, int len);
	char	*ltrim();

protected:
	char *m_strFileName;
	char *m_strReadLine;
	FILE *m_hFile;
	int  m_nLastReadCount;
};

