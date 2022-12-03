// header.h : include file for standard system include files,
// or project specific include files
//

#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#define _CRT_SECURE_NO_WARNINGS

// Windows Header Files
#include <windows.h>
#include <iostream>
#include <stdio.h>
#include <d3d11.h>
#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <algorithm>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"

#include "kiero.h"
#include "sigscan.h"
#include "Globals.h"