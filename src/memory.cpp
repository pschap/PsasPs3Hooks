#include "memory.hpp"
#include "exports.hpp"

uint32_t sys_dbg_write_process_memory(uint32_t pid, void *address, const void *data, size_t size)
{
	system_call_4(905, (uint64_t)pid, (uint64_t)address, (uint64_t)size, (uint64_t)data);
	return_to_user_prog(uint32_t);
}

uint32_t sys_hen_write_process_memory(uint32_t pid, void *address, const void *data, size_t size)
{
	system_call_6(8, 0x7777, 0x32, (uint64_t)pid, (uint64_t)address, (uint64_t)size, (uint64_t)data);
	return_to_user_prog(uint32_t);
}

uint32_t WriteProcessMemory(uint32_t pid, void *address, const void *data, size_t size)
{
	static bool useHenSyscalls = false;

	if (!useHenSyscalls)
	{
		uint32_t write = sys_dbg_write_process_memory(pid, address, data, size);
		if (write == SUCCEEDED)
		{
			return write;
		}
	}

	useHenSyscalls = true;
	return sys_hen_write_process_memory(pid, address, data, size);
}

void HexDump(const char *desc, const void *addr, const int len, int per_line)
{
	unsigned int i;
	unsigned int idx;
	unsigned char *buff;
	const unsigned char *pc = (const unsigned char *)addr;
	const unsigned int base = (const unsigned int)addr;

	buff = (unsigned char *)malloc(static_cast<size_t>(per_line));
	if (!buff)
	{
		printf("Hex Dump Error: Unable to allocate memory for buffer.\n");
		return;
	}

	// Silently ignore silly per-line values
	if (per_line < 4 || per_line > 64)
		per_line = 16;

	// Output description if given
	if (desc)
		printf("%s:\n", desc);

	// Length Checks
	if (len == 0)
	{
		printf("Hex Dump Error: Zero Length\n");
		return;
	}

	if (len < 0)
	{
		printf("Hex Dump Error: Negative Length\n");
		return;
	}

	// Process every byte in data
	for (i = 0, idx = 0; i < len; i++)
	{
		if (i == 0)
			printf("  %04x: ", base + i);

		// Multiple of per_line means new or first line (with line offset)
		if (idx == per_line)
		{
			idx = 0;
			if (i != 0)
				printf("  %s\n", buff);

			// Output the offset of current line
			printf("  %04x: ", base + i);
		}

		// Now the hex code for the specific character
		printf(" %02x", pc[i]);

		// And buffer a printable ASCII character for later
		if (pc[i] < 0x20 || pc[i] > 0x7e)
			buff[idx] = '.';
		else
			buff[idx] = pc[i];

		buff[idx + 1] = '\0';
		idx++;
	}

	// Pad out the last line if not exact per_line characters
	while (idx != per_line)
	{
		printf("   ");
		idx++;
	}

	// And print the final ASCII buffer
	printf("  %s\n", buff);

	free(buff);
	buff = nullptr;
}
