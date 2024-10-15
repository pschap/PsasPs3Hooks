#include "hook.hpp"
#include "exports.hpp"
#include "powerpc.hpp"

// Initialize trampoline buffer
uint8_t Hook::tramps = 0;
uint8_t Hook::trampoline_buffer[MAX_HOOKS][TRAMPOLINE_BUFFER_SIZE] = { 0 };

Hook::Hook()
{
	this->hook_target = nullptr;
	this->detour = nullptr;
	this->register_index = POWERPC_REGISTERINDEX_R0;

	memset(this->stolen_instructions, 0, sizeof(this->stolen_instructions));
}

Hook::Hook(uintptr_t hook_target, uintptr_t detour, PPCRegister register_index)
{
	this->hook_target = reinterpret_cast<void*>(hook_target);
	this->detour = reinterpret_cast<void*>(detour);
	this->register_index = register_index;

	this->id = Hook::InitTrampoline((void *)detour, (void *)hook_target, register_index);
	memset(this->stolen_instructions, 0, sizeof(this->stolen_instructions));

	this->InstallHook();
}

Hook::~Hook()
{
	UninstallHook();
}

void Hook::InstallHook()
{
	// Get the size of the hook but don't hook anything yet
	size_t hook_size = GetHookSize(this->detour, false, false);

	// Save the original instructions for unhooking later on
	WriteProcessMemory(sys_process_getpid(), this->stolen_instructions, this->hook_target, hook_size);

	// Our trampoline should already be initialized at this point, so write in our jump to it
	this->Jump((void*)this->hook_target, &Hook::trampoline_buffer[this->id][0], false, false, this->register_index);
}

bool Hook::UninstallHook()
{
	size_t hook_size = GetHookSize(this->detour, false, false);
	if (this->hook_target && hook_size)
	{
		WriteProcessMemory(sys_process_getpid(), (void *)this->hook_target, this->stolen_instructions, hook_size);
		this->hook_target = nullptr;
		
		return true;
	}

	return false;
}

uint8_t Hook::InitTrampoline(void *detour, void *target, PPCRegister register_index)
{
	uint8_t id;
	size_t reg;
	uint32_t ret;
	uint32_t inst;
	uint32_t *instruction_addr;
	size_t tramp_index = 0;
	uint8_t ds = 0;

	target = reinterpret_cast<void*>(target);
	detour = reinterpret_cast<void*>(*reinterpret_cast<uintptr_t*>(detour));

	if (Hook::tramps < MAX_HOOKS)
	{	
		id = Hook::tramps;

		// Decrement stack pointer and store the old one on the stack
		inst = POWERPC_STDU(POWERPC_REGISTERINDEX_SP, -0x100, POWERPC_REGISTERINDEX_SP);
		WriteProcessMemory(sys_process_getpid(), &Hook::trampoline_buffer[id][tramp_index], &inst, sizeof(inst));
		tramp_index += sizeof(inst);
		ds += sizeof(uint64_t);

		// Store all registers on the stack
		for (reg = POWERPC_REGISTERINDEX_R0; reg <= POWERPC_REGISTERINDEX_R31; reg++)
		{
			if (reg == POWERPC_REGISTERINDEX_SP)
				continue;

			inst = POWERPC_STD(reg, ds, POWERPC_REGISTERINDEX_SP);
			WriteProcessMemory(sys_process_getpid(), &Hook::trampoline_buffer[id][tramp_index], &inst, sizeof(inst));
			tramp_index += sizeof(inst);
			ds += sizeof(uint64_t);
		}

		// Branch and link to our detour
		tramp_index += Hook::Jump(&Hook::trampoline_buffer[id][tramp_index], detour, true, false, register_index);

		// Load all saved registers back off of the stack
		ds = sizeof(uint64_t);
		for (reg = POWERPC_REGISTERINDEX_R0; reg <= POWERPC_REGISTERINDEX_R31; reg++)
		{
			if (reg == POWERPC_REGISTERINDEX_SP)
				continue;

			inst = POWERPC_LD(reg, ds, POWERPC_REGISTERINDEX_SP);
			WriteProcessMemory(sys_process_getpid(), &Hook::trampoline_buffer[id][tramp_index], &inst, sizeof(inst));
			tramp_index += sizeof(inst);
			ds += sizeof(uint64_t);
		}

		// Restore our stack pointer
		inst = POWERPC_ADDI(POWERPC_REGISTERINDEX_SP, POWERPC_REGISTERINDEX_SP, 0x100);
		WriteProcessMemory(sys_process_getpid(), &Hook::trampoline_buffer[id][tramp_index], &inst, sizeof(inst));
		tramp_index += sizeof(inst);

		// Copy stolen instructions from address we are hooking to our trampoline
		size_t hook_size = GetHookSize(detour, false, false);
		instruction_addr = reinterpret_cast<uint32_t*>((uint32_t)target);
		WriteProcessMemory(sys_process_getpid(), &Hook::trampoline_buffer[id][tramp_index], instruction_addr, hook_size);
		tramp_index += hook_size;

		// Jump back to our function
		ret = (uint32_t)target + hook_size;
		tramp_index += Hook::Jump(&Hook::trampoline_buffer[id][tramp_index], (void *)ret, true, false, register_index);

		Hook::tramps++;

		return id;
	}
}

size_t Hook::Jump(void *destination, const void *branch_target, bool linked, bool preserve_register, PPCRegister register_index)
{
	return JumpWithOptions(destination, branch_target, linked, preserve_register, POWERPC_BRANCH_OPTIONS_ALWAYS, 0, register_index, true);
}

