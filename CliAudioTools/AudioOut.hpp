#ifndef AudioOut_hpp
#define AudioOut_hpp
//==============================================================================
#include <stdio.h>
#include "AudioWavFileReadWrite.hpp"
#ifdef __APPLE__
#include "TargetConditionals.h"
#ifdef TARGET_OS_MAC
#include <OpenAL/OpenAL.h>
#endif
#elif defined _WIN32 || defined _WIN64
#include <AL/al.h>
#include <AL/alc.h>
#endif
#include <OpenAL/OpenAL.h>
//==============================================================================

class AudioOut
{
public:
    AudioOut();
    ~AudioOut();
    //==========================================================================
    /**
     Send list of available audio devices to stdout
     
     @param devices Device specifier
     
     list_audio_devices(alcGetString(NULL, ALC_DEVICE_SPECIFIER));
     */
    static void listAudioDevices();
    //==========================================================================
    void playFile(const char *inputfname);
    //==========================================================================
    void printMidiHeader(const char *filename);
    
private:
    void testError(const char *message);
    
    /**
     get al format given a bit depth and channel count of the last read file.
     
     @return OpenAL Format enum
     */
    ALenum getAlFormat();
private:
    AudioWavFileReadWrite wavReadWrite;
    
};

#endif /* AudioOut_hpp */
