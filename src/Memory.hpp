#pragma once

#include "framework.h"

namespace Memory
{
	DWORD GetAddressBySignature(std::vector<int> signature, DWORD startaddress = 0, DWORD endaddress = 0) {
		SYSTEM_INFO si;
		GetSystemInfo(&si);
		if (startaddress == 0) {
			startaddress = (DWORD)(si.lpMinimumApplicationAddress);
		}
		if (endaddress == 0) {
			endaddress = (DWORD)(si.lpMaximumApplicationAddress);
		}

		MEMORY_BASIC_INFORMATION mbi;
		DWORD protectflags = (PAGE_GUARD | PAGE_NOCACHE | PAGE_NOACCESS | PAGE_READONLY);

		for (DWORD i = startaddress; i < endaddress - signature.size(); i++) {
			if (VirtualQuery((LPCVOID)i, &mbi, sizeof(mbi))) {
				if (mbi.Protect & protectflags || !(mbi.State & MEM_COMMIT)) {
					i += mbi.RegionSize;
					continue;
				}
				for (DWORD k = (DWORD)mbi.BaseAddress; k < (DWORD)mbi.BaseAddress + mbi.RegionSize - signature.size(); k++) {
					for (DWORD j = 0; j < signature.size(); j++) {
						if (signature.at(j) != (BYTE)-1 && signature.at(j) != *(byte*)(k + j))
							break;
						if (j + 1 == signature.size())
							return k;
					}
				}
				i = (DWORD)mbi.BaseAddress + mbi.RegionSize;
			}
		}
		return NULL;
	}

	void MakeJump(BYTE* pAddress, DWORD dwJumpTo, DWORD dwLen)
	{
		DWORD dwOldProtect, dwBkup, dwRelAddr;

		VirtualProtect(pAddress, dwLen, PAGE_EXECUTE_READWRITE, &dwOldProtect);

		dwRelAddr = (DWORD)(dwJumpTo - (DWORD)pAddress) - 5;

		*pAddress = 0xE9;

		*((DWORD*)(pAddress + 0x1)) = dwRelAddr;

		for (DWORD x = 0x5; x < dwLen; x++) *(pAddress + x) = 0x90;

		VirtualProtect(pAddress, dwLen, dwOldProtect, &dwBkup);

		return;
	}

	void MakePatch(BYTE* pAddr, BYTE* signature, DWORD dwLen)
	{
		DWORD dwOldProtect, dwBkup;

		VirtualProtect(pAddr, dwLen, PAGE_EXECUTE_READWRITE, &dwOldProtect);

		for (DWORD x = 0; x < dwLen; x++) {
			*(pAddr + x) = signature[x];
		}

		VirtualProtect(pAddr, dwLen, dwOldProtect, &dwBkup);
	}
}