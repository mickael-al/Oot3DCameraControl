#include <intrin.h>
#include "proc.h"

DWORD GetProcId(const wchar_t* procName)
{
	DWORD procId = 0;
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnap != INVALID_HANDLE_VALUE)
	{
		PROCESSENTRY32 procEntry;
		procEntry.dwSize = sizeof(procEntry);

		if (Process32First(hSnap, &procEntry))
		{
			do
			{
				if (!_wcsicmp(procEntry.szExeFile, procName))
				{
					procId = procEntry.th32ProcessID;
					break;
				}

			} while (Process32Next(hSnap, &procEntry));
		}
	}
	CloseHandle(hSnap);
	return procId;
}

uintptr_t GetModuleBaseAddress(DWORD procId, const wchar_t* modName)
{
	uintptr_t modBaseAddr = 0;
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procId);
	if (hSnap != INVALID_HANDLE_VALUE)
	{
		MODULEENTRY32 modEntry;
		modEntry.dwSize = sizeof(modEntry);
		if (Module32First(hSnap, &modEntry))
		{
			do
			{
				if (!_wcsicmp(modEntry.szModule, modName))
				{
					modBaseAddr = (uintptr_t)modEntry.modBaseAddr;
					break;
				}
			} while (Module32Next(hSnap, &modEntry));
		}
	}
	CloseHandle(hSnap);
	return modBaseAddr;
}

MODULEENTRY32 GetModuleEntry(DWORD procId, const wchar_t* modName)
{
	MODULEENTRY32 modBaseAddr;
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procId);
	if (hSnap != INVALID_HANDLE_VALUE)
	{
		MODULEENTRY32 modEntry;
		modEntry.dwSize = sizeof(modEntry);
		if (Module32First(hSnap, &modEntry))
		{
			do
			{
				if (!_wcsicmp(modEntry.szModule, modName))
				{
					modBaseAddr = modEntry;
					break;
				}
			} while (Module32Next(hSnap, &modEntry));
		}
	}
	CloseHandle(hSnap);
	return modBaseAddr;
}


uintptr_t FindDMAAddy(HANDLE hProc, uintptr_t ptr, std::vector<unsigned int> offsets)
{
	uintptr_t addr = ptr;
	for (unsigned int i = 0; i < offsets.size(); i++)
	{
		ReadProcessMemory(hProc, (BYTE*)addr, &addr, sizeof(addr), 0);
		addr += offsets[i];
	}
	return addr;
}

struct PartData
{
	int32_t mask = 0;
	__m128i needle;

	PartData()
	{
		memset(&needle, 0, sizeof(needle));
	}
};

// Credits to @DarthTon for this function (source: https://github.com/learn-more/findpattern-bench/blob/master/patterns/DarthTon.h)
const void* Search(const uint8_t* data, const uint32_t size, const uint8_t* pattern, const char* mask)
{
	const uint8_t* result = nullptr;
	auto len = strlen(mask);
	auto first = strchr(mask, '?');
	size_t len2 = (first != nullptr) ? (first - mask) : len;
	auto firstlen = min(len2, 16);
	intptr_t num_parts = (len < 16 || len % 16) ? (len / 16 + 1) : (len / 16);
	PartData parts[4];

	for (intptr_t i = 0; i < num_parts; ++i, len -= 16)
	{
		for (size_t j = 0; j < min(len, 16) - 1; ++j)
			if (mask[16 * i + j] == 'x')
				_bittestandset((LONG*)&parts[i].mask, j);

		parts[i].needle = _mm_loadu_si128((const __m128i*)(pattern + i * 16));
	}

	bool abort = false;

#pragma omp parallel for
	for (intptr_t i = 0; i < static_cast<intptr_t>(size) / 32 - 1; ++i)
	{
#pragma omp flush (abort)
		if (!abort)
		{
			auto block = _mm256_loadu_si256((const __m256i*)data + i);
			if (_mm256_testz_si256(block, block))
				continue;

			auto offset = _mm_cmpestri(parts->needle, firstlen, _mm_loadu_si128((const __m128i*)(data + i * 32)), 16, _SIDD_CMP_EQUAL_ORDERED);
			if (offset == 16)
			{
				offset += _mm_cmpestri(parts->needle, firstlen, _mm_loadu_si128((const __m128i*)(data + i * 32 + 16)), 16, _SIDD_CMP_EQUAL_ORDERED);
				if (offset == 32)
					continue;
			}

			for (intptr_t j = 0; j < num_parts; ++j)
			{
				auto hay = _mm_loadu_si128((const __m128i*)(data + (2 * i + j) * 16 + offset));
				auto bitmask = _mm_movemask_epi8(_mm_cmpeq_epi8(hay, parts[j].needle));
				if ((bitmask & parts[j].mask) != parts[j].mask)
					goto next;
			}

			result = data + 32 * i + offset;
			abort = true;
#pragma omp flush (abort)
		}

	next:;
	}

	return result;
}

uintptr_t SearchInProcessMemory(HANDLE hProcess, const uint8_t* pattern, const char* mask)
{
	const int BUFFER_SIZE = 1024 * 1024;
	uint8_t* buffer = new uint8_t[BUFFER_SIZE];
	uintptr_t result = 0x0;
	uint8_t* readAddr = nullptr;

	while (result == 0)
	{
		SIZE_T bytesRead = 0;
		BOOL success = ReadProcessMemory(hProcess, readAddr, buffer, BUFFER_SIZE, &bytesRead);
		if (!success || bytesRead == 0) {
			readAddr += BUFFER_SIZE;
			continue;
		}

		const void* ptr = Search(buffer, bytesRead, pattern, mask);
		if (ptr != nullptr)
		{
			result = (uintptr_t)readAddr + ((uintptr_t)ptr - (uintptr_t)buffer);
			break;
		}

		readAddr += BUFFER_SIZE;
	}
	delete[] buffer;

	return result;
}
