#pragma once

#include "SDK/Scope.h"
#include "SDK/Game.h"

namespace Hack
{
	extern bool bIsInGame;

	extern DWORD baseAddress;
	extern DWORD gameBaseAddress;
	extern DWORD baseScopeAddress;

	extern DWORD fogPointer;
	extern DWORD cameraPointer;
	extern DWORD rangePointer;
	extern DWORD scopePointer;

	extern DWORD jmpInfRange1;
	extern DWORD jmpInfRange2;
	extern DWORD jmpScopeFunc;

	struct Vec3 {
		float x;
		float y;
		float z;
	};

	struct PlayerResources {
		float mp;
		float sp;
		float cp;
		float cp_max;
	};

	struct ListPlayer {
		unsigned char id;
		const char* army;
		const char* name;
		float mp;
		float sp;
		float cp;
		float cp_max;
	};

	extern Vec3	lockedScopePosition;

	extern std::multimap<bool, ListPlayer>	PlayerCache;

	extern int				max_players;
	extern DWORD			playerBasePoints;
	extern DWORD			playerBase;
	extern PlayerTeamInfo*	teams;
	extern bool				minimap_was_patched;

	int	GetMaxSlots();
	int	GetActivePlayer();

	void				ClampPlayerSlot(unsigned char& playerID);
	PlayerArmy*			GetPlayerInfo(unsigned char playerID);
	PlayerResources		GetPlayerResources(unsigned char playerID);

	bool IsValidPlayer(unsigned char playerID);
	void SwitchToPlayer(unsigned char playerID);
	int	 SwitchCurrentPlayer(bool reset);
	void SwitchInfiniteResources(bool activated);
	void UpdatePlayerList();

	void SaveScope();
	void FreezeScope();
	void PatchMinimap();

	void Init();
}