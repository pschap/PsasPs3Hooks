#include "psarc.hpp"
#include "powerpc.hpp"
#include "memory.hpp"

// Code to jump to LoadPsarc
MARK_AS_EXECUTABLE uint8_t LoadPsarcTramp[] = {
	0xF8, 0x21, 0xFF, 0xE9,		// stdu		r1, -0x18(r1)
	0X7C, 0x08, 0x02, 0xA6,		// mfspr	r0, LR
	0xF8, 0x01, 0x00, 0x10,		// std		r0, 0x10(r1)
	0xF8, 0x41, 0x00, 0x08,		// std		r2, 0x8(r1)
	0x3C, 0x40, 0x00, 0xCA,		// lis		r2, 0xca
	0x60, 0x42, 0x55, 0xF0,		// ori		r2, r2, 0x55f0
	0xF8, 0x21, 0xFF, 0xC1,		// stdu		r1, -0x40(r1)
	0x3C, 0xC0, 0x00, 0x07,		// lis		r6, 0x7
	0x60, 0xC6, 0x68, 0x18,		// ori		r6, r6, 0x6818
	0x7C, 0xC9, 0x03, 0xA6,		// mtctr	r6
	0x4E, 0x80, 0x04, 0x21,		// bctrl
	0x38, 0x21, 0x00, 0x40,		// addi		r1, r1, 0x40
	0xE8, 0x41, 0x00, 0x08,		// ld		r2, 0x8(r1)
	0xE8, 0x01, 0x00, 0x10,		// ld		r0, 0x10(1)
	0x7C, 0x08, 0x03, 0xA6,		// mtspr	LR, r0
	0x38, 0x21, 0x00, 0x18,		// addi		r1, r1, 0x18
	0x4E, 0x80, 0x00, 0x20		// blr
};

// Code to jump to ReadFile
MARK_AS_EXECUTABLE uint8_t ReadFileTramp[] = {
	0xF8, 0x21, 0xFF, 0xE9,		// stdu		r1, -0x18(r1)
	0X7C, 0x08, 0x02, 0xA6,		// mfspr	r0, LR
	0xF8, 0x01, 0x00, 0x10,		// std		r0, 0x10(r1)
	0xF8, 0x41, 0x00, 0x08,		// std		r2, 0x8(r1)
	0x3C, 0x40, 0x00, 0xCA,		// lis		r2, 0xca
	0x60, 0x42, 0x55, 0xF0,		// ori		r2, r2, 0x55f0
	0xF8, 0x21, 0xFF, 0xC1,		// stdu		r1, -0x40(r1)
	0x3C, 0xC0, 0x00, 0x07,		// lis		r6, 0x7
	0x60, 0xC6, 0x75, 0x54,		// ori		r6, r6, 0x7554
	0x7C, 0xC9, 0x03, 0xA6,		// mtctr	r6
	0x4E, 0x80, 0x04, 0x21,		// bctrl
	0x38, 0x21, 0x00, 0x40,		// addi		r1, r1, 0x40
	0xE8, 0x41, 0x00, 0x08,		// ld		r2, 0x8(r1)
	0xE8, 0x01, 0x00, 0x10,		// ld		r0, 0x10(1)
	0x7C, 0x08, 0x03, 0xA6,		// mtspr	LR, r0
	0x38, 0x21, 0x00, 0x18,		// addi		r1, r1, 0x18
	0x4E, 0x80, 0x00, 0x20		// blr
};

// Code to jump to PSABR's implementation of free
MARK_AS_EXECUTABLE uint8_t FreeTramp[] = {
	0xF8, 0x21, 0xFF, 0xE9,		// stdu		r1, -0x18(r1)
	0X7C, 0x08, 0x02, 0xA6,		// mfspr	r0, LR
	0xF8, 0x01, 0x00, 0x10,		// std		r0, 0x10(r1)
	0xF8, 0x41, 0x00, 0x08,		// std		r2, 0x8(r1)
	0x3C, 0x40, 0x00, 0xCA,		// lis		r2, 0xca
	0x60, 0x42, 0x55, 0xF0,		// ori		r2, r2, 0x55f0
	0xF8, 0x21, 0xFF, 0xC1,		// stdu		r1, -0x40(r1)
	0x3C, 0xC0, 0x00, 0x01,		// lis		r6, 0x1
	0x60, 0xC6, 0x0B, 0xD8,		// ori		r6, r6, 0xb0d8
	0x7C, 0xC9, 0x03, 0xA6,		// mtctr	r6
	0x4E, 0x80, 0x04, 0x21,		// bctrl
	0x38, 0x21, 0x00, 0x40,		// addi		r1, r1, 0x40
	0xE8, 0x41, 0x00, 0x08,		// ld		r2, 0x8(r1)
	0xE8, 0x01, 0x00, 0x10,		// ld		r0, 0x10(1)
	0x7C, 0x08, 0x03, 0xA6,		// mtspr	LR, r0
	0x38, 0x21, 0x00, 0x18,		// addi		r1, r1, 0x18
	0x4E, 0x80, 0x00, 0x20		// blr
};

