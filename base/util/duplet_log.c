#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/stat.h>

#include "mmap_queue.h"
#include "log_helper.h"
#include "host_codec.h"
#include "string_func.h"
#include "duplet_log.h"

#ifndef MAX_PATH
#define MAX_PATH (260)
#endif

static bool Inner_Init();

class CInitFilePathWrapper
{
public:
	CInitFilePathWrapper()
	{
		Inner_Init();
	}
};

static uint16_t				s_u16ProcOrderNO = -1;

static char					s_szFileName[MAX_PATH] = { 0 };
static char					s_szDirName[MAX_PATH] = { 0 };
static uint16_t				s_u16FilePathLen = 0;
static char					s_szFilePath[MAX_PATH] = { 0 };// s_szFilePath = s_szDirName + s_szFileName

static enum Log::LOG_LEVEL	s_eLocalLowestLevel = Log::LL_WARN;
static enum Log::LOG_LEVEL	s_eRemoteLowestLevel = Log::LL_NONE;

static struct mmap_queue*	s_queue = NULL;

static	bool				s_bDateCategory = true;
static	uint32_t			s_u32LimitLengthPerFile = Log::DEFAULT_LIMITLENGTH_PERFILE;
static  uint8_t				s_u8FileCycleNum = Log::DEFAULT_FILE_NUM;

static CInitFilePathWrapper	s_objInit;//for call init

static bool MakeFullPath()
{
	if (!strlen(s_szDirName))
	{
		char szProcessPath[MAX_PATH] = { 0 };
		int cnt = readlink("/proc/self/exe", szProcessPath, MAX_PATH);
		if (cnt < 0 || cnt >= MAX_PATH)
		{
			return false;
		}
		szProcessPath[cnt] = 0;

		char* pszProcessName = strrchr(szProcessPath, '/');
		if (!pszProcessName)
		{
			return false;
		}
		*pszProcessName = 0;
				
		strlcpy(s_szDirName, szProcessPath, sizeof(s_szDirName));
	}

	strlcpy(s_szFilePath, s_szDirName, sizeof(s_szFilePath));
	size_t len = strlen(s_szFilePath);
	if (len > 0)
	{
		if (len + 1 >= sizeof(s_szFilePath))
		{
			return false;
		}
		if (s_szFilePath[len - 1] != '/')
		{
			strlcat(s_szFilePath, "/", sizeof(s_szFilePath));		
			len++;
		}

		if (len + 1 >= sizeof(s_szFilePath))
		{
			return false;
		}
	}

	char* pszProcessName = s_szFileName;
	if (!pszProcessName || !strlen(pszProcessName))
	{
		char szProcessPath[MAX_PATH] = { 0 };
		int cnt = readlink("/proc/self/exe", szProcessPath, MAX_PATH);
		if (cnt < 0 || cnt >= MAX_PATH)
		{
			return false;
		}
		szProcessPath[cnt] = 0;

		pszProcessName = strrchr(szProcessPath, '/');
		if (!pszProcessName)
		{
			return false;
		}
		++pszProcessName;
	}
	strlcat(s_szFilePath, pszProcessName, sizeof(s_szFilePath));
	s_u16FilePathLen = (uint16_t)strlen(s_szFilePath);

	return true;
}

//初始化
static bool Inner_Init()
{
	s_queue = mq_open(LOG_MMAP_FILE);
	if (s_queue == NULL)
	{
		return false;
	}

	Log::set_file_name();
	
	return true;
}

