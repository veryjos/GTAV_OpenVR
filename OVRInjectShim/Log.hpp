#pragma once

#include <cstdio>

#define LOGSTRF(format, ...) \
{ \
FILE* fp = fopen("gtavrInjectLog.txt", "a"); \
fprintf(fp, format, __VA_ARGS__); \
fclose(fp); \
}