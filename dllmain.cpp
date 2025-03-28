// dllmain.cpp : Defines entry point of DLL application.
// This module is for the game Vampire: The Masquerade - Bloodlines
// 
// Author: L@Zar0
// Date: 27/03/2025
// Version: 1.0 based on UP 11.5 (rc14.1)
// Contact: formerly ClanDLAN.net member, now in Discord.

// INFORMATION OF THE LIBRARY
// This mod changes some visual aspects when using 16:9 Widescreen resolution
// Some of this changes are:
//  a.  Skip Intro checkbox in Character Creation is positioned correctly for 16:9 Widescreen resolution
//  b.	'Hotkeys' word in Hotkeys Window is centered for 16:9 Widescreen resolution
//  c.  Numbers in Hotkeys Window are REMOVED (only present left numbers) and added in texture for WideScreen 16:9
//  d.  Horizontal Dividers in Hotkeys Window are REMOVED


#include "pch.h"
#include <windows.h>
#include <winbase.h>
#include <winreg.h>
//#include <stdio.h>			// For tracing if we access .ini file

typedef unsigned char			UInt8;
typedef short unsigned int		UInt16;
typedef unsigned int			UInt32;


BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	return TRUE;
}

void SafeWriteBuf(UInt32 addr, void* data, UInt32 len)
{
	DWORD	oldProtect;

	VirtualProtect((void*)addr, len, PAGE_EXECUTE_READWRITE, &oldProtect);
	memcpy((void*)addr, data, len);
	VirtualProtect((void*)addr, len, oldProtect, &oldProtect);
}

void SafeWrite32(UInt32 addr, UInt32 data)
{
	DWORD oldProtect;

	VirtualProtect((void*)addr, 4, PAGE_EXECUTE_READWRITE, &oldProtect);
	*((UInt32*)addr) = data;
	VirtualProtect((void*)addr, 4, oldProtect, &oldProtect);
}

extern "C" __declspec(dllexport) void loaded_client()
{
	DWORD screen_width = 0;
	DWORD screen_height = 0;
	DWORD dataSizeW = sizeof(screen_width);
	DWORD dataSizeH = sizeof(screen_height);

	UInt32 addr;

	unsigned char disableNums[1] = { 0x0 };
	unsigned char disableHorizDivisors[5] = { 0x90, 0x90, 0x90, 0x90, 0x90 };

		
	HMODULE client = GetModuleHandleA("client.dll");
	if (client != NULL)
	{
		// Let's get the width and height set in settings.
		RegGetValueA(HKEY_CURRENT_USER,
			"Software\\Troika\\Vampire\\ResPatch",
			"ScreenWidth",
			RRF_RT_DWORD,
			nullptr,
			&screen_width,
			&dataSizeW);

		RegGetValueA(HKEY_CURRENT_USER,
			"Software\\Troika\\Vampire\\ResPatch",
			"ScreenHeight",
			RRF_RT_DWORD,
			nullptr,
			&screen_height,
			&dataSizeH);


		//  a.  Skip Intro checkbox in Character Creation is positioned correctly for 16:9 Widescreen resolution
		//		Resolution		xpos: (0,83)		ypos: (0,68)
		//		1280×720		
		//		1366x768		0x0460	1120		0x20F	527
		//		1600×900
		//		1920x1080		0x0640	1600		0x2E4	740
		//		2560×1440
		//		3840×2160
		addr = (UInt32)client + 0x17F3ED;
		SafeWrite32(addr, screen_height * 0.68);
		//SafeWrite32(addr, GetPrivateProfileIntA("SkipIntro", "ypos", 0x20E, ".\\Bin\\loader\\widescreenUI_mod.ini"));

		addr = (UInt32)client + 0x17F3F2;
		SafeWrite32(addr, screen_width * 0.83);
		//SafeWrite32(addr, GetPrivateProfileIntA("SkipIntro", "xpos", 0x320, ".\\Bin\\loader\\widescreenUI_mod.ini"));


		//	b.	'Hotkeys' word in Hotkeys Window is centered for 16:9 Widescreen resolution
		addr = (UInt32)client + 0x182778;
		SafeWrite32(addr, 0xC0);


		//  c.  Numbers in Hotkeys Window are REMOVED
		//      When in Widescreen 16:9 there are only present the left numbers.
		//		We need an alternate solution, like apply a texture that includes numbers.
		//		I have not found a method to adjust correctly the numbers in 16:9
		if (GetPrivateProfileIntA("DisableNumsInHotkeysWindow", "enabled", 0, ".\\Bin\\loader\\widescreenUI_mod.ini")) 
		{
			addr = (UInt32)client + 0x1832EC;
			SafeWriteBuf(addr, disableNums, 1);
		}
		

		//  d.  Horizontal Dividers in Hotkeys Window are REMOVED
		addr = (UInt32)client + 0x18335B;
		SafeWriteBuf(addr, disableHorizDivisors, 5);


		// For tracing if we access .ini file
		//char sample[10];
		//FILE* fout = fopen(".\\Bin\\loader\\test.txt", "wt");
		//sprintf(sample, "%d", GetPrivateProfileIntA("SkipIntro", "ypos", 0x20E, ".\\Bin\\loader\\widescreenUI_mod.ini"));
		//fputs(sample, fout);
		//fclose(fout);
	}
}
