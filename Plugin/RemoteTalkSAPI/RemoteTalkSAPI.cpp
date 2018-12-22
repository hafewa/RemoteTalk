﻿#include "pch.h"
#include "rtspCommon.h"
#include "rtspHookHandler.h"
#include "rtspTalkServer.h"


BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    if (fdwReason == DLL_PROCESS_ATTACH) {
        if (FAILED(::CoInitialize(NULL)))
            return FALSE;
    }
    else if (fdwReason == DLL_PROCESS_DETACH) {
    }
    return TRUE;
}

rtAPI rt::TalkInterface* rtGetTalkInterface()
{
    return nullptr;
}

rtAPI rt::TalkServer* rtspStartServer(int port)
{
    auto& inst = rtsp::TalkServer::getInstance();
    if (!inst.isRunning()) {
        rt::TalkServerSettings settings;
        settings.port = (uint16_t)port;
        inst.setSettings(settings);
        inst.start();
    }
    return &inst;
}
