#include <cell/cell_fs.h>

#include "psabr.hpp"
#include "utils.hpp"
#include "exports.hpp"

// This is the path we'll use to write all of our files
#define WRITE_PATH "/dev_hdd0/tmp"

// Filename of file list
#define FILENAMES "/dev_hdd0/tmp/ps3_filenames.txt"

/***
* Write the name of the file and its corresponding PSARC to the file 
* containing a list of filenames.
* @param psarc name of PSARC 
* @param filename the name of the file being read
*/
void WriteMediaInfoToFile(char *psarc, char *filename);

/***
* Read all filenames from the file containing the files list.
* @param numLines number of lines read
* @returns all lines read from the files list
*/
char **ReadAllFilenames(int *numLines);

/***
* Load a PSARC and read a file contained in the corresponding PSARC.
* After the file has been read, dump its contents to our local filesystem.
* @param psarc the name of the archive
* @param filename the name of the file to read into memory and dump
*/
void LoadPsarcAndReadFile(char *psarc, char *filename);