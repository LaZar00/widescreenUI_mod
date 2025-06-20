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
//  a1. Added checkbox DeltaXPos and CheckboxWidth values for wider texts
//  b.	'Hotkeys' word in Hotkeys Window is centered for 16:9 Widescreen resolution
//  c.  Numbers in Hotkeys Window are REMOVED (only present left numbers) and added in texture for WideScreen 16:9
//  d.  Horizontal Dividers in Hotkeys Window are REMOVED
//  e.  Dialog Box width fixed to be wider and show the lines INSIDE the box.
//  f.  Fix for "Dividers" \material\interface\charactermaintenance\cm_divider" to align with windows
//  g.  Fix for "Titles" of the windows (Options/Load/Save...) for some small resolutions
//  h.  Set size of row manually for loading/saving tables
//  i.  Fix for inventory buttons shadow outside boundaries
//	j.	Fix loading bar aspect ratio for some 16:9 and 16:10 widescreens

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

void SafeWriteFloat(UInt32 addr, float data) {

	DWORD oldProtect;

	VirtualProtect((void*)addr, 4, PAGE_EXECUTE_READWRITE, &oldProtect);

	*((FLOAT*)addr) = (FLOAT)(data);

	VirtualProtect((void*)addr, 4, oldProtect, &oldProtect);
}

void SafeWriteDouble(UInt64 addr, double data) {

	DWORD oldProtect;

	VirtualProtect((void*)addr, 8, PAGE_EXECUTE_READWRITE, &oldProtect);

	*((QDWORD*)addr) = (QDWORD)(data);

	VirtualProtect((void*)addr, 8, oldProtect, &oldProtect);
}

void SafeWriteDoubleWithOffsetChange(UInt64 addr_data, double data, UInt32 offset)
{
	SafeWriteDouble(addr_data, data);
	SafeWrite32(offset, addr_data);
}

