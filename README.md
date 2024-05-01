# Indigo
This is a _toy/educational_ OS/kernel I made for the sole purpose of learning how these work and how one goes about making them.

### Basic anatomy:
Most (all) of the code is in the `src/` directory, with subdirs containing different "bits":
- `src/loader` is a UEFI-compatible executable which loads the kernel image + trampoline image into memory,
builds a memory map for it to read and launches it (and also handles UEFI stuff/exiting boot services)
- `src/trampoline` is what the loader jumps to, which handles setting up higher-half paging and mapping the kernel to negative 2GB
in the address space (in accordance with relocation model = kernel), and then calls the actual kernel
- `src/kernel` is the "actual" kernel which handles everything else.
- `src/libs` is code I wrote when I didn't know where to put code, and might eventually vanish, but fttb it's a libraries directory
for things I might find useful. It has some overlap with kernel's `utils` subdir though.
#### Deps
The only code dependency this thing has is `posix-uefi` from `bzt (bztsrc@gitlab)`, which is used in `loader` only
to make programming under UEFI more palatable.

### Progress:
So far, the OS boots, handles 64-bit mode, and starts up all other cores detected,
and all the other stuff required to get to a point where starting cores is a reasonable thing to do.
The current focus is kernel modules, which we'll use to make an initramfs-stored file loader to load
other modules which will contain everything else, from Actual Modules (shared objs, not just non-relocated objects)
to the scheduler and multi-processing management stuff.
We've also got a depencency-graph style loader, so we can write modules for everything and only load the ones which are actually required.

### Installing and running this:
#### Installing
If you received this via `git clone` then the git stuff should be setup, otherwise run `git init` as normal.
Install the (singular) runtime dependency with `git submodule init && git submodule update`. The only change that's been made to submodules is the removal
of line 73 of `vendor/posix-uefi/uefi/uefi.h` (the `wchar_t` def), which you might need to do. (TODO: fork posix-uefi)
This project has a large number of build dependencies, most of which I cannot remember. Off the top of my head you may need:
- clang/clang++ & GNU Make
- ld.lld (I think this comes with clang)
- NASM
- lldb (if debugging)
- qemu (for running in a VM)
- mkgpt
- the edk2-ovmf QEMU firmware (I have no idea how I installed this one, try looking [here](https://github.com/tianocore/tianocore.github.io/wiki/OVMF))
- A suitable PC Screen Font file symlinked to `src/kernel/arch/x86_64/font.psfu.gz`, which might be set up already but I'm not sure.
#### Running
This project is by no means guaranteed to work on your machine.
It also probably wants a Linux environment, I have no idea if this works under cygwin/WSL.
To run, just invoke `make run` to run without debugging, or `make run_dbg` to launch the same image with QEMU bound to a LLDB session.

### legacy content for legacy me:
sane coding conventions:
- CONSTS\_IN\_SCREAMING\_SNAKE
- variables\_in\_snake
- TypesInPascal
- functionsInCamel
- membersInCamel
