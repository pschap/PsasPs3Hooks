#pragma once
#include <cell/cell_fs.h>

/***
* Split a string into multiple substrings based on a delimeter.
* @param str the string to split
* @param delim the delimeter to split on
* @param numSplits the number of times the string was split
* @returns array of string splits
*/
char** str_split(char* str, char delim, int* numSplits);

/***
* Create a directory recursively (a full path) by creating each individual directory in the path.
* @param dir the directory to create
*/
void RecursiveMkdir(const char *dir);