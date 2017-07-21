// zjg 20160816 add interface for pragamme report

// only support char-code utf8	

#ifndef ATTR_REPORT_H
#define ATTR_REPORT_H

#include <stdint.h>

namespace Report
{
	const uint16_t MAX_TEXT_INPUTLEN = 56;//for Server,Name,Text

	//注册进程序号，可选
	void set_proc_order_no(uint16_t u16ProcOrderNO = -1);

	// 累加上报
	int inc(const char* pszServer, const char* pszName, uint32_t Value);

	// 累加上报1
	int inc_once(const char* pszServer, const char* pszName);

	enum SetType 
	{
		ST_NONE = 0,
		ST_MIN = 1,
		ST_MAX = 2,
		ST_FIRST = 3,
		ST_LAST = 4,
	};
	// 覆盖上报
	int set(const char* pszServer, const char* pszName, SetType eType, uint32_t Value);

	// 告警
	int alarm(const char* pszServer, const char* pszName, const char* pszText);
}

#endif //ATTR_REPORT_H
