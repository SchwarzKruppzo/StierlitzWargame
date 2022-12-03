// dllmain.cpp : Defines the entry point for the DLL application.
#include "framework.h"
#include "Hook.hpp"
#include "Hack.h"

int MainThread()
{
	Hook::RenderBind();
	Hack::Init();

	return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
    {
		DisableThreadLibraryCalls(hModule);
        CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)MainThread, NULL, 0, NULL);
    }

    return TRUE;
}