//u8LogType + u8LogLevel + u8DateCategory + u32LimitLengthPerFile + u8FileCycleNum + u16FilePathLen * c8FilePath + u8KeywordLen * c8Keyword + u16TextLen * c8Text
static int InnerLog(LOG_TYPE eLogType, Log::LOG_LEVEL eLogLevel, const char* pszKeyword, const char* pszFormat, va_list va)
{
	//filter
	switch (eLogType)
	{
	case LT_LOCAL:
		if (eLogLevel < s_eLocalLowestLevel)
		{
			return 1;
		}
		break;
	case LT_REMOTE:
		if (eLogLevel < s_eRemoteLowestLevel)
		{
			return 1;
		}
		break;
	case LT_DUPLET:
		if (eLogLevel < s_eLocalLowestLevel)
		{
			return InnerLog(LT_REMOTE, eLogLevel, pszKeyword, pszFormat, va);
		}
		if (eLogLevel < s_eRemoteLowestLevel)
		{
			return InnerLog(LT_LOCAL, eLogLevel, pszKeyword, pszFormat, va);
		}
		break;
	default:
		return -1;
	}

	// 挂载mmap
	if (NULL == s_queue)
	{
		return -1;
	}

	int iKeywordLen = strlen(pszKeyword);
	if (iKeywordLen > Log::MAX_KEYWORD_LEN)
	{
		return -2;
	}
	char szText[Log::MAX_TEXT_LEN];
	vsnprintf(szText, sizeof(szText), pszFormat, va);

	//input
	uint8_t	buf[sizeof(uint8_t) * 5 + sizeof(uint32_t) + sizeof(uint16_t) + Log::MAX_TEXT_LEN + Log::MAX_KEYWORD_LEN];
	host_codec os(sizeof(buf), buf);

	os << static_cast<uint8_t>(eLogType);
	os << static_cast<uint8_t>(eLogLevel);
	os << static_cast<uint8_t>(s_bDateCategory);
	os << s_u32LimitLengthPerFile;
	os << s_u8FileCycleNum;

	if (!AppendU16Str(os, s_szFilePath, s_u16FilePathLen))
	{
		return -2;
	}
	if (!AppendU8Str(os, pszKeyword, (uint8_t)iKeywordLen))
	{
		return -2;
	}
	if (!AppendU16Str(os, szText))
	{
		return -2;
	}
	if (!os)
	{
		return -2;
	}

	return mq_put(s_queue, (void*)buf, os.wpos());
}

////////////////////////////////////////////////////////////////////////////
//可选，默认内部会自动设置为进程名
bool Log::set_file_name(const char* pszFileName/* = NULL*/, uint16_t u16ProcOrderNO/* = -1*/)
{
	if (pszFileName)
	{
		strlcpy(s_szFileName, pszFileName, sizeof(s_szFileName));
	}
	
	if (!MakeFullPath())
	{
		return false;
	}

	s_u16ProcOrderNO = u16ProcOrderNO;

	return true;
}

//可选，默认为程序运行路径所在目录	
bool Log::set_local_cfg(LOG_LEVEL eLowestLevel/* = LL_TRACE*/, const char* pszDirPath/* = NULL*/, bool bDateCategory/* = true*/, uint32_t u32LimitLengthPerFile/* = DEFAULT_LIMITLENGTH_PERFILE*/, uint8_t u8FileCycleNum/* = DEFAULT_FILE_NUM*/)
{
	s_eLocalLowestLevel = eLowestLevel;

	if (pszDirPath)
	{
		strlcpy(s_szDirName, pszDirPath, sizeof(s_szDirName));
	}
	if (!MakeFullPath())
	{
		return false;
	}

	s_bDateCategory = bDateCategory;

	s_u32LimitLengthPerFile = u32LimitLengthPerFile;

	s_u8FileCycleNum = u8FileCycleNum;

	return true;
}

//可选。不小于其参数值执行远端写log
bool Log::set_remote_cfg(LOG_LEVEL eLowestLevel/* = LL_NONE*/)
{
	s_eRemoteLowestLevel = eLowestLevel;

	return true;
}

int Log::Local::trace(const char* pszKeyword, const char* pszFormat, ...)
{
	va_list va;
	va_start(va, pszFormat);
	int iRet = InnerLog(LT_LOCAL, LL_TRACE, pszKeyword, pszFormat, va);
	va_end(va);

	return iRet;
}

int Log::Local::info(const char* pszKeyword, const char* pszFormat, ...)
{
	va_list va;
	va_start(va, pszFormat);
	int iRet = InnerLog(LT_LOCAL, LL_INFO, pszKeyword, pszFormat, va);
	va_end(va);

	return iRet;
}

