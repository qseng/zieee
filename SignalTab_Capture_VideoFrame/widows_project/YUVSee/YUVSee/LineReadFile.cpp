#include "stdafx.h"
#include "LineReadFile.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

CLineReadFile::CLineReadFile(char *strFileName):m_hFile(NULL),m_strReadLine(NULL),m_nLastReadCount(0)
{
	if ( strFileName && *strFileName )
	{
		int len = strlen(strFileName);
		m_strFileName = new char[len+1];
		memset(m_strFileName, 0, len+1);
		strcpy(m_strFileName, strFileName);
	}
	else
	{
		m_strFileName = NULL;
	}
}

CLineReadFile::~CLineReadFile(void)
{
	if ( m_strFileName )
	{
		delete[] m_strFileName;
	}
	if ( m_strReadLine )
	{
		delete [] m_strReadLine;
	}
	if ( m_hFile )
	{
		fclose(m_hFile);
	}
}

int CLineReadFile::readline()
{
	int iRet = -1;
	m_nLastReadCount = 0;
	if (!m_hFile )
	{
		m_hFile = fopen(m_strFileName, "rb"); 
		if ( !m_hFile )
			return iRet;
		m_strReadLine = new char[LINE_READ_FILE_MAX_LINECOUNT+1];
		memset(m_strReadLine, 0, LINE_READ_FILE_MAX_LINECOUNT+1);
	}

	int iCount = 0; 
	char c;
	while ( iCount < LINE_READ_FILE_MAX_LINECOUNT )
	{
		c = getc(m_hFile);
		if ( c == '\n' || c == EOF )
		{
			iCount++;
			break;
		}
		m_strReadLine[iCount++] = c;
	}

	m_strReadLine[iCount -1] = '\0';
	m_nLastReadCount = iCount -1;
	return iCount -1;
}

char *CLineReadFile::ltrim(char *str, int len)
{
	char *p;
	p = str;
	while ( *p == ' ' || *p == '\t' ) {*p++;}
	memcpy(str, p, len+1);
	return str;
}

char *CLineReadFile::ltrim()
{
	return m_nLastReadCount > 0 ? ltrim(m_strReadLine, m_nLastReadCount) : NULL;
}
