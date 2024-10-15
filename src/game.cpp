#include "game.hpp"
#include "exports.hpp"

// Define hooks here
#pragma region
Hook *ReadFile_Hook;
#pragma endregion

// Define detours here
#pragma region
void ReadFile_Detour(const char *filename, unsigned int *info)
{
	printf("[hook] ReadFile	filename: 0x%lx, info: 0x%lx\n", filename, info); 
}
#pragma endregion

void InstallHooks()
{
	// void *ReadFile(char *file, int *file_info) @ 0x77554
	ReadFile_Hook = new Hook(0x7755C, (uintptr_t)ReadFile_Detour, POWERPC_REGISTERINDEX_R5);
}

void RemoveHooks()
{
	delete ReadFile_Hook;
}