#include <cstdio>
#include <stdarg.h>

#include "Log.hpp"

#include <Windows.h>

#pragma warning(push, 0)

void LOGSTRF(char* format, ...)
{
  va_list args;
  va_start(args, format);

  FILE* fp = fopen("gtavrInjectShimLog.txt", "a");
  vfprintf(fp, format, args);
  fclose(fp);

  va_end(args);
};

void LOGWNDF(char* format, ...)
{
  char* buf_fmtted = (char*)malloc(strlen(format) * 4 + 1);

  va_list args;
  va_start(args, format);
  vsprintf(buf_fmtted, format, args);

  va_end(args);

  MessageBoxA(0, buf_fmtted, "Error Logged", MB_OK);

  free(buf_fmtted);
};

void LOGFATALF(char* format, ...)
{
  char* buf_fmtted = (char*)malloc(strlen(format) * 4 + 1);

  va_list args;
  va_start(args, format);
  vsprintf(buf_fmtted, format, args);

  va_end(args);

  MessageBoxA(0, buf_fmtted, "Fatal Error Logged", MB_OK);

  free(buf_fmtted);

  exit(1);
};

void LOGOUTF(char* format, ...) {
  char buf_fmtted[4096];

  va_list args;
  va_start(args, format);
  vsprintf(buf_fmtted, format, args);

  va_end(args);

  OutputDebugStringA(buf_fmtted);
};

#pragma warning(pop)