void PatchMemoryNop(UInt32 Address, UInt32 Size)
{
	DWORD oldProtect = 0;
	VirtualProtect((LPVOID)Address, Size, PAGE_EXECUTE_READWRITE, &oldProtect);

	for (UInt32 i = 0; i < Size; i++)
		*(volatile BYTE*)(Address + i) = 0x90;

	VirtualProtect((LPVOID)Address, Size, oldProtect, &oldProtect);

	FlushInstructionCache(GetCurrentProcess(), (LPVOID)Address, Size);
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
	double dEmptyBarXPos, dEmptyBarWidth, dRedBarXPos, dRedBarWidth;

	UInt32 addr;

	unsigned char disableNums[1] = { 0x0 };
	//unsigned char disableHorizDivisors[5] = { 0x90, 0x90, 0x90, 0x90, 0x90 };

		
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

		// This info is for the Loading Bar.
		//// 1024 texture (original)
		////					EmptyBarXPos		EmptyBarWidth		RedBarXPos		RedBarWidth
		//// 1024				152.0				869.0				190.0			830.0
		//// 1280x720			208.0				832.0				234.0			798.0
		//// 1280x800			200.0				832.0				234.0			798.0
		//// 1366				216.0 (0,155)		808.0				247.0			776.0		
		//// 1600x1024			248.0				790.0				277.0			761.0
		//// 1680				248.0				776.0				277.0			748.0
		//// 1920				288.0				740.0				312.0			717.0
		if (iARFactor > -1)
		{
			yFactor = (float)0.69;
			dEmptyBarXPos = -1.0;
			dEmptyBarWidth = -1.0;
			dRedBarXPos = -1.0;
			dRedBarWidth = -1.0;

			switch (iARFactor)
			{
			case 0:
				xFactor = (float)0.80;
				iHotkeysWordXPos = 0x19F;
				break;

			case 1:
				xFactor = (float)0.82;
				iHotkeysWordXPos = 0xC0;
				break;

			case 2:
				xFactor = (float)0.81;
				iHotkeysWordXPos = 0x108;
				break;

			case 3:
				xFactor = (float)0.81;
				iHotkeysWordXPos = 0x1D4;
				break;

			case 4:
				xFactor = (float)0.81;
				iHotkeysWordXPos = 0x138;
				break;

			case 5:
				xFactor = (float)0.81;
				iHotkeysWordXPos = 0x11F;
				break;

			default:
				break;
			}

			// j. Loading Bar coef assignment
			switch (screen_width)
			{
			case 1024:
				break;

			case 1280:
				dEmptyBarXPos = 208.0;
				dEmptyBarWidth = 832.0;
				dRedBarXPos = 242.0;
				dRedBarWidth = 798.0;
				break;

			case 1360:
			case 1366:
				dEmptyBarXPos = 216.0;
				dEmptyBarWidth = 808.0;
				dRedBarXPos = 247.0;
				dRedBarWidth = 776.0;
				break;

			case 1440:
				dEmptyBarXPos = 235.0;
				dEmptyBarWidth = 787.0;
				dRedBarXPos = 265.0;
				dRedBarWidth = 757.0;
				break;
					
			case 1600:
			case 1680:
				dEmptyBarXPos = 248.0;
				dEmptyBarWidth = 776.0;
				dRedBarXPos = 277.0;
				dRedBarWidth = 748.0;
				break;

			case 1920:
				dEmptyBarXPos = 280.0;
				dEmptyBarWidth = 740.0;
				dRedBarXPos = 305.0;
				dRedBarWidth = 715;
				break;
			}


			//  a.  Skip Intro checkbox in Character Creation is positioned correctly for 16:9 Widescreen resolution
			if (GetPrivateProfileIntA("SkipIntroFix", "enabled", 0, ".\\Bin\\loader\\widescreenUI_mod.ini"))
			{
				WORD value;

				addr = (UInt32)client + 0x17F3ED;
				SafeWrite32(addr, (UInt32)(screen_height * yFactor));

				//  a1. Added checkbox DeltaXPos and CheckboxWidth values for wider texts
				addr = (UInt32)client + 0x17F3F2;
				value = GetPrivateProfileIntA("SkipIntroFix", "DeltaXPos", 0, ".\\Bin\\loader\\widescreenUI_mod.ini");
				SafeWrite32(addr, (UInt32)((screen_width * xFactor)) + value);

				addr = (UInt32)client + 0x17F3E8;
				value = GetPrivateProfileIntA("SkipIntroFix", "CheckboxWidth", 0xD0, ".\\Bin\\loader\\widescreenUI_mod.ini");
				SafeWrite32(addr, (UInt32)(value));
			}

			//	b.	'Hotkeys' word in Hotkeys Window is centered for Widescreen resolutions
			if (GetPrivateProfileIntA("HotkeysWordCentered", "enabled", 0, ".\\Bin\\loader\\widescreenUI_mod.ini"))
			{
				addr = (UInt32)client + 0x182778;
				SafeWrite32(addr, iHotkeysWordXPos);
			}
			
			//	j.	Fix loading bar aspect ratio for some 16:9 and 16:10 widescreens
			if (GetPrivateProfileIntA("LoadingBarFix", "enabled", 0, ".\\Bin\\loader\\widescreenUI_mod.ini"))
			{
				if (dEmptyBarWidth != -1) 
				{
					SafeWriteDouble((UInt64)client + 0x249870, dEmptyBarWidth); // image loading bar horizontal scaling
					SafeWriteDouble((UInt64)client + 0x249880, dEmptyBarXPos); // image loading bar x position

					SafeWriteDouble((UInt64)client + 0x249890, dRedBarWidth); // image loading red bar horizontal scaling
					SafeWriteDouble((UInt64)client + 0x2498A0, dRedBarXPos); // image loading red bar x position
				}
			}
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
		PatchMemoryNop((UInt32)client + 0x18335B, 5);
		//addr = (UInt32)client + 0x18335B;
		//SafeWriteBuf(addr, disableHorizDivisors, 5);

		//  e.  Dialog Box width fixed to be wider and show the lines INSIDE the box.
		QDWORD value;

		if (GetPrivateProfileIntA("DialogBoxWidthFix", "enabled", 0, ".\\Bin\\loader\\widescreenUI_mod.ini"))
		{
			addr = (UInt64)client + 0x228FA8;
			value = GetPrivateProfileIntA("DialogBoxWidthFix", "width", 832, ".\\Bin\\loader\\widescreenUI_mod.ini");

			SafeWriteDouble(addr, value);
		}

		//  f.  Fix for "Dividers" \material\interface\charactermaintenance\cm_divider" to adjust to the windows
		//  These are for CharacterEditor "Dividers"
		if (GetPrivateProfileIntA("DividersFix", "enabled", 0, ".\\Bin\\loader\\widescreenUI_mod.ini"))
		{
			// Experience Divider (centered in the word Experience in Character Editor menu)
			SafeWriteDoubleWithOffsetChange((UInt64)client + 0x1E2FF0, 0.0009475, (UInt32)client + 0x18C6C2);	// Load

			// Main Dividers (Skills, Abilities...) (centered the best possible)
			addr = (UInt32)client + 0x17FAC4;
			unsigned char maindividers[1] = { 0xE4 };
			SafeWriteBuf(addr, maindividers, 1);

			// Information Divider (centered the best possible)
			unsigned char info_and_activequests[1] = { 0x3A };
			addr = (UInt32)client + 0x17BB68;
			SafeWriteBuf(addr, info_and_activequests, 1);

			// Active Quests Divider (centered for the infowindow)
			addr = (UInt32)client + 0x188901;
			SafeWriteBuf(addr, info_and_activequests, 1);
		}

		//  i.  Fix for inventory buttons shadow outside boundaries
		if (GetPrivateProfileIntA("InventoryButtonsShadowFix", "enabled", 0, ".\\Bin\\loader\\widescreenUI_mod.ini"))
		{
			// For this fix you will need the modified buttons.tth and buttons.ttz files in
			// "\materials\interface\charactermaintenance" folder.
			// y position coordinate and height of the shadow for Action & Drop buttons
			// (most left and middle buttons respectively)
			double y_position = 686.0;
			addr = (UInt32)client + 0x24A908;
			SafeWriteDouble((UInt64)addr, y_position);

			double y_height = 54.0;
			addr = (UInt32)client + 0x24A918;
			SafeWriteDouble((UInt64)addr, y_height);


			// x width of the shadow for Drop button (middle button)
			//double x_position = 450.0;
			//addr = (UInt32)client + 0x24A8F0;
			//SafeWriteDouble((UInt64)addr, x_position);

			double x_width = 132.0;
			addr = (UInt32)client + 0x24A8F8;
			SafeWriteDouble((UInt64)addr, x_width);

			// x width of the shadow for Action button (most left button)
			// We will reuse the previous value.
			addr = (UInt32)client + 0x16B9BD;
			SafeWrite32(addr, (UInt32)client + 0x24A8F8);
		}


		// For tracing if we access .ini file
		//char sample[10];
		//FILE* fout = fopen(".\\Bin\\loader\\test.txt", "wt");
		//sprintf(sample, "%d", GetPrivateProfileIntA("SkipIntro", "ypos", 0x20E, ".\\Bin\\loader\\widescreenUI_mod.ini"));
		//fputs(sample, fout);
		//fclose(fout);
	}


	//  These are for Load/Save/Delete Save/Confirm/EntryText/Notify windows
	HMODULE GameUI = GetModuleHandleA("GameUI.dll");
	if (GameUI != NULL)
	{
		//  f.  Fix for "Dividers" \material\interface\charactermaintenance\cm_divider" to adjust to the windows
		if (GetPrivateProfileIntA("DividersFix", "enabled", 0, ".\\Bin\\loader\\widescreenUI_mod.ini"))
		{
			// Load & Save Game Divider (centered in the screen)
			SafeWriteDoubleWithOffsetChange((UInt64)GameUI + 0x4E760, 144.0, (UInt32)GameUI + 0x17650); // Save
			SafeWriteDoubleWithOffsetChange((UInt64)GameUI + 0x4E760, 144.0, (UInt32)GameUI + 0x13D10);	// Load

			// EntryText_UserName Divider (made thinner)
			SafeWriteDoubleWithOffsetChange((UInt64)GameUI + 0x4E768, 490.0, (UInt32)GameUI + 0xD390);	// Load

		}

		//  g. Fix for "Titles" of the windows (Options/Load/Save...) for some small resolutions
		if (GetPrivateProfileIntA("TitlesFix", "enabled", 0, ".\\Bin\\loader\\widescreenUI_mod.ini"))
		{
			// Options Title (Tabs and Panel with lowered Ypos)
			SafeWriteDouble((UInt64)GameUI + 0x4F598, 40.0);

			// Load & Save Game Titles (centered in the screen)
			SafeWriteDoubleWithOffsetChange((UInt64)GameUI + 0x4E750, 0.0015461444854736328, (UInt32)GameUI + 0x1762B); // Save
			SafeWriteDoubleWithOffsetChange((UInt64)GameUI + 0x4E750, 0.0015461444854736328, (UInt32)GameUI + 0x13CEB);	// Load

			// EntryText_UserName Title
			SafeWriteDoubleWithOffsetChange((UInt64)GameUI + 0x4E750, 0.0015461444854736328, (UInt32)GameUI + 0xD3A9);

			// Notification Title
			SafeWriteDoubleWithOffsetChange((UInt64)GameUI + 0x4E750, 0.0015461444854736328, (UInt32)GameUI + 0x9E7E);

			// Confirmation Title
			SafeWriteDoubleWithOffsetChange((UInt64)GameUI + 0x4E750, 0.0015461444854736328, (UInt32)GameUI + 0x2055);
		}

		// h. Set size of row manually for loading/saving tables
		if (GetPrivateProfileIntA("SizeofRowTables", "enabled", 0, ".\\Bin\\loader\\widescreenUI_mod.ini"))
		{
			int size;
			int screenheight;

			if (size = GetPrivateProfileIntA("SizeofRowTables", "sizeHeight", 14, ".\\Bin\\loader\\widescreenUI_mod.ini"))
			{
				addr = ((UInt32)GameUI + 0x2BE38);
				SafeWrite32(addr, size);
			}
		}
	}
}
