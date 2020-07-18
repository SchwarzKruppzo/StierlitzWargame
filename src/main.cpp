#include <iostream>
#include <Windows.h>
#include <string>
#include <vector>
#include "sigscan.h"

bool bStopThreads = false;

#include "pTimer.hpp"

HMODULE hModule;
HANDLE hMainThread;
DWORD dwBaseAddress;
DWORD dwFogPointer;
DWORD dwCameraPointer;
DWORD dwRangePointer;
DWORD dwJump1;
DWORD dwJump2;
DWORD dwScopeFunc;
DWORD dwJmpScopeFunc;
DWORD dwBaseScopeAddress;
DWORD scopeX = 0;
DWORD scopeY = 0;
DWORD scopeZ = 0;
BYTE flags;

static const BYTE sigOrigRange[] = { 0x0F, 0x86, 0x4E, 0x02, 0x00, 0x00, 0xC7, 0x07, 0x00, 0x00, 0x00, 0x00, 0xE9, 0x63, 0x02 };
static const BYTE sigMinimap1[] = { 0x7B, 0x76, 0x69, 0x65, 0x77, 0x20, 0x22, 0x68, 0x75, 0x6D, 0x61, 0x6E, 0x22, 0x0D, 0x0A, 0x09, 0x09, 0x28, 0x22, 0x73, 0x63, 0x61, 0x6C, 0x65, 0x22, 0x20, 0x73, 0x28, 0x30, 0x2E, 0x34, 0x35, 0x29, 0x29, 0x0D, 0x0A, 0x09, 0x09, 0x7B, 0x70, 0x72, 0x6F, 0x70, 0x73, 0x20, 0x22, 0x68, 0x75, 0x6D, 0x61, 0x6E, 0x22, 0x7D, 0x0D, 0x0A, 0x09, 0x7D }; // minimap.set
static const BYTE sigMinimap2[] = { 0x09, 0x7B, 0x6F, 0x6E, 0x20, 0x22, 0x64, 0x69, 0x65, 0x5F, 0x73, 0x63, 0x72, 0x65, 0x61, 0x6D, 0x22 };
static const BYTE sigMinimap3[] = { 0x7B, 0x6F, 0x6E, 0x20, 0x22, 0x6C, 0x69, 0x6E, 0x6B, 0x5F, 0x77, 0x65, 0x61, 0x70, 0x6F, 0x6E, 0x22, 0x0D, 0x0A, -1, -1, 0x7B, 0x69, 0x66, 0x20, 0x73, 0x74, 0x75, 0x66, 0x66 }; // {on "die"
static const BYTE sigMinimap3_Robz[] = { 0x7B, 0x6F, 0x6E, 0x20, 0x22, 0x6C, 0x69, 0x6E, 0x6B, 0x5F, 0x77, 0x65, 0x61, 0x70, 0x6F, 0x6E, 0x22, 0x0D, 0x0A, 0x0D, 0x0A, 0x7B, 0x69, 0x66, 0x20, 0x73, 0x74, 0x75, 0x66, 0x66 };
static const BYTE patchMinimap1[] = { 0x28, 0x69, 0x6E, 0x63, 0x6C, 0x75, 0x64, 0x65, 0x20, 0x22, 0x63, 0x2E, 0x78, 0x65, 0x78, 0x22, 0x29, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20 };
static const BYTE patchMinimap2[] = { 0x28, 0x69, 0x6E, 0x63, 0x6C, 0x75, 0x64, 0x65, 0x20, 0x22, 0x63, 0x2E, 0x64, 0x64, 0x73, 0x22, 0x29 };
static const BYTE patchMinimap3[] = { 0x28, 0x69, 0x6E, 0x63, 0x6C, 0x75, 0x64, 0x65, 0x20, 0x22, 0x61, 0x2E, 0x64, 0x64, 0x73, 0x22, 0x29, 0x0D, 0x0A, 0x0D, 0x0A, 0x7B, 0x69, 0x66, 0x20, 0x73, 0x74, 0x75, 0x66, 0x66 };
static const BYTE sigOrigScope[] = { 0xC7, 0x87, 0xCC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

static const DWORD offsetsInf[] = {
	0x7E76BE,
	0x7E7717,
	0x7A5E91,
	0x7E3FDD,
	0x7E42AF
};

static const BYTE sigInfPatch1[6][2] = {
	{0x90, 0x90}, // patch
	{0x77, 0x13},
	{0xEB, 0x10}, 
	{0x7A, 0x79}, // restore
	{0x7A, 0x13},
	{0x7A, 0x10},
};

static const BYTE sigInfPatch2[4][3] = {
	{0x90, 0x90, 0x90}, // patch
	{0x0F, 0x96, 0xC0},
	{0x0F, 0x93, 0xC3}, // restore
	{0x0F, 0x92, 0xC0},
};

bool bIsInGame = false;

struct activated {
	bool fog = false;
	bool camera = false;
	bool range = false;
	bool resources = false;
	bool scope = false;
} activated;

DWORD GetAddressFromSignature(std::vector<int> signature, DWORD startaddress = 0, DWORD endaddress = 0) {
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

void PatchSig(BYTE * pAddr, BYTE *signature, DWORD dwLen)
{
	DWORD dwOldProtect, dwBkup;

	VirtualProtect(pAddr, dwLen, PAGE_EXECUTE_READWRITE, &dwOldProtect);

	for (DWORD x = 0; x < dwLen; x++) {
		*(pAddr + x) = signature[x];
	}

	VirtualProtect(pAddr, dwLen, dwOldProtect, &dwBkup);
}

void SwitchInfiniteResources(bool activated)
{
	if (dwBaseAddress == NULL) {
		return;
	}

	if (activated) {
		for (int i = 0; i < 5; i++) {
			DWORD dwAddr = (dwBaseAddress + offsetsInf[i]);

			if (i > 2) {
				PatchSig((BYTE*)dwAddr, (BYTE*)sigInfPatch2[i % 3], 0x03);
			}
			else {
				PatchSig((BYTE*)dwAddr, (BYTE*)sigInfPatch1[i], 0x02);
			}
		}
	} else {
		for (int i = 0; i < 5; i++) {
			DWORD dwAddr = (dwBaseAddress + offsetsInf[i]);

			if (i > 2) {
				PatchSig((BYTE*)dwAddr, (BYTE*)sigInfPatch2[2 + (i % 3)], 0x03);
			}
			else {
				PatchSig((BYTE*)dwAddr, (BYTE*)sigInfPatch1[3 + (i % 3)], 0x02);
			}
		}
	}
}

BYTE GetActivePlayer()
{
	if (dwBaseAddress == NULL) {
		return 0;
	}

	return *(BYTE*)(dwBaseAddress + 0xB39E64);
}

BYTE GetMaxSlots()
{
	if (dwBaseAddress == NULL) {
		return 0;
	}

	return *(BYTE*)(dwBaseAddress + 0xB39E68);
}

bool IsValidPlayer(int playerID)
{
	if (dwBaseAddress == NULL) {
		return false;
	}

	int maxSlots = GetMaxSlots();
	
	if (playerID >= maxSlots) {
		return false;
	}

	DWORD gameBase = *(DWORD*)(dwBaseAddress + 0xBED0C4);

	if (gameBase == NULL) {
		return false;
	}

	DWORD players = *(DWORD*)(gameBase + 0xC4);

	if (players == NULL) {
		return false;
	}

	DWORD basePlayer = *(DWORD*)(players);

	if (basePlayer == NULL) {
		return false;
	}

	DWORD player = *(DWORD*)(basePlayer + ((playerID - 1) * 0xB0) + 0x08);

	return player != NULL;
}

int SwitchCurrentPlayer(bool reset) {
	BYTE activePlayer = GetActivePlayer();
	BYTE targetPlayer = reset ? 1 : (activePlayer + 1);

	if (!IsValidPlayer(targetPlayer)) {
		targetPlayer = 1;
	}

	*(BYTE*)(dwBaseAddress + 0xB39E64) = targetPlayer;

	return targetPlayer;
}

__declspec(naked) void InfiniteRange()
{
	__asm
	{
		lahf
		mov [flags], ah
	}

	if (!activated.range) {
		__asm
		{
			mov ah, [flags]
			sahf
			jbe $+15
			mov [edi], 0x00000000
			jmp [dwJump1]
			jmp [dwJump2]
		}
	} else {
		__asm
		{
			jmp [dwJump2]
		}
	}
}

__declspec(naked) void ForceCollisionScope()
{
	if (!activated.scope) {
		__asm
		{
			mov [edi + 0xCC], 0
		}
	}
	else {
		__asm
		{
			mov [edi + 0xCC], 0x4
		}
	}

	__asm
	{
		jmp [dwJmpScopeFunc]
	}
}

void ScopeFreezer() {
	if (bIsInGame && activated.scope) {
		DWORD addr1 = (*(DWORD*)dwBaseScopeAddress);

		if (addr1 != NULL) {
			if (*(DWORD*)addr1 != 0x0) {
				DWORD addr2 = addr1 + 0x240;

				if (addr2 != NULL) {
					if (*(DWORD*)addr2 != 0x0) {
						DWORD scope = (*(DWORD*)addr2) + 0xE8;

						if (scope != NULL) {
							DWORD x = (*(DWORD*)scope) + 0x9C;
							if (x != NULL) {
								*(DWORD*)x = scopeX;
							}

							DWORD y = (*(DWORD*)scope) + 0xA0;
							if (y != NULL) {
								*(DWORD*)y = scopeY;
							}

							DWORD z = (*(DWORD*)scope) + 0xA4;
							if (z != NULL) {
								*(DWORD*)z = scopeZ;
							}
						}
					}
				}
			}
		}
	}
}

void OnDetach(HMODULE hModule) {
	while (true)
	{
		if (GetAsyncKeyState(VK_F9) & 1)
		{
			std::cout << ">Unloading Stierlitz from the game process..." << std::endl;
			bStopThreads = true;

			Sleep(1000);

			PatchSig((BYTE*)dwScopeFunc, (BYTE*)sigOrigScope, 0xA);
			PatchSig((BYTE*)dwRangePointer, (BYTE*)sigOrigRange, 0xF);
			fclose(stdin);
			fclose(stdout);
			fclose(stderr);
			FreeConsole();

			FreeLibraryAndExitThread(hModule, 0);
			ExitThread(0);
		}

		Sleep(100);
	}
}

void __stdcall MainThread(HMODULE hModule) {
	DisableThreadLibraryCalls(hModule);

	std::cout << "=----------------------------------------=" << std::endl;
	std::cout << "= Stierlitz v6.0 - by Schwarz Kruppzo.   =" << std::endl;
	std::cout << "=----------------------------------------=" << std::endl;
	std::cout << "= [F9]        - unload Stierlitz.        =" << std::endl;
	std::cout << "=                                        =" << std::endl;
	std::cout << "= [Home]      - toggle Fog of War.       =" << std::endl;
	std::cout << "= [Insert]    - toggle Infinite Range.   =" << std::endl;
	std::cout << "= [Page Up]   - toggle Inf. Resources.   =" << std::endl;
	std::cout << "=                                        =" << std::endl;
	std::cout << "= [End]       - freeze collision scope.  =" << std::endl;
	std::cout << "= [*]         - unlock camera borders.   =" << std::endl;
	std::cout << "=                                        =" << std::endl;
	std::cout << "= [Page Down] - switch to next player.   =" << std::endl;
	std::cout << "= [Numpad 1]  - reset switch.            =" << std::endl;
	std::cout << "=                                        =" << std::endl;
	std::cout << "= [Delete]    - patch minimap.           =" << std::endl;
	std::cout << "= USE MINIMAP PATCH IN MAIN MENU!        =" << std::endl;
	std::cout << "=----------------------------------------=" << std::endl;
	
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)OnDetach, hModule, 0, NULL);

	dwBaseAddress = (DWORD)GetModuleHandle("mowas_2.exe");
	
	if (dwBaseAddress == NULL) {
		std::cout << ">Error: something went wrong (0x01)." << std::endl;
		return;
	}

	dwBaseScopeAddress = (dwBaseAddress + 0xBE6384);
	dwRangePointer = (dwBaseAddress + 0x44B5A7);
	dwFogPointer = *((DWORD*)(dwBaseAddress + 0xBE40AC));
	dwCameraPointer = (dwBaseAddress + 0xB3B538);
	dwJump1 = (dwBaseAddress + 0x44B5B2);
	dwJump2 = (dwBaseAddress + 0x44B7FB);
	dwScopeFunc = (dwBaseAddress + 0x44A818);
	dwJmpScopeFunc = (dwBaseAddress + 0x44A821);

	MakeJump((BYTE*)dwRangePointer, (DWORD)InfiniteRange, 0xC);
	MakeJump((BYTE*)dwScopeFunc, (DWORD)ForceCollisionScope, 0xA);

	CSigScan::GetDllMemInfo((BYTE*)dwBaseAddress);

	DWORD dwMinimap1;
	DWORD dwMinimap2;
	DWORD dwMinimap3;
	
	int n = sizeof(sigMinimap1) / sizeof(sigMinimap1[0]);
	std::vector<int> vSigMinimap1(sigMinimap1, sigMinimap1 + n);
	n = sizeof(sigMinimap2) / sizeof(sigMinimap2[0]);
	std::vector<int> vSigMinimap2(sigMinimap2, sigMinimap2 + n);
	n = sizeof(sigMinimap3) / sizeof(sigMinimap3[0]);
	std::vector<int> vSigMinimap3(sigMinimap3, sigMinimap3 + n);
	n = sizeof(sigMinimap3_Robz) / sizeof(sigMinimap3_Robz[0]);
	std::vector<int> vSigMinimap3_Robz(sigMinimap3_Robz, sigMinimap3_Robz + n);
	
	pTimer timer;
	timer.setLoop([]() {
		if (dwBaseAddress == NULL) {
			bIsInGame = false;
			return;
		}

		DWORD gameBase = *(DWORD*)(dwBaseAddress + 0xBED0C4);

		bIsInGame = (gameBase != NULL);

		if (bIsInGame) {
			if (activated.fog) {
				*((DWORD*)dwFogPointer) = 0;
			}

			if (activated.camera) {
				*((DWORD*)dwCameraPointer) = 0;
			}
		}
	}, 1000);
	timer.setLoop(ScopeFreezer, 10);
	
	std::cout << ">Stierlitz is ready!" << std::endl;

	while (true) {
		if (GetAsyncKeyState(VK_HOME) & 1) {
			std::cout << (activated.fog ? ">Fog of War on." : ">Fog of War off.") << std::endl;

			activated.fog = !activated.fog;

			if (bIsInGame && !activated.fog) {
				*((DWORD*)dwFogPointer) = 1;
			}
		} else if (GetAsyncKeyState(VK_MULTIPLY) & 1) {
			std::cout << (activated.camera ? ">Camera locked." : ">Camera unlocked.") << std::endl;

			activated.camera = !activated.camera;

			if (bIsInGame && !activated.camera) {
				*((DWORD*)dwCameraPointer) = 1;
			}
		} else if (GetAsyncKeyState(VK_INSERT) & 1) {
			std::cout << (activated.range ? ">Infinite Range off." : ">Infinite Range on.") << std::endl;

			activated.range = !activated.range;
		} else if (GetAsyncKeyState(VK_END) & 1) {
			if (!activated.scope) {
				std::cout << ">Scope frozen." << std::endl;

				DWORD addr1 = (*(DWORD*)dwBaseScopeAddress);
				if (addr1 != NULL) {
					if (*(DWORD*)addr1 != 0x0) {
						DWORD addr2 = addr1 + 0x240;

						if (addr2 != NULL) {
							if (*(DWORD*)addr2 != 0x0) {
								DWORD scope = (*(DWORD*)addr2) + 0xE8;

								if (scope != NULL) {
									DWORD x = (*(DWORD*)scope + 0x9C);
									if (x != NULL) {
										scopeX = *(DWORD*)x;
									}

									DWORD y = (*(DWORD*)scope + 0xA0);
									if (y != NULL) {
										scopeY = *(DWORD*)y;
									}

									DWORD z = (*(DWORD*)scope + 0xA4);
									if (z != NULL) {
										scopeZ = *(DWORD*)z;
									}
								}
							}
						}
					}
				}
			} else {
				std::cout << ">Scope unfrozen." << std::endl;

				scopeX = 0;
				scopeY = 0;
				scopeZ = 0;
			}

			activated.scope = !activated.scope;
		} else if (GetAsyncKeyState(VK_PRIOR) & 1) {
			if (!activated.resources) {
				std::cout << ">Infinite resources on." << std::endl;
			}
			else {
				std::cout << ">Infinite resources off." << std::endl;
			}

			activated.resources = !activated.resources;

			SwitchInfiniteResources(activated.resources);
		} else if (GetAsyncKeyState(VK_NEXT) & 1) {
			std::cout << ">Switched to " << SwitchCurrentPlayer(false) << " player." << std::endl;
		} else if (GetAsyncKeyState(VK_NUMPAD1) & 1) {
			std::cout << ">Switched to " << SwitchCurrentPlayer(true) << " player." << std::endl;
		} else if (GetAsyncKeyState(VK_DELETE) & 1) {
			if (!bIsInGame) {
				std::cout << ">Attempting to patch minimap, please wait..." << std::endl;

				dwMinimap1 = GetAddressFromSignature(vSigMinimap1, dwBaseAddress);
				dwMinimap3 = GetAddressFromSignature(vSigMinimap3, dwBaseAddress);
				DWORD dwMinimap3Robz = GetAddressFromSignature(vSigMinimap3_Robz, dwBaseAddress);

				if (dwMinimap1 && dwMinimap3) {
					PatchSig((BYTE*)dwMinimap1, (BYTE*)patchMinimap1, 0x39);
					PatchSig((BYTE*)dwMinimap3, (BYTE*)patchMinimap3, 0x1E);

					if (dwMinimap3Robz) {
						PatchSig((BYTE*)dwMinimap3Robz, (BYTE*)patchMinimap3, 0x1E);
					}

					DWORD dwLastMinimap2Address = dwBaseAddress;
					for (int i = 0; i < 3; i++) {
						DWORD dwMinimap2 = GetAddressFromSignature(vSigMinimap2, dwLastMinimap2Address);

						if (dwMinimap2) {
							dwLastMinimap2Address = dwMinimap2;
							PatchSig((BYTE*)dwMinimap2, (BYTE*)patchMinimap2, 0x11);
						}
						else {
							break;
						}
					}

					std::cout << ">Minimap patched successfully." << std::endl;
				}
				else {
					std::cout << ">Error: failed to patch minimap." << std::endl;
				}

				Sleep(1000);
			}
		}

		if (bStopThreads) {
			ExitThread(0);
		} else {
			Sleep(25);
		}
	}
}

bool __stdcall DllMain(HINSTANCE module, int reason, void*) {
	if (reason == DLL_PROCESS_ATTACH) {
		hModule = (HMODULE)module;
		bStopThreads = false;

		AllocConsole();
		SetConsoleTitle("Stierlitz Console");

		freopen("CONIN$", "r", stdin);
		freopen("CONOUT$", "w", stdout);
		freopen("CONOUT$", "w", stderr);

		hMainThread = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)MainThread, hModule, NULL, NULL);
	}

	return true;
}
