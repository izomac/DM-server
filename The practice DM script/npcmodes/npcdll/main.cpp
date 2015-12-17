#include <stdio.h>
#include <iostream>
#include "SDK/plugin.h"

typedef
	void (* logprintf_t)(char *, ...);

logprintf_t
	logprintf;

extern void *
	pAMXFunctions;

static cell AMX_NATIVE_CALL
	n_HelloWorld(AMX * amx, cell * params)
{
	logprintf("logprintf %d: Hello from this NPC plugin!\n", 42);
	return 0;
}

PLUGIN_EXPORT int PLUGIN_CALL
	AmxLoad(AMX * amx)
{
	logprintf("AmxLoad\n");
	AMX_NATIVE_INFO
		pluginNatives[] =
			{
				{"HelloWorld", n_HelloWorld},
				{0,        0}
			};
	return amx_Register(amx, pluginNatives, -1);
}

PLUGIN_EXPORT int PLUGIN_CALL
	AmxUnload(AMX * amx)
{
	logprintf("AmxUnload\n");
	return 0;
}

PLUGIN_EXPORT bool PLUGIN_CALL
	Load(void ** ppData)
{
	pAMXFunctions = ppData[PLUGIN_DATA_AMX_EXPORTS];
	logprintf = (logprintf_t)ppData[PLUGIN_DATA_LOGPRINTF];
	logprintf("Load\n");
	return true;
}

PLUGIN_EXPORT void PLUGIN_CALL
	Unload()
{
	logprintf("Unload\n");
}

PLUGIN_EXPORT unsigned int PLUGIN_CALL
	Supports() 
{
	logprintf("Supports\n");
	return SUPPORTS_VERSION | SUPPORTS_AMX_NATIVES | SUPPORTS_PROCESS_TICK;
}

PLUGIN_EXPORT void PLUGIN_CALL
	ProcessTick() 
{
	logprintf("ProcessTick\n");
}
