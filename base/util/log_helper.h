#ifndef LOG_HELPER_H
#define LOG_HELPER_H

//---------------Log-------------------
#define LOG_MMAP_FILE		"/ecim/agent/log/log.mmap"

//IPC 协议：	u8LogType + u8LogLevel + u8DateCategory + u32LimitLengthPerFile + u8FileCycleNum + u16FilePathLen * c8FilePath + u8KeywordLen * c8Keyword + u16TextLen * c8Text

enum LOG_TYPE
{
	LT_NONE = 0,
	LT_LOCAL = 1,
	LT_REMOTE = 2,
	LT_DUPLET = 3,
};

#endif
