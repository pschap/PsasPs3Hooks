#include <cell/cell_fs.h>

#include "utils.hpp"
#include "psabr.hpp"
#include "exports.hpp"
#include "psarc.hpp"

// Define hooks here
#pragma region
Hook *ReadFile_Hook;
Hook *PsasbrInit_Hook;
Hook *MountPsarc_Hook;
Hook *FUN_004998E0_Hook;
#pragma endregion

// Define detours here
#pragma region
void ReadFile_Detour(const char *filename, unsigned int *info)
{
	printf("[hook] ReadFile	filename: 0x%lx, info: 0x%lx\n", filename, info); 
	printf("\tReading File: %s\n", filename);
}

void PsasbrInit_Detour(uint32_t param_1, uint32_t param_2, uint32_t param_3)
{
	uint32_t i;
	int numLines;
	char **lines;
	char **tokens;
	char *line;
	char *psarc;
	char *filename;
	
	lines = ReadAllFilenames(&numLines);
	printf("Got %d filenames from %s\n", numLines, FILENAMES);
	
	if (lines != NULL)
	{
		// Iterate over each line
		for (i = 0; i < numLines; i++)
		{
			line = lines[i];
			tokens = str_split(line, ':', NULL);

			if (!tokens)
				continue;

			psarc = tokens[0];
			filename = tokens[1];

			if (psarc == NULL || *psarc == '\0')
				continue;

			if (filename == NULL || *filename == '\0')
				continue;

			// TODO
			if (i == 0)
				continue;
			
			LoadPsarcAndReadFile(psarc, filename);
			free(tokens);
		}

		free(lines);
	}
}

void MountPsarc_Detour(uint64_t param_1, const char *path)
{
	printf("[hook] MountPsarc	param_1: 0x%lx, path: 0x%lx\n", param_1, path);
	if (path)
		printf("\tLoading PSARC: %s\n", path);
}

void FUN_004998E0_Detour(uint64_t param_1, uint64_t param_2, uint64_t psarc_info, uint32_t entry_index)
{
	char *filename;
	char *psarc;
	uint32_t fiosObj;

	// Grab the name of the file being read
	fiosObj = *(uint32_t *)(param_2 + 0x24);
	filename = *(char **)(fiosObj + 0x8);
	
	// Grab the name of the PSARC that we're decompressing
	psarc = (char *)(psarc_info + 0x8C);

	// Write the PSARC and filename into a list of files that have been read
	WriteMediaInfoToFile(psarc, filename);
}

#pragma endregion

void InstallHooks()
{
	int fd;
	CellFsErrno err;

	// void *ReadFile(char *file, int *file_info) @ 0x77554
	// ReadFile_Hook = new Hook(0x7755C, (uintptr_t)ReadFile_Detour, POWERPC_REGISTERINDEX_R5);

	// undefined8 MountPsarc(longlong param_1, char *path) @ 0x76818
	// MountPsarc_Hook = new Hook(0x76820, (uintptr_t)MountPsarc_Detour, POWERPC_REGISTERINDEX_R5);

	// For PSARC Dumping; check if we have a list of filenames we can read from and insert hooks accordingly
	err = cellFsOpen(FILENAMES, CELL_FS_O_RDONLY, &fd, NULL, 0);
	if (err == CELL_FS_SUCCEEDED)
	{
		// int PsasbrInit(undefined4 param_1, int param_2, undefined4 param_3) @ 0x1f8cc
		PsasbrInit_Hook = new Hook(0x1F8D4, (uintptr_t)PsasbrInit_Detour, POWERPC_REGISTERINDEX_R6);
		cellFsClose(fd);
	}
	else
	{
		// void FUN_004998E0(ulonglong param_1, longlong param_2, PSARC_INFO *psarc_info, uint entry_index) @ 0x4998e0
		FUN_004998E0_Hook = new Hook(0x4998F4, (uintptr_t)FUN_004998E0_Detour, POWERPC_REGISTERINDEX_R7);
	}
}

void RemoveHooks()
{
	// delete ReadFile_Hook;
	// delete PsasbrInit_Hook;
	// delete MountPsarc_Hook;
	delete FUN_004998E0_Hook;
}