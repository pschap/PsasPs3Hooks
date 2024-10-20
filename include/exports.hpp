#pragma once
#include <stdio.h>
#include <libpsutil.h>
#include <cell/cell_fs.h>

// File IO Helper
#define API_ERROR(x) {if(x!=CELL_FS_SUCCEEDED){printf("error=0x%x\n",x);sys_process_exit(1);}}

#define ARRAYSIZE(a) (sizeof(a) / sizeof(*(a)))

extern "C" void* _sys_malloc(size_t size);
#define malloc _sys_malloc

extern "C" void* _sys_calloc(size_t nitems, size_t size);
#define calloc _sys_calloc

extern "C" void _sys_free(void* ptr);
#define free _sys_free

extern "C" void* _sys_memalign(size_t boundary, size_t size);
#define memalign _sys_memalign

extern "C" int _sys_printf(const char* format, ...);
#define printf _sys_printf

extern "C" int _sys_sprintf(char* buffer, const char* format, ...);
#define sprintf _sys_sprintf

extern "C" int _sys_snprintf(char* buffer, int size, const char* format, ...);
#define snprintf _sys_snprintf

extern "C" int _sys_strncasecmp(const char* s1, const char* s2, size_t n);
#define strncasecmp _sys_strncasecmp

extern "C" char* _sys_strncat(char* dest, const char* src, size_t n);
#define strncat _sys_strncat

extern "C" int _sys_strncmp(const char* s1, const char* s2, size_t n);
#define strncmp _sys_strncmp

extern "C" char* _sys_strncpy(char* dest, const char* src, size_t n);
#define strncpy _sys_strncpy

extern "C" int _sys_memcmp(const void* s1, const void* s2, size_t n);
#define memcmp _sys_memcmp

extern "C" void* _sys_memcpy(void* dest, const void* src, size_t n);
#define memcpy _sys_memcpy

extern "C" void* _sys_memset(void* m, int c, size_t n);
#define memset _sys_memset

extern "C" void* _sys_memmove(void* str1, const void* str2, size_t n);
#define memmove _sys_memmove

extern "C" int _sys_vsprintf(char* str, const char* format, va_list arg);
#define vsprintf _sys_vsprintf

extern "C" int _sys_vsnprintf(char* s, size_t n, const char* fmt, va_list arg);
#define vsnprintf _sys_vsnprintf