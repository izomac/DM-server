#include "plugin.h"
#include <stdio.h>

#ifdef LINUX
#else
	#define VC_EXTRALEAN
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
	//#include <afx.h>
	
	DWORD
		gMainThread;
#endif

typedef int (PLUGIN_CALL * LoadAPI_t)(void **);
typedef void (PLUGIN_CALL * UnloadAPI_t)();
typedef int (PLUGIN_CALL * SupportsAPI_t)();
typedef int (PLUGIN_CALL * ProcessTickAPI_t)();

LoadAPI_t
	LoadAPI;

UnloadAPI_t
	UnloadAPI;

SupportsAPI_t
	SupportsAPI;

ProcessTickAPI_t
	ProcessTickAPI;

void *
	pAMXFunctions = 0;

char
	gNPCSleepfAssembly[5];

void
	AssemblySwap(char * addr, char * dat, int len)
{
	char
		swp;
	while (len--)
	{
		swp = addr[len];
		addr[len] = dat[len];
		dat[len] = swp;
	}
}

void
	AssemblyRedirect(void * from, void * to, char * ret)
{
	#ifdef LINUX
		size_t
			iPageSize = getpagesize(),
			iAddr = ((reinterpret_cast <uint32_t>(from) / iPageSize) * iPageSize),
			iCount = (5 / iPageSize) * iPageSize + iPageSize * 2;
		mprotect(reinterpret_cast <void*>(iAddr), iCount, PROT_READ | PROT_WRITE | PROT_EXEC);
		//mprotect(from, 5, PROT_READ | PROT_WRITE | PROT_EXEC);
	#else
		DWORD
			old;
		VirtualProtect(from, 5, PAGE_EXECUTE_READWRITE, &old);
	#endif
	*((unsigned char *)ret) = 0xE9;
	*((char **)(ret + 1)) = (char *)((char *)to - (char *)from) - 5;
	//std::cout << "fixes.plugin: load: " << (sizeof (char *)) << *((int *)(ret + 1)) << std::endl;
	AssemblySwap((char *)from, ret, 5);
}

#ifdef LINUX
#else
	void WINAPI NPCSleep(DWORD ms)
	{
		//OutputDebugString(L"Sleep\n");
		// Note, this may be called quite sporadically!
		//ProcessTick();
		if (gMainThread == GetCurrentThreadId()) ProcessTickAPI();
		SleepEx(ms, FALSE);
		//OutputDebugString(L"Done\n");
	}

	void NPClogprintf(char * s, ...)
	{
		// Format the message.
		char
			buffer[1024];
		va_list
			args;
		va_start(args, s);
		vsprintf(buffer, s, args);
		va_end(args);
		// Convert to a wide string.
		WCHAR
			lb[1024];
		MultiByteToWideChar(0, 0, buffer, sizeof (buffer) / sizeof (char), lb, sizeof (lb) / sizeof (WCHAR));
		// Output the debug message.
		OutputDebugString(lb);
	}
	
	BOOL WINAPI
		DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
	{
		switch (fdwReason)
		{
			case DLL_PROCESS_ATTACH:
			{
				// Find the (decorated) exported functions.
				LoadAPI = (LoadAPI_t)GetProcAddress(hinstDLL, "_Load@4");
				UnloadAPI = (UnloadAPI_t)GetProcAddress(hinstDLL, "_Unload@0");
				SupportsAPI = (SupportsAPI_t)GetProcAddress(hinstDLL, "_Supports@0");
				ProcessTickAPI = (ProcessTickAPI_t)GetProcAddress(hinstDLL, "_ProcessTick@0");
				// Does the "Load" function exist?
				if (LoadAPI)
				{
					void *
						exports[] = {
							(void *)NPClogprintf,
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
					if (!LoadAPI(exports)) return FALSE;
				}
				int
					sup = 0;
				// Does the "Supports" function exist?
				if (SupportsAPI) sup = SupportsAPI();
				if (sup & SUPPORTS_PROCESS_TICK && ProcessTickAPI)
				{
					// Hook "Sleep" so we can get called every server tick.
					FARPROC
						addr = GetProcAddress(GetModuleHandleA("Kernel32.dll"), "Sleep");
					AssemblyRedirect((void *)addr, (void *)NPCSleep, gNPCSleepfAssembly);
				}
				// Get the current thread for use in "Sleep".
				gMainThread = GetCurrentThreadId();
				break;
			}
			case DLL_PROCESS_DETACH:
				if (UnloadAPI) UnloadAPI();
				break;
		}
		return TRUE;
	}
#endif
