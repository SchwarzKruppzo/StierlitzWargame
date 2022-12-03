#include "framework.h"
#include "Memory.hpp"
#include "pTimer.hpp"
#include "Hack.h"

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

__declspec(naked) void InfiniteRange()
{
	static BYTE flags;

	__asm
	{
		lahf
		mov[flags], ah
	}

	if (!Settings::NoRange)
	{
		__asm
		{
			mov ah, [flags]
			sahf
			jbe $ + 15
			mov[edi], 0x00000000
			jmp[Hack::jmpInfRange1]
			jmp[Hack::jmpInfRange2]
		}
	}
	else
	{
		__asm
		{
			jmp[Hack::jmpInfRange2]
		}
	}
}

__declspec(naked) void ShowScopeCollisionIndicator()
{
	if (!Settings::FreezeScope)
	{
		__asm
		{
			mov[edi + 0xCC], 0
		}
	}
	else
	{
		__asm
		{
			mov[edi + 0xCC], 0x4
		}
	}

	__asm
	{
		jmp[Hack::jmpScopeFunc]
	}
}

namespace Hack
{
	bool bIsInGame = false;

	DWORD baseAddress;
	DWORD gameBaseAddress;
	DWORD baseScopeAddress;

	DWORD fogPointer;
	DWORD cameraPointer;
	DWORD rangePointer;
	DWORD scopePointer;

	DWORD jmpInfRange1;
	DWORD jmpInfRange2;
	DWORD jmpScopeFunc;

	Vec3 lockedScopePosition;
	std::multimap<bool, ListPlayer> PlayerCache{};

	bool			minimap_was_patched;
	int				max_players;
	DWORD			playerBasePoints;
	DWORD			playerBase;
	PlayerTeamInfo* teams;

	int GetMaxSlots()
	{
		if (!bIsInGame)
			return max_players;

		max_players = *reinterpret_cast<unsigned char*>(baseAddress + 0xB39E68);
		max_players--;

		if (max_players < 0)
			max_players = 0;

		return max_players;
	}

	int GetActivePlayer()
	{
		if (!bIsInGame)
			return 0;

		int result = *reinterpret_cast<unsigned char*>(baseAddress + 0xB39E64);

		return (result - 1);
	}

	void ClampPlayerSlot(unsigned char &playerID)
	{
		if (playerID > GetMaxSlots())
			playerID = max_players;

		if (playerID < 0)
			playerID = 0;
	}

	PlayerArmy* GetPlayerInfo(unsigned char playerID)
	{
		teams = *(PlayerTeamInfo**)(gameBaseAddress + 0x68);

		if (teams == NULL)
			return nullptr;

		ClampPlayerSlot(playerID);

		return teams->players[playerID];
	}

	PlayerResources GetPlayerResources(unsigned char playerID)
	{
		PlayerResources data;
		DWORD playerOffset = (playerID * 0xB0);
		DWORD playerBasePtr = *reinterpret_cast<DWORD*>(gameBaseAddress + 0xC4);

		if (playerBasePtr == NULL)
			return data;

		playerBase = *reinterpret_cast<DWORD*>(playerBasePtr);
		playerBasePoints = *reinterpret_cast<DWORD*>(playerBase + 0x8 + playerOffset);

		ClampPlayerSlot(playerID);

		data.cp = *reinterpret_cast<float*>(playerBase + playerOffset + 0x70);
		data.mp = *reinterpret_cast<float*>(playerBasePoints + 0x2D8);
		data.sp = *reinterpret_cast<float*>(playerBasePoints + 0x658);
		data.cp_max = *reinterpret_cast<float*>(playerBase + playerOffset + 0x74);

		return data;
	}

	bool IsValidPlayer(unsigned char playerID)
	{
		if (!bIsInGame)
			return false;

		if (playerID > GetMaxSlots())
			return false;

		DWORD playerBasePtr = *reinterpret_cast<DWORD*>(gameBaseAddress + 0xC4);

		if (playerBasePtr == NULL)
			return false;

		playerBase = *reinterpret_cast<DWORD*>(playerBasePtr);

		if (playerBase == NULL)
			return false;

		unsigned char id = *reinterpret_cast<unsigned char*>(playerBase + (playerID * 0xB0));

		return id != 0;
	}

	void SwitchToPlayer(unsigned char playerID)
	{
		if (playerID > GetMaxSlots())
			return;

		*reinterpret_cast<unsigned char*>(baseAddress + 0xB39E64) = (playerID + 1);
	}

	void SwitchInfiniteResources(bool activated)
	{
		if (baseAddress == NULL)
			return;

		if (activated) {
			for (int i = 0; i < 5; i++) {
				DWORD dwAddr = (baseAddress + offsetsInf[i]);

				if (i > 2) {
					Memory::MakePatch((BYTE*)dwAddr, (BYTE*)sigInfPatch2[i % 3], 0x03);
				}
				else {
					Memory::MakePatch((BYTE*)dwAddr, (BYTE*)sigInfPatch1[i], 0x02);
				}
			}
		}
		else {
			for (int i = 0; i < 5; i++) {
				DWORD dwAddr = (baseAddress + offsetsInf[i]);

				if (i > 2) {
					Memory::MakePatch((BYTE*)dwAddr, (BYTE*)sigInfPatch2[2 + (i % 3)], 0x03);
				}
				else {
					Memory::MakePatch((BYTE*)dwAddr, (BYTE*)sigInfPatch1[3 + (i % 3)], 0x02);
				}
			}
		}
	}

