# PSAS PS3 Hooks

PSAS PS3 Hooks is a tool that allows for inserting function hooks in Playstation All-Stars Battle Royale for the Sony Playstation 3.
These function hooks allow you to inject and run your own custom C code within the game. 

# Prerequisites
- Visual Studio 2013+
- Sony PS3 4.75+ SDK w/ Visual Studio Integration
- [Fixed std::string library](https://github.com/skiff/libpsutil/releases "Fixed std::string library")

# How does this work/how can I insert my own hooks?
To insert a hook, you only need to provide an address in the game's EBOOT that you want to hook and a pointer to 
a function that contains some C code that you want to run inside your hook. You can define a new hook in `psabr.cpp`
as follows:

```C
// Define hooks here
#pragma region
Hook *ReadFile_Hook;
#pragma endregion

...

// Define detours/your own custom C code here
#pragma region
void ReadFile_Detour(const char *filename, unsigned int *info)
{
	printf("[hook] ReadFile	filename: 0x%lx, info: 0x%lx\n", filename, info); 
	printf("\tReading File: %s\n", filename);
}
#pragma endregion

...

void InstallHooks()
{
	// Tell the game where and how to insert your hook
	// void *ReadFile(char *file, int *file_info) @ 0x77554
	ReadFile_Hook = new Hook(0x7755C, (uintptr_t)ReadFile_Detour, POWERPC_REGISTERINDEX_R5);
}

void RemoveHooks()
{
	// Delete your hook once you exit out of the game
	delete ReadFile_Hook;
}
```

The hook works by overwriting the code at the address you specify to jump to a "trampoline". The trampoline:

1. Saves the "state" of the game by pushing all registers onto the stack
2. Jumps to your custom C code
3. Restores the "state" of the program by popping all registers off of the stack
4. Executes the instructions that the hook overwrote
5. Jumps back to the game's code just after the overwritten instructions

By saving all registers on the stack, the hook only overwrites a single register that it is not capable of restoring,
whatever initial register that was used to jump to the trampoline (register r5 in the above code example).

Not all addresses within the game are safe to hook. In particular, I had trouble hooking addresses that have instructions
that deal with saving and/or setting the link register. For example, most functions in the game have this instruction in its prologue:

```
mfspr r0, LR
```

Which saves the link register into r0. When I tried hooking over top of this instruction, I often crashed my game. I recommend inserting
your hook just past this instruction. Hooking the middle of functions should be possible as well.

# How to run
So far I've only tested my hooking code on RPCS3, although it should work on a PS3 with custom firmware as well. The instructions below
cover how to get set-up to run everything using RPCS3.

1. Go to the directory in which you've installed RPCS3. Edit `config.yml` and set the configuration option `Empty /dev_hdd0/tmp/: false` under `VFS`
(add this option if it does not exist).
2. In RPCS3, under `Configuration->CPU->`, set `PPU Decoder` to `Interpreter (static)`
3. Locate Playstation All-Stars `EBOOT.BIN`. If you've already installed the game in RPCS3, you can find this by right-clicking
the game icon and choosing `Open Install Folder`. The file should be located under `USRDIR`.
4. Decrypt `EBOOT.BIN` to an ELF by going into `Utilities->Decrypt PS3 Binaries` in RPCS3. Choose your `EBOOT.BIN`.
5. Download [SPRXPatcher](https://github.com/NotNite/SPRXPatcher). Run the below command:
	```bash
	dotnet run --project SPRXPatcher -- ./EBOOT.elf /dev_hdd0/tmp/PsasPs3Hooks.sprx ./PsasPs3Hooks.elf
4. Place 'PsasPs3Hooks.sprx' in `/dev_hdd0/tmp` in your RPCS3 folder
5. Place 'PsasPs3Hooks.elf' in the same folder as your `EBOOT.BIN`.
6. In RPCS3, go to `File->Boot (S)Elf` and choose `PsasPs3Hooks.elf`. Hit the Play button. If you did everything right, your hooks
get inserted.

# PSARC Dumping
The main use I've gotten out of this hooking framework so far is dumping the PS3 PSARCs. In the releases sidebar,
I've included an already built `PsasPs3Hooks.sprx` that you can use to dump the PSARCs yourself by following the above steps.
To dump the PSARCs, first boot the game. The hooks will first build a list of every file read into `/dev_hdd0/tmp/ps3_filenames.txt`.
The next time you boot the game, the hooks will read this file back and force the game to load every single file in that list into memory.
Once loaded, memory is then dumped back into your local filesystem under `/dev_hdd0/tmp`. See the below video for a demo of this functionality:

[![PSARC Dumping](https://img.youtube.com/vi/N-OY8vLMyoQ/0.jpg)](https://www.youtube.com/watch?v=N-OY8vLMyoQ&t=3s)

# Acknowledgements
- Thanks to duck from the RPCS3 discord for helping me get set-up with some general infrastructure and helping
me figure out how to patch my EBOOT to load my sprx 
- Thanks to [TheRouletteBoi](https://github.com/TheRouletteBoi), his hooking code for his [Minecraft Mod Menu](https://github.com/TheRouletteBoi/Minecraft) provided
a pretty good foundation for me to get started with writing my own hooking code.

# Questions
Feel free to message me on Discord (ps_chap#4898) if you have any questions or need help getting set-up.
