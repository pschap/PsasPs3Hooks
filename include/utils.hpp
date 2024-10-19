#pragma once
#include <cell/cell_fs.h>

char** str_split(char* str, char delim, int* numSplits);
void RecursiveMkdir(const char *dir);