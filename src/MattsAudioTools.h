
#ifndef CliAudioTools_h
#define CliAudioTools_h
//==============================================================================
#ifdef __APPLE__
#include "TargetConditionals.h"
#ifdef TARGET_OS_MAC
#include "AudioPlayerOsX.hpp"
#endif
#endif
#include "AudioPlayerOpenAL.hpp"
#include "WavCodec.hpp"
//==============================================================================
#endif /*CliAudioTools_h*/
