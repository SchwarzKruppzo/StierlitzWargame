#pragma once

class ScopePosition
{
public:
	char offset[0x9C];
	float x;
	float y;
	float z;
};

class Scope
{
public:
	char offset[0xE8];
	ScopePosition* pos;
};

class PlayerController
{
public:
	char offset[0x240];
	Scope* scope;
};