int Log::Local::warn(const char* pszKeyword, const char* pszFormat, ...)
{
	va_list va;
	va_start(va, pszFormat);
	int iRet = InnerLog(LT_LOCAL, LL_WARN, pszKeyword, pszFormat, va);
	va_end(va);

	return iRet;
}

int Log::Local::error(const char* pszKeyword, const char* pszFormat, ...)
{
	va_list va;
	va_start(va, pszFormat);
	int iRet = InnerLog(LT_LOCAL, LL_ERROR, pszKeyword, pszFormat, va);
	va_end(va);

	return iRet;
}

int Log::Local::fatal(const char* pszKeyword, const char* pszFormat, ...)
{
	va_list va;
	va_start(va, pszFormat);
	int iRet = InnerLog(LT_LOCAL, LL_FATAL, pszKeyword, pszFormat, va);
	va_end(va);

	return iRet;
}

int Log::Remote::trace(const char* pszKeyword, const char* pszFormat, ...)
{
	va_list va;
	va_start(va, pszFormat);
	int iRet = InnerLog(LT_REMOTE, LL_TRACE, pszKeyword, pszFormat, va);
	va_end(va);

	return iRet;
}

int Log::Remote::info(const char* pszKeyword, const char* pszFormat, ...)
{
	va_list va;
	va_start(va, pszFormat);
	int iRet = InnerLog(LT_REMOTE, LL_INFO, pszKeyword, pszFormat, va);
	va_end(va);

	return iRet;
}

int Log::Remote::warn(const char* pszKeyword, const char* pszFormat, ...)
{
	va_list va;
	va_start(va, pszFormat);
	int iRet = InnerLog(LT_REMOTE, LL_WARN, pszKeyword, pszFormat, va);
	va_end(va);

	return iRet;
}

int Log::Remote::error(const char* pszKeyword, const char* pszFormat, ...)
{
	va_list va;
	va_start(va, pszFormat);
	int iRet = InnerLog(LT_REMOTE, LL_ERROR, pszKeyword, pszFormat, va);
	va_end(va);

	return iRet;
}

int Log::Remote::fatal(const char* pszKeyword, const char* pszFormat, ...)
{
	va_list va;
	va_start(va, pszFormat);
	int iRet = InnerLog(LT_REMOTE, LL_FATAL, pszKeyword, pszFormat, va);
	va_end(va);

	return iRet;
}

int Log::Duplet::trace(const char* pszKeyword, const char* pszFormat, ...)
{
	va_list va;
	va_start(va, pszFormat);
	int iRet = InnerLog(LT_DUPLET, LL_TRACE, pszKeyword, pszFormat, va);
	va_end(va);

	return iRet;
}

int Log::Duplet::info(const char* pszKeyword, const char* pszFormat, ...)
{
	va_list va;
	va_start(va, pszFormat);
	int iRet = InnerLog(LT_DUPLET, LL_INFO, pszKeyword, pszFormat, va);
	va_end(va);

	return iRet;
}

int Log::Duplet::warn(const char* pszKeyword, const char* pszFormat, ...)
{
	va_list va;
	va_start(va, pszFormat);
	int iRet = InnerLog(LT_DUPLET, LL_WARN, pszKeyword, pszFormat, va);
	va_end(va);

	return iRet;
}

int Log::Duplet::error(const char* pszKeyword, const char* pszFormat, ...)
{
	va_list va;
	va_start(va, pszFormat);
	int iRet = InnerLog(LT_DUPLET, LL_ERROR, pszKeyword, pszFormat, va);
	va_end(va);

	return iRet;
}

int Log::Duplet::fatal(const char* pszKeyword, const char* pszFormat, ...)
{
	va_list va;
	va_start(va, pszFormat);
	int iRet = InnerLog(LT_DUPLET, LL_FATAL, pszKeyword, pszFormat, va);
	va_end(va);

	return iRet;
}
