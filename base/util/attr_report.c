#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "mmap_queue.h"
#include "report_helper.h"
#include "host_codec.h"

#include "attr_report.h"

#define MAX_TEXT_LIMITLEN		(Report::MAX_TEXT_INPUTLEN + 10) //append "_xxxxx"

static bool Inner_Init();

class CInitFilePathWrapper
{
public:
	CInitFilePathWrapper()
	{
		Inner_Init();
	}
};

static struct mmap_queue*	s_queue = NULL;
static uint16_t				s_u16ProcOrderNO = -1;
static CInitFilePathWrapper	s_objInit;//for call init

//初始化
static bool Inner_Init()
{
	s_queue = mq_open(REPORT_MMAP_FILE);
	if (s_queue == NULL)
	{
		return false;
	}
	return true;
}

static bool SerializeBasic(host_codec& os, const uint8_t u8Type, const char* pszServer, const char* pszName)
{
	if (CheckText(pszServer, Report::MAX_TEXT_INPUTLEN) < 0)
	{
		return -1;
	}
	if (CheckText(pszName, Report::MAX_TEXT_INPUTLEN) < 0)
	{
		return -1;
	}

	char*		pszFullServerName = (char*)pszServer;

	char		szFullServerName[MAX_TEXT_LIMITLEN];
	if (s_u16ProcOrderNO != (uint16_t)-1)
	{
		snprintf(szFullServerName, sizeof(szFullServerName), "%s_%u", pszServer, s_u16ProcOrderNO);
		pszFullServerName = szFullServerName;
	}
	if (strlen(pszFullServerName) > MAX_TEXT_LIMITLEN)
	{
		return -1;
	}

	os << u8Type;
	if (!AppendU8Str(os, pszFullServerName))
	{
		return -2;
	}
	if (!AppendU8Str(os, pszName))
	{
		return -2;
	}

	return 0;
}

//注册进程号，可选
void Report::set_proc_order_no(uint16_t u16ProcOrderNO/* = -1*/)
{
	s_u16ProcOrderNO = u16ProcOrderNO;
}

//IPC 协议：	u8Type + u8ServerLen * sServer + u8NameLen * sName + stOther
//					u8Type	=	1,	表示	inc,	stOther = u32Value
//							=	2,	表示	set,	stOther = u8SetType + u32Value，其中u8Alarm同SetType
//							=	3,	表示	alarm，	stOther = u8AlarmLen * c8Alarm

// 累加上报
int Report::inc(const char* pszServer, const char* pszName, uint32_t Value)
{
	// 挂载mmap
	if (NULL == s_queue)
	{
		return -1;
	}
	
	uint8_t	buf[sizeof(uint8_t) * 3 + Report::MAX_TEXT_INPUTLEN + MAX_TEXT_LIMITLEN + sizeof(uint32_t)];
	host_codec os(sizeof(buf), buf);
	if(SerializeBasic(os, 1, pszServer, pszName) < 0)
	{
		return -2;
	}		

	os << Value;
	if (!os)
	{
		return -2;
	}

	return mq_put(s_queue, (void*)os.data(), os.wpos());
}

int Report::inc_once(const char* pszServer, const char* pszName)
{
	return inc(pszServer, pszName, 1);
}

// 覆盖上报
int Report::set(const char* pszServer, const char* pszName, SetType eType, uint32_t Value)
{
	// 挂载mmap
	if (NULL == s_queue)
	{
		return -1;
	}

	uint8_t	buf[sizeof(uint8_t) * 4 + Report::MAX_TEXT_INPUTLEN + MAX_TEXT_LIMITLEN + sizeof(uint32_t)];
	host_codec os(sizeof(buf), buf);
	if (SerializeBasic(os, 2, pszServer, pszName) < 0)
	{
		return -2;
	}

	os << static_cast<uint8_t>(eType) << Value;
	if (!os)
	{
		return -2;
	}

	return mq_put(s_queue, (void*)os.data(), os.wpos());
}

// 上报字符串
int Report::alarm(const char* pszServer, const char* pszName, const char* pszText)
{
	// 挂载mmap
	if (NULL == s_queue)
	{
		return -1;
	}
	if (CheckText(pszText, Report::MAX_TEXT_INPUTLEN) < 0)
	{
		return -3;
	}

	uint8_t	buf[sizeof(uint8_t) * 4 + Report::MAX_TEXT_INPUTLEN + MAX_TEXT_LIMITLEN];
	host_codec os(sizeof(buf), buf);
	if (SerializeBasic(os, 3, pszServer, pszName) < 0)
	{
		return -2;
	}
	if (!AppendU8Str(os, pszText))
	{
		return -2;
	}

	return mq_put(s_queue, (void*)os.data(), os.wpos());
}

