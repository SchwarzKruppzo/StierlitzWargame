#pragma once

class PlayerArmy
{
public:
	int teamid;
	int unknown;
	char name[16];
	unsigned int namesize;
	char offset1[8];
	char team[2];
	char offset2[134];
	char army[5];
};

struct PlayerPoints
{
	char offset1[0x2D8];
	float manpower;
	char offset2[0x37C];
	float special;
};

class PlayerTeamInfo
{
public:
	PlayerArmy* players[16];
};
