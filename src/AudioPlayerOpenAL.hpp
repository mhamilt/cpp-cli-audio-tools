#ifndef AudioPlayerOpenAL_hpp
#define AudioPlayerOpenAL_hpp
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

class AudioPlayerOpenAL
{
public:
    AudioPlayerOpenAL();
    ~AudioPlayerOpenAL();
    //==========================================================================
    /**
     Send list of available audio devices to stdout
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

#endif /* AudioPlayerOpenAL_hpp */
