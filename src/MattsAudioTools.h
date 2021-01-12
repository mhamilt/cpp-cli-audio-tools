
#ifndef CliAudioTools_h
#define CliAudioTools_h
//==============================================================================
#ifdef __APPLE__
#include "TargetConditionals.h"
    #ifdef TARGET_OS_MAC
        #include "AudioPlayerOsX.hpp"
    #endif
#elif defined _WIN32 || defined _WIN64
    #include <windows.h>
    #include <mmsystem.h>
#endif
#include "AudioPlayerOpenAL.hpp"
#include "WavCodec.hpp"


//==============================================================================
#endif /*CliAudioTools_h*/
