#include "..\includes\stdafx.h"
#include "..\includes\hooking\Hooking.Patterns.h"

HWND hWnd;
bool bDelay;

struct Screen
{
	int Width;
	int Height;
	float fWidth;
	float fHeight;
	float fAspectRatio;
} Screen;

DWORD WINAPI Init(LPVOID)
{
	auto pattern = hook::pattern("83 EC 58 53 56 57 89 65 E8");
	if (!(pattern.size() > 0) && !bDelay)
	{
		bDelay = true;
		CreateThread(0, 0, (LPTHREAD_START_ROUTINE)&Init, NULL, 0, NULL);
		return 0;
	}

	if (bDelay)
	{
		while (!(pattern.size() > 0))
			pattern = hook::pattern("83 EC 58 53 56 57 89 65 E8");
	}

	CIniReader iniReader("");
	Screen.Width = iniReader.ReadInteger("MAIN", "ResX", 0);
	Screen.Height = iniReader.ReadInteger("MAIN", "ResY", 0);

	if (!Screen.Width || !Screen.Height) {
		HMONITOR monitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
		MONITORINFO info;
		info.cbSize = sizeof(MONITORINFO);
		GetMonitorInfo(monitor, &info);
		Screen.Width = info.rcMonitor.right - info.rcMonitor.left;
		Screen.Height = info.rcMonitor.bottom - info.rcMonitor.top;
	}

	Screen.fWidth = static_cast<float>(Screen.Width);
	Screen.fHeight = static_cast<float>(Screen.Height);
	Screen.fAspectRatio = (Screen.fWidth / Screen.fHeight);

	pattern = hook::pattern("68 58 02 00 00 68 20 03 00 00");
	injector::WriteMemory(pattern.get(0).get<uint32_t>(1), Screen.Width, true);
	injector::WriteMemory(pattern.get(0).get<uint32_t>(6), Screen.Height, true);

	pattern = hook::pattern("BE 20 03 00 00");
	for (size_t i = 0; i < pattern.size(); ++i)
	{
		injector::WriteMemory(pattern.get(i).get<uint32_t>(1), Screen.Width, true);
	}
	pattern = hook::pattern("BF 58 02 00 00");
	for (size_t i = 0; i < pattern.size(); ++i)
	{
		injector::WriteMemory(pattern.get(i).get<uint32_t>(1), Screen.Height, true);
	}

	pattern = hook::pattern("74 22 48 75 25 68");
	injector::WriteMemory<uint8_t>(pattern.get(0).get<uint32_t>(0), 0xEB, true);

	pattern = hook::pattern("83 7D F0 02 75 ? 8B 45 08");
	injector::WriteMemory<uint8_t>(pattern.get(0).get<uint32_t>(4), 0xEB, true);
	

	pattern = hook::pattern("C7 45 D0 20 03 00 00");
	for (size_t i = 0; i < pattern.size(); ++i)
	{
		injector::WriteMemory(pattern.get(i).get<uint32_t>(3), Screen.Width, true);
	}

	pattern = hook::pattern("C7 45 D4 58 02 00 00");
	for (size_t i = 0; i < pattern.size(); ++i)
	{
		injector::WriteMemory(pattern.get(i).get<uint32_t>(3), Screen.Height, true);
	}

	pattern = hook::pattern("C7 85 5C FF FF FF 20 03 00 00");
	injector::WriteMemory(pattern.get(0).get<uint32_t>(6), Screen.Width, true);
	pattern = hook::pattern("C7 85 60 FF FF FF 58 02 00 00");
	injector::WriteMemory(pattern.get(0).get<uint32_t>(6), Screen.Height, true);

	Screen.fAspectRatio = (Screen.fHeight / Screen.fWidth);
	pattern = hook::pattern("D9 05 ? ? ? ? 83 EC 10 D9 5C 24 0C");
	injector::WriteMemory<float>(*pattern.get(0).get<uint32_t*>(2), Screen.fAspectRatio, true);
	return 0;
}


BOOL APIENTRY DllMain(HMODULE /*hModule*/, DWORD reason, LPVOID /*lpReserved*/)
{
	if (reason == DLL_PROCESS_ATTACH)
	{
		Init(NULL);
	}
	return TRUE;
}