uint32_t LoadPsarcTrampAddr = (uint32_t)(&LoadPsarcTramp);
uint32_t ReadFileTrampAddr = (uint32_t)(&ReadFileTramp);
uint32_t FreeTrampAddr = (uint32_t)(&FreeTramp);

void WriteMediaInfoToFile(char *psarc, char *filename)
{
	int fd;
	uint32_t i;
	uint64_t sw;
	size_t psarcLen;
	size_t filenameLen;
	CellFsErrno err;
	char *buffer;

	psarcLen = strlen(psarc);
	filenameLen = strlen(filename);

	// Allocate our buffer that we'll write to our file
	i = 0;
	buffer = (char *)malloc(psarcLen + filenameLen + 2);

	// Copy our PSARC name and filename into the buffer
	memcpy(buffer + i, psarc, psarcLen);
	i += psarcLen;
	buffer[i] = ':';
	i++;
	memcpy(buffer + i, filename, filenameLen);
	i += filenameLen;
	buffer[i] = '\n';
	i++;

	// Write our buffer to a file
	err = cellFsOpen(
		FILENAMES,
		CELL_FS_O_RDWR | CELL_FS_O_CREAT | CELL_FS_O_APPEND,
		&fd, NULL, 0);
	API_ERROR(err);

	err = cellFsWrite(fd, (const void *)buffer, psarcLen + filenameLen + 2, &sw);
	API_ERROR(err);
	free(buffer);
	buffer = NULL;

	cellFsClose(fd);
}

char **ReadAllFilenames(int *numLines)
{
	int fd;
	void *buffer;
	char **lines;
	uint64_t sr;
	CellFsErrno err;
	CellFsStat status;

	// First get the size of our file and allocate a buffer big enough to hold it (plus 1 for null byte)
	err = cellFsStat(FILENAMES, &status);
	API_ERROR(err);
	printf("Size of file %s: %d\n", FILENAMES, status.st_size);
	buffer = malloc(status.st_size + 1);
	if (buffer == NULL)
		return NULL;

	memset(buffer, 0, status.st_size + 1);

	// Try to read our filenames file
	printf("Opening file %s...\n", FILENAMES);
	err = cellFsOpen(FILENAMES, CELL_FS_O_RDWR, &fd, NULL, 0);
	API_ERROR(err);

	printf("Reading file %s...\n", FILENAMES);
	err = cellFsRead(fd, buffer, status.st_size, &sr);
	API_ERROR(err);
	
	printf("Closing file %s...\n", FILENAMES);
	err = cellFsClose(fd);
	API_ERROR(err);

	// Split our file into individual lines
	lines = str_split((char *)buffer, '\n', numLines);
	free(buffer);
	buffer = NULL;

	return lines;
}

void *LoadPsarcAndReadFile(char *psarc, char *filename)
{
	void *buffer;
	uint32_t pos;
	size_t psarcLen;
	size_t filenameLen;
	size_t pathLen;
	uint32_t fileSize;
	char *wildcardFilename;
	char *outFilename;
	CellFsErrno err;
	int fd;
	uint64_t sw;

	void * (*ReadFile)(const char *path, uint32_t *fileSize) = (
		void * (*)(const char *, uint32_t *))&ReadFileTrampAddr;

	int(*LoadPsarc)(uint32_t param_1, const char *psarc) = (
		int(*)(uint32_t, const char *))&LoadPsarcTrampAddr;

	void(*Free)(void *ptr) = (void(*)(void *))&FreeTrampAddr;

	// First load the PSARC
	printf("Loading PSARC: %s...\n", (char *)psarc + 1);
	if (!LoadPsarc(0x2274B10, (char *)(psarc + 1)))
		return NULL;

	// Allocate a wildcard filename (just the filename prepended with a '$')
	wildcardFilename = (char *)malloc(strlen(filename) + 2);
	if (!wildcardFilename)
		return NULL;

	wildcardFilename[0] = '$';
	strcpy(wildcardFilename + 1, filename);

	// Read the file into a buffer
	printf("Reading file: %s...\n", wildcardFilename);
	buffer = ReadFile(wildcardFilename, &fileSize);
	if (buffer)
	{
		pathLen = strlen(WRITE_PATH);
		psarcLen = strlen(psarc);
		filenameLen = strlen(filename);
		pos = 0;
		outFilename = (char *)malloc(pathLen + psarcLen + filenameLen + 1);
		strcpy(outFilename + pos, WRITE_PATH);
		pos += pathLen;
		strcpy(outFilename + pos, psarc);
		pos += psarcLen;
		strcpy(outFilename + pos, wildcardFilename + 1);
		pos += filenameLen;
		outFilename[pos] = '\0';
		printf("Writing file: %s...\n", outFilename);
		RecursiveMkdir(outFilename);

		// Now write our file!!!
		err = cellFsOpen(
			outFilename,
			CELL_FS_O_RDWR | CELL_FS_O_CREAT,
			&fd, NULL, 0);
		API_ERROR(err);

		err = cellFsWrite(fd, (const void *)buffer, fileSize, &sw);
		cellFsClose(fd);

		free(outFilename);
		Free(buffer);
	}

	free(wildcardFilename);

	return NULL;
}