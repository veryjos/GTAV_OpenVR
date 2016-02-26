#include <cstdio>
#include <stdarg.h>

#include "Log.hpp"

#include <Windows.h>

#pragma warning(push, 0)

void LOGSTRF(char* format, ...)
{
	va_list args;
	va_start(args, format);

	FILE* fp = fopen("gtavrInjectLog.txt", "a");
	vfprintf(fp, format, args);
	fclose(fp);

	va_end(args);
};

void LOGWNDF(char* format, ...)
{
	WCHAR* wbuf_fmtted = (WCHAR*)malloc(strlen(format) * 8 + 1);
	WCHAR* wbuf = (WCHAR*)malloc(strlen(format) * 4 + 1);

	mbstowcs(wbuf, format, strlen(format) + 1);

	va_list args;
	va_start(args, format);
	wvsprintf(wbuf_fmtted, wbuf, args);

	va_end(args);

	MessageBox(0, wbuf_fmtted, L"Error Logged", MB_OK);

	free(wbuf);
};

void LOGFATALF(char* format, ...)
{
  WCHAR* wbuf_fmtted = (WCHAR*)malloc(strlen(format) * 8 + 1);
  WCHAR* wbuf = (WCHAR*)malloc(strlen(format) * 4 + 1);

  mbstowcs(wbuf, format, strlen(format) + 1);

  va_list args;
  va_start(args, format);
  wvsprintf(wbuf_fmtted, wbuf, args);

  va_end(args);

  MessageBox(0, wbuf_fmtted, L"Fatal Error Logged", MB_OK);

  free(wbuf);

  exit(1);
}

#pragma warning(pop)