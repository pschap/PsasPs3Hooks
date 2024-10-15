#pragma once

#include <string>
#include <sys/process.h>

#include "memory.hpp"
#include "powerpc.hpp"

// Debug prints
#define DEBUG 1

// Marker for definining executable sections of memory
#define MARK_AS_EXECUTABLE __attribute__((section(".text")))

// Number of bytes in trampoline
#define TRAMPOLINE_BUFFER_SIZE 312

// Maximum number of hooks that can be inserted
#define MAX_HOOKS 256

// Maximum number of bytes stolen when inserting the hook
#define MAX_STOLEN_BYTES 30

class Hook
{
public:
	Hook();
	Hook(uintptr_t hook_target, uintptr_t detour, PPCRegister register_index = POWERPC_REGISTERINDEX_R0);
	Hook(Hook const&) = delete;
	Hook(Hook&&) = delete;
	Hook& operator=(Hook const &) = delete;
	Hook& operator=(Hook&&) = delete;
	virtual ~Hook();

	/***
	* Installs the hook at the specified address.
	*/
	virtual void InstallHook();

	/***
	* Uninstalls hook and performs cleanup by restoring original instructions.
	* @returns true if hook could be successfully uninstalled, false otherwise
	*/
	virtual bool UninstallHook();

private:

	/***
	* Initialize the hook trampoline buffer.
	* @param detour address of detour function
	* @param target address to insert hook
	* @param register_index register to use for jump
	* @returns index into trampoline buffer where trampoline was written
	*/
	static uint8_t InitTrampoline(void *detour, void *target, PPCRegister register_index);

	/***
	* Writes an unconditional branch to the destination address that will branch to the target address.
	* @param destination address that the branch will be written to
	* @param branch_target target of the branch
	* @param linked set link register in branch
	* @param preserve_register preserve the register clobbered after loading the branch address
	* @param register_index Register to use when loading the destination address into the count register.
	* @returns number of bytes needed to overwrite the instructions at the destination address to perform the desired branch
	*/
	static size_t Jump(void *destination, const void *branch_target, bool linked, bool preserve_register, PPCRegister register_index);

	/***
	* Writes both conditional and unconditional branches using the count register to the destination address that will branch to the target address.
	* @param destination Where the branch will be written to.
	* @param branch_target The address the branch will jump to.
	* @param linked Branch is a call or a jump? aka bl or b
	* @param preserve_register Preserve the register clobbered after loading the branch address.
	* @param branch_options Options for determining when a branch to be followed.
	* @param condition_register_bit The bit of the condition register to compare.
	* @param register_index Register to use when loading the destination address into the count register.
	* @param write write the jumpcode to memory
	* @returns size of relocating the instruction in bytes
	*/
	static size_t JumpWithOptions(void* destination, const void* branch_target, bool linked, bool preserve_register,
		uint32_t branch_options, uint8_t condition_register_bit, PPCRegister register_index, bool write);

	/***
	* Copies and fixes relative branch instructions to a new location.
	* @param destination address to write the new branch
	* @param source address of the instruction that is being relocated
	* @returns 
	*/
	static size_t RelocateBranch(uint32_t *destination, uint32_t *source);

	/***
	* Copies an instruction to a new location ensuring PC relative offsets are fixed.
	* @param destination address that the new instruction will be written to
	* @param source address of the instruction that is being relocated
	* @returns
	*/
	static size_t RelocateCode(uint32_t *destination, uint32_t *source);

	/***
	* Get's size of method hook in bytes
	* @param branch_target The address the branch will jump to.
	* @param linked set link register in branch
	* @param preserve_register preserve the register clobbered after loading the branch address\
	* @returns size of hook in bytes
	*/
	static size_t GetHookSize(const void *branch_target, bool linked, bool preserve_register);

protected:
	const void*  hook_target;								// The address of the function we are hooking
	void*		 detour;									// Address of hook detour function
	uint8_t		 stolen_instructions[MAX_STOLEN_BYTES];		// Overwritten/stolen instructions from the hook target
	size_t		 size_stolen_bytes;							// Number of bytes overwritten/stolen at the hook target
	PPCRegister  register_index;							// Index of register to use in unconditional jump at hook target
	uint8_t		 id;										// Identifier for hook

	// Buffer to hold trampoline
	MARK_AS_EXECUTABLE static uint8_t trampoline_buffer[MAX_HOOKS][TRAMPOLINE_BUFFER_SIZE];

	// Number of trampolines currently in use
	static uint8_t tramps;
};