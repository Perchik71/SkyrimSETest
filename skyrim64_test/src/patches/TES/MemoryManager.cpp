#include "../../common.h"
#include "MemoryManager.h"

void *Jemalloc(size_t Size, size_t Alignment = 0, bool Aligned = false, bool Zeroed = false)
{
	ProfileCounterInc("Alloc Count");
	ProfileCounterAdd("Byte Count", Size);
	ProfileTimer("Time Spent Allocating");

	void *ptr = nullptr;

	// Does this need to be on a certain boundary?
	if (Aligned)
	{
		AssertMsg(Alignment != 0 && Alignment % 2 == 0, "Alignment is fucked");

		// Must be a power of 2, round it up if needed
		if ((Alignment & (Alignment - 1)) != 0)
		{
			Alignment--;
			Alignment |= Alignment >> 1;
			Alignment |= Alignment >> 2;
			Alignment |= Alignment >> 4;
			Alignment |= Alignment >> 8;
			Alignment |= Alignment >> 16;
			Alignment++;
		}

		// Size must be a multiple of alignment, round up to nearest
		if ((Size % Alignment) != 0)
			Size = ((Size + Alignment - 1) / Alignment) * Alignment;

		ptr = je_aligned_alloc(Alignment, Size);
	}
	else
	{
		// Normal allocation
		ptr = je_malloc(Size);
	}

	if (ptr && Zeroed)
		return memset(ptr, 0, Size);

	return ptr;
}

void Jefree(void *Memory)
{
	ProfileCounterInc("Free Count");
	ProfileTimer("Time Spent Freeing");

	if (!Memory)
		return;

	je_free(Memory);
}

//
// VS2015 CRT hijacked functions
//
void *__fastcall hk_calloc(size_t Count, size_t Size)
{
	// The allocated memory is always zeroed
	return Jemalloc(Count * Size, 0, false, true);
}

void *__fastcall hk_malloc(size_t Size)
{
	return Jemalloc(Size);
}

void *__fastcall hk_aligned_malloc(size_t Size, size_t Alignment)
{
	return Jemalloc(Size, Alignment, true);
}

void __fastcall hk_free(void *Block)
{
	Jefree(Block);
}

void __fastcall hk_aligned_free(void *Block)
{
	Jefree(Block);
}

size_t __fastcall hk_msize(void *Block)
{
	return je_malloc_usable_size(Block);
}

//
// Internal engine heap allocators backed by VirtualAlloc()
//
void *MemoryManager::Alloc(size_t Size, uint32_t Alignment, bool Aligned)
{
	return Jemalloc(Size, Alignment, Aligned, true);
}

void MemoryManager::Free(void *Memory, bool Aligned)
{
	Jefree(Memory);
}

void *ScrapHeap::Alloc(size_t Size, uint32_t Alignment)
{
	if (Size > MAX_ALLOC_SIZE)
		return nullptr;

	return Jemalloc(Size, Alignment, Alignment != 0);
}

void ScrapHeap::Free(void *Memory)
{
	Jefree(Memory);
}

void PatchMemory()
{
	PatchIAT(hk_calloc, "API-MS-WIN-CRT-HEAP-L1-1-0.DLL", "calloc");
	PatchIAT(hk_malloc, "API-MS-WIN-CRT-HEAP-L1-1-0.DLL", "malloc");
	PatchIAT(hk_aligned_malloc, "API-MS-WIN-CRT-HEAP-L1-1-0.DLL", "_aligned_malloc");
	PatchIAT(hk_free, "API-MS-WIN-CRT-HEAP-L1-1-0.DLL", "free");
	PatchIAT(hk_aligned_free, "API-MS-WIN-CRT-HEAP-L1-1-0.DLL", "_aligned_free");
	PatchIAT(hk_msize, "API-MS-WIN-CRT-HEAP-L1-1-0.DLL", "_msize");

	PatchMemory(g_ModuleBase + 0x59B560, (PBYTE)"\xC3", 1);// [3GB  ] MemoryManager - Default/Static/File heaps
	PatchMemory(g_ModuleBase + 0x59B170, (PBYTE)"\xC3", 1);// [1GB  ] BSSmallBlockAllocator
														   // [512MB] hkMemoryAllocator is untouched due to complexity
														   // [128MB] BSScaleformSysMemMapper is untouched due to complexity
	PatchMemory(g_ModuleBase + 0xC02E60, (PBYTE)"\xC3", 1);// [64MB ] ScrapHeap init
	PatchMemory(g_ModuleBase + 0xC037C0, (PBYTE)"\xC3", 1);// [64MB ] ScrapHeap deinit

	Detours::X64::DetourFunctionClass((PBYTE)(g_ModuleBase + 0xC01DA0), &MemoryManager::Alloc);
	Detours::X64::DetourFunctionClass((PBYTE)(g_ModuleBase + 0xC020A0), &MemoryManager::Free);
	Detours::X64::DetourFunctionClass((PBYTE)(g_ModuleBase + 0xC02FE0), &ScrapHeap::Alloc);
	Detours::X64::DetourFunctionClass((PBYTE)(g_ModuleBase + 0xC03600), &ScrapHeap::Free);
}