// #include "stdafx.h"

#include <cellstatus.h>
#include <sys/prx.h>
#include <sys/ppu_thread.h>

#include "exports.hpp"
#include "psabr.hpp"

SYS_MODULE_INFO( PsasPs3Hooks, 0, 1, 1);
SYS_MODULE_START( _PsasPs3Hooks_prx_entry );
SYS_MODULE_STOP( _PsasPs3Hooks_prx_exit );

sys_ppu_thread_t psas_ppu_thread = SYS_PPU_THREAD_ID_INVALID;

extern "C" int _PsasPs3Hooks_prx_entry(int argc, char *argv[])
{
	sys_ppu_thread_create(&psas_ppu_thread, [](uint64_t arg)
	{
		InstallHooks();
		sys_ppu_thread_exit(0);
	}, 0, 3000, 0x8000, SYS_PPU_THREAD_CREATE_JOINABLE, "PsasPs3Hooks");

	return 0;
}

extern "C" int _PsasPs3Hooks_prx_exit(int argc, char *argv[])
{
	uint64_t ret;
	sys_ppu_thread_join(psas_ppu_thread, &ret);
	RemoveHooks();

	return 0;
}