size_t Hook::JumpWithOptions(void *destination, const void *branch_target, bool linked, bool preserve_register,
	uint32_t branch_options, uint8_t condition_register_bit, PPCRegister register_index, bool write)
{
	uint32_t *branch_asm;
	size_t branch_asm_size;

	uint32_t BranchFarAsm[] = {
		POWERPC_LIS(register_index, POWERPC_HI((uint32_t)branch_target)),						// lis   %rX, branchTarget@hi
		POWERPC_ORI(register_index, register_index, POWERPC_LO((uint32_t)branch_target)),		// ori   %rX, %rX, branchTarget@lo
		POWERPC_MTCTR(register_index),															// mtctr %rX
		POWERPC_BCCTR(branch_options, condition_register_bit, linked)							// bcctr (bcctr 20, 0 == bctr)
	};

	uint32_t BranchFarAsmPreserve[] = {
		POWERPC_STD(register_index, -0x30, POWERPC_REGISTERINDEX_SP),							// std   %rX, -0x30(%r1)
		POWERPC_LIS(register_index, POWERPC_HI((uint32_t)branch_target)),						// lis   %rX, branchTarget@hi
		POWERPC_ORI(register_index, register_index, POWERPC_LO((uint32_t)branch_target)),		// ori   %rX, %rX, branchTarget@lo
		POWERPC_MTCTR(register_index),															// mtctr %rX
		POWERPC_LD(register_index, -0x30, POWERPC_REGISTERINDEX_SP),							// ld    %rX, -0x30(%r1)
		POWERPC_BCCTR(branch_options, condition_register_bit, linked)							// bcctr (bcctr 20, 0 == bctr)
	};

	branch_asm = preserve_register ? BranchFarAsmPreserve : BranchFarAsm;
	branch_asm_size = preserve_register ? sizeof(BranchFarAsmPreserve) : sizeof(BranchFarAsm);

	if (write)
		WriteProcessMemory(sys_process_getpid(), destination, branch_asm, branch_asm_size);

	return branch_asm_size;
}

size_t Hook::RelocateBranch(uint32_t *destination, uint32_t *source)
{
	uint32_t instruction = *source;
	uint32_t instruction_addr = (uint32_t)source;

	// Absolute branches don't need to be handled
	if (instruction & POWERPC_BRANCH_ABSOLUTE)
	{
		WriteProcessMemory(sys_process_getpid(), destination, &instruction, sizeof(instruction));
		return sizeof(instruction);
	}

	int32_t branch_offset_bit_size = 0;
	int32_t branch_offset_bit_base = 0;
	uint32_t branch_options = 0;
	uint8_t condition_register_bit = 0;

	switch (instruction & POWERPC_OPCODE_MASK)
	{
		// B - Branch
		// [Opcode]            [Address]           [Absolute] [Linked]
		//   0-5                 6-29                  30        31
		//
		// Example
		//  010010   0000 0000 0000 0000 0000 0001      0         0
	case POWERPC_OPCODE_B:
		branch_offset_bit_size = 24;
		branch_offset_bit_base = 2;
		branch_options = POWERPC_BRANCH_OPTIONS_ALWAYS;
		condition_register_bit = 0;
		break;

		// BC - Branch Conditional
		// [Opcode]   [Branch Options]     [Condition Register]         [Address]      [Absolute] [Linked]
		//   0-5           6-10                    11-15                  16-29            30        31
		//
		// Example
		//  010000        00100                    00001             00 0000 0000 0001      0         0
	case POWERPC_OPCODE_BC:
		branch_offset_bit_size = 14;
		branch_offset_bit_base = 2;
		branch_options = (instruction >> POWERPC_BIT32(10)) & MASK_N_BITS(5);
		condition_register_bit = (instruction >> POWERPC_BIT32(15)) & MASK_N_BITS(5);
		break;
	}

	// Even though the address part of the instruction begins from bit 29 to the case of bc and b.
	// The value of the first bit is 4 as all addresses are aligned to for 4 for code therefore,
	// the branch offset can be calculated by anding in place and removing any suffix bits such as the
	// link register or absolute flags.
	int32_t branch_offset = instruction & (MASK_N_BITS(branch_offset_bit_size) << branch_offset_bit_base);

	// Check if the MSB of the offset is set
	if (branch_offset >> ((branch_offset_bit_size + branch_offset_bit_base) - 1))
	{
		// Add the necessary bits to our integer to make it negative
		branch_offset |= ~MASK_N_BITS(branch_offset_bit_size + branch_offset_bit_base);
	}

	void *branch_address = reinterpret_cast<void *>(instruction_addr + branch_offset);

	return JumpWithOptions(destination, branch_address, instruction & POWERPC_BRANCH_LINKED, true, branch_options, condition_register_bit, POWERPC_REGISTERINDEX_R0, true);
}

size_t Hook::RelocateCode(uint32_t *destination, uint32_t *source)
{
	uint32_t instruction = *source;
	switch (instruction & POWERPC_OPCODE_MASK)
	{
	case POWERPC_OPCODE_B:	// B BL BA BLA
	case POWERPC_OPCODE_BC:	// BEQ BNE BLT BGE
		return RelocateBranch(destination, source);
	default:
		WriteProcessMemory(sys_process_getpid(), destination, &instruction, sizeof(instruction));
		return sizeof(instruction);
	}
}

size_t Hook::GetHookSize(const void *branch_target, bool linked, bool preserve_register)
{
	return JumpWithOptions(nullptr, branch_target, linked, preserve_register, POWERPC_BRANCH_OPTIONS_ALWAYS, 0, POWERPC_REGISTERINDEX_R0, false);
}