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
//  e.  Dialog Box width fixed to be wider and show the lines INSIDE the box.

// I will adjust some of the features depending on the aspect ratio (excep 4:3, that is the default values)
//Common resolutions in ratios :
//5 : 4
//320x256
//600x480
//640x512
//1280x1024
//1800x1440
//
//4:3 standard TV / monitor
//320x240
//400x300
//640x480
//800x600
//960x720
//1024x768
//1280x960
//1440x1080
//1600x1200
//1920x1440
//
//16:9 standard widescreen
//640x360
//960x540
//1024x576
//1280x720 aka HD 720p
//1600x900
//1920x1080 aka HD 1080p
//2560×1440
//3840×2160
//
//16:10 or 8:5
//320x200
//640x400
//768x480
//1024x640
//1152x720
//1280x800
//1440x900
//1680x1050
//1920x1200


#include "pch.h"
#include <windows.h>
#include <winbase.h>
#include <winreg.h>
#include <iomanip>
//#include <stdio.h>			// For tracing if we access .ini file

typedef unsigned char			UInt8;
typedef short unsigned int		UInt16;
typedef unsigned int			UInt32;
typedef unsigned long			UInt64;
typedef double					QDWORD;


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

void SafeWriteDouble(UInt64 addr, double data) {

	DWORD oldProtect;

	VirtualProtect((void*)addr, 8, PAGE_EXECUTE_READWRITE, &oldProtect);

	*((QDWORD*)addr) = (QDWORD)(data);

	VirtualProtect((void*)addr, 8, oldProtect, &oldProtect);
}

// We will get the AR we are using. We assume 4:3 is the default of the game.
// Common resolutions:
// 0 = 4:3
// 1 = 16:9		(1.77)
// 2 = 16:10	(1.6)
// 3 = 5:4		(1.25)
// 4 = 3:2		(1.5)
// 5 = 25:16	(1.5625)
int GetARFactor(int screenwidth, int screenheight)
{
	int returnAR = -1;
	float ratioAR = ((float)(screenwidth * 100) / screenheight);
	ratioAR = trunc(ratioAR) / 100;

	if (ratioAR >= 1.32 and ratioAR <= 1.34)			//  4:3
	{
		returnAR = 0;
	}
	else if (ratioAR >= 1.76 and ratioAR <= 1.78)		//	16:9
	{
		returnAR = 1;
	}
	else if (ratioAR >= 1.59 and ratioAR <= 1.61)		//	16:10
	{
		returnAR = 2;
	}
	else if (ratioAR >= 1.24 and ratioAR <= 1.26)		//	5:4
	{
		returnAR = 3;
	}
	else if (ratioAR >= 1.49 and ratioAR <= 1.51)		//	3:2
	{
		returnAR = 4;
	}
	else if (ratioAR >= 1.55 and ratioAR <= 1.57)		//	25:16
	{
		returnAR = 5;
	}

	return returnAR;
}

extern "C" __declspec(dllexport) void loaded_client()
{
	DWORD screen_width = 0;
	DWORD screen_height = 0;
	DWORD dataSizeW = sizeof(screen_width);
	DWORD dataSizeH = sizeof(screen_height);
	float xFactor = 0, yFactor = 0;
	int iHotkeysWordXPos = 0, iARFactor = 0;

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

		// Get the Aspect Ratio factor
		iARFactor = GetARFactor(screen_width, screen_height);

		//  a.  Skip Intro checkbox in Character Creation is positioned correctly for Widescreen resolutions
		//      Resolution		xpos 					ypos 
		//  0   800x600			0x320					0x20F	527	(0,68)
		//  1   1366x768		0x0460	1120 (0,82)		0x20F	527	(0,68)
		//      1920x1080		0x0640	1600 (0,83)		0x2E4	740	(0,68)
		//  2	1680x1050		0x0640	1560 (0,81)		0x2E4	740	(0,68)
		//  3	1280x1024					 (0,81)					(0,68)
		//  4	720x480						 
		// If we have some Aspect Ratio, we push the values.
		if (iARFactor > -1)
		{
			switch (iARFactor)
			{
			case 0:
				yFactor = (float)0.68;
				xFactor = (float)0.80;
				iHotkeysWordXPos = 0x19F;
				break;

			case 1:
				yFactor = (float)0.68;
				xFactor = (float)0.82;
				iHotkeysWordXPos = 0xC0;
				break;

			case 2:
				yFactor = (float)0.68;
				xFactor = (float)0.81;
				iHotkeysWordXPos = 0x108;
				break;

			case 3:
				yFactor = (float)0.68;
				xFactor = (float)0.81;
				iHotkeysWordXPos = 0x1D4;
				break;

			case 4:
				yFactor = (float)0.68;
				xFactor = (float)0.81;
				iHotkeysWordXPos = 0x138;
				break;

			case 5:
				yFactor = (float)0.68;
				xFactor = (float)0.81;
				iHotkeysWordXPos = 0x11F;
				break;

			default:
				break;
			}

			addr = (UInt32)client + 0x17F3ED;
			SafeWrite32(addr, (UInt32)(screen_height * yFactor));
			//SafeWrite32(addr, GetPrivateProfileIntA("SkipIntro", "ypos", 0x20E, ".\\Bin\\loader\\widescreenUI_mod.ini"));

			addr = (UInt32)client + 0x17F3F2;
			SafeWrite32(addr, (UInt32)(screen_width * xFactor));
			//SafeWrite32(addr, GetPrivateProfileIntA("SkipIntro", "xpos", 0x320, ".\\Bin\\loader\\widescreenUI_mod.ini"));


			//	b.	'Hotkeys' word in Hotkeys Window is centered for Widescreen resolutions
			addr = (UInt32)client + 0x182778;
			SafeWrite32(addr, iHotkeysWordXPos);
		}


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

		//  e.  Dialog Box width fixed to be wider and show the lines INSIDE the box.
		QDWORD value;

		if (GetPrivateProfileIntA("DialogBoxWidthFix", "enabled", 0, ".\\Bin\\loader\\widescreenUI_mod.ini"))
		{
			addr = (UInt64)client + 0x228FA8;
			value = GetPrivateProfileIntA("DialogBoxWidthFix", "width", 832, ".\\Bin\\loader\\widescreenUI_mod.ini");

			SafeWriteDouble(addr, value);
		}


		// For tracing if we access .ini file
		//char sample[10];
		//FILE* fout = fopen(".\\Bin\\loader\\test.txt", "wt");
		//sprintf(sample, "%d", GetPrivateProfileIntA("SkipIntro", "ypos", 0x20E, ".\\Bin\\loader\\widescreenUI_mod.ini"));
		//fputs(sample, fout);
		//fclose(fout);
	}
}
