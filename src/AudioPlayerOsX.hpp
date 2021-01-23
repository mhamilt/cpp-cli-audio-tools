#ifdef __APPLE__
#include "TargetConditionals.h"
#if defined(TARGET_OS_MAC) && defined(AUDIO_TOOLBOX_VERSION)

#ifndef OSX_AUDIOPLAYER_H
#define OSX_AUDIOPLAYER_H
//==============================================================================
#include <AudioToolbox/AudioToolbox.h>
#include <CoreFoundation/CoreFoundation.h>
#include <iostream>
//==============================================================================
/***/
static const int kNumberBuffers = 2;                        // 1
/***/
struct AQPlayerState
{
    /***/
    AudioStreamBasicDescription   mDataFormat;              // 2
    /***/
    AudioQueueRef                 mQueue;                   // 3
    /***/
    AudioQueueBufferRef           mBuffers[kNumberBuffers]; // 4
    /***/
    AudioFileID                   mAudioFile;               // 5
    /***/
    UInt32                        bufferByteSize;           // 6
    /***/
    SInt64                        mCurrentPacket;           // 7
    /***/
    UInt32                        mNumPacketsToRead;        // 8
    /***/
    AudioStreamPacketDescription  *mPacketDescs;            // 9
    /***/
    bool                          mIsRunning;               // 10
};
//==============================================================================
class AudioPlayerOsX
{
    //==========================================================================
public:
    /***/
    AudioPlayerOsX();
    /***/
    ~AudioPlayerOsX();
    //==========================================================================
    /** Load audio file
     @param url CFURLRef of the file path and name
     
     @returns false on failure and true in sucessful
     */
    bool load(CFURLRef url);
    //==========================================================================
    
    /** Virtual function from AudioPlayer Class
     */
    bool isPlaying();
    
    //==========================================================================
    /***/
    void primeBuffer();
    //==========================================================================
    /***/
    void play();
    //==========================================================================
    /***/
    double duration();
    //==========================================================================
    /***/
    void seekToPacket(uint64_t packet);
    //==========================================================================
    /***/
    void seek(double sec);
    //==========================================================================
    /***/
    double progress() const;
    //==========================================================================
    /**
     play the entire contents of the given filename

     @param fn filename to play
     */
    void playFile(const char *fn);
    //==========================================================================
    /***/
    static void checkStatus_(OSStatus status, const char* file, int line);
    //==========================================================================
    static void HandleOutputBuffer (void                *aqData,
                                    AudioQueueRef       inAQ,
                                    AudioQueueBufferRef inBuffer);
    
private:
    //==========================================================================
    /***/
    static void deriveBufferSize (AudioStreamBasicDescription &ASBDesc,                            // 1
                                  UInt32                      maxPacketSize,                       // 2
                                  Float64                     seconds,                             // 3
                                  UInt32                      *outBufferSize,                      // 4
                                  UInt32                      *outNumPacketsToRead                 // 5
    );
    //==========================================================================
private:
    double timeBase;
    AQPlayerState aqData;
};

#endif /*EPOXY_AUDIOPLAYER_H*/
#endif
#endif