	void UpdatePlayerList()
	{
		std::multimap<bool, ListPlayer> listBuffer;

		if (bIsInGame)
		{
			for (int i = 0; i < GetMaxSlots(); i++)
			{
				if (!IsValidPlayer(i))
					continue;

				PlayerArmy* info = GetPlayerInfo(i);

				if (info == NULL)
					continue;

				PlayerResources data = GetPlayerResources(i);

				const char* name = info->name;

				if (info->namesize > 0x10u)
					name = *(const char**)info->name;

				ListPlayer player{
					i,
					info->army,
					name,
					data.mp,
					data.sp,
					data.cp,
					data.cp_max,
				};

				listBuffer.emplace(strcmp(info->team, "a") == 0, player);
			}
		}

		listBuffer.swap(PlayerCache);
	}

	void SaveScope()
	{
		PlayerController* playerController = *(PlayerController**)baseScopeAddress;

		if (playerController == NULL || playerController->scope == NULL)
			return;

		lockedScopePosition.x = playerController->scope->pos->x;
		lockedScopePosition.y = playerController->scope->pos->y;
		lockedScopePosition.z = playerController->scope->pos->z;
	}

	void FreezeScope() 
	{
		if (Settings::FreezeScope) {
			PlayerController* playerController = *(PlayerController**)baseScopeAddress;

			if (playerController == NULL || playerController->scope == NULL)
				return;

			playerController->scope->pos->x = lockedScopePosition.x;
			playerController->scope->pos->y = lockedScopePosition.y;
			playerController->scope->pos->z = lockedScopePosition.z;
		}
	}

	void PatchMinimap()
	{
		if (bIsInGame)
			return;

		if (minimap_was_patched)
			return;

		CSigScan::GetDllMemInfo((BYTE*)baseAddress);

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

		dwMinimap1 = Memory::GetAddressBySignature(vSigMinimap1, baseAddress);
		dwMinimap3 = Memory::GetAddressBySignature(vSigMinimap3, baseAddress);
		DWORD dwMinimap3Robz = Memory::GetAddressBySignature(vSigMinimap3_Robz, baseAddress);

		if (dwMinimap1 && dwMinimap3) {
			Memory::MakePatch((BYTE*)dwMinimap1, (BYTE*)patchMinimap1, 0x39);
			Memory::MakePatch((BYTE*)dwMinimap3, (BYTE*)patchMinimap3, 0x1E);

			if (dwMinimap3Robz)
				Memory::MakePatch((BYTE*)dwMinimap3Robz, (BYTE*)patchMinimap3, 0x1E);

			DWORD dwLastMinimap2Address = baseAddress;
			for (int i = 0; i < 3; i++) {
				DWORD dwMinimap2 = Memory::GetAddressBySignature(vSigMinimap2, dwLastMinimap2Address);

				if (dwMinimap2) {
					dwLastMinimap2Address = dwMinimap2;
					Memory::MakePatch((BYTE*)dwMinimap2, (BYTE*)patchMinimap2, 0x11);
				}
				else 
				{
					break;
				}
			}

			minimap_was_patched = true;
		}
		else
		{
			minimap_was_patched = false;
		}
	}

	void Init()
	{
		baseAddress = (DWORD)GetModuleHandle(L"mowas_2.exe");

		if (baseAddress == NULL)
			return;

		baseScopeAddress = (baseAddress + 0xBE6384);
		rangePointer = (baseAddress + 0x44B5A7);
		fogPointer = *((DWORD*)(baseAddress + 0xBE40AC));
		cameraPointer = (baseAddress + 0xB3B538);
		scopePointer = (baseAddress + 0x44A818);

		jmpInfRange1 = (baseAddress + 0x44B5B2);
		jmpInfRange2 = (baseAddress + 0x44B7FB);
		jmpScopeFunc = (baseAddress + 0x44A821);

		Memory::MakeJump((BYTE*)rangePointer, (DWORD)InfiniteRange, 0xC);
		Memory::MakeJump((BYTE*)scopePointer, (DWORD)ShowScopeCollisionIndicator, 0xA);

		pTimer timer;

		timer.setLoop([]() {
			UpdatePlayerList();
		}, 1000);

		timer.setLoop([]() {
			if (baseAddress == NULL) {
				bIsInGame = false;
				return;
			}

			gameBaseAddress = *(DWORD*)(baseAddress + 0xBED0C4);
			bIsInGame = (gameBaseAddress != NULL);

			if (bIsInGame)
			{
				*((DWORD*)fogPointer) = Settings::NoFog ? 0 : 1;
				*((DWORD*)cameraPointer) = Settings::FreeCamera ? 0 : 1;

				FreezeScope();
			}
		}, 1);
	}
}