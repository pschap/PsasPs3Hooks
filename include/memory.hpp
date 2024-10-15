#pragma once

#include <stdint.h>
#include <stdio.h>
#include <ppu_asm_intrinsics.h>
#include <sys/process.h>

uint32_t sys_dbg_write_process_memory(uint32_t pid, void *address, const void *data, size_t size);
uint32_t sys_hen_write_process_memory(uint32_t pid, void *address, const void *data, size_t size);
uint32_t WriteProcessMemory(uint32_t pid, void *address, const void *data, size_t size);

void HexDump(const char *desc, const void *addr, const int len, int per_line);