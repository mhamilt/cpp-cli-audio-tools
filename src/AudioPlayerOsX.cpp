#ifdef __APPLE__
#include "TargetConditionals.h"
#if defined(TARGET_OS_MAC) && defined(AUDIO_TOOLBOX_VERSION)
//==============================================================================
#ifndef OSX_AUDIOPLAYER_H
#define checkStatus(status) checkStatus_(status, __FILE__, __LINE__)
#include "AudioPlayerOsX.hpp"
//==============================================================================
AudioPlayerOsX::AudioPlayerOsX()
{
}
//==============================================================================
AudioPlayerOsX::~AudioPlayerOsX()
{
    OSStatus status;
    status = AudioQueueDispose (aqData.mQueue,true);
    //checkStatus(status);
    status = AudioFileClose(aqData.mAudioFile);
    //checkStatus(status);
    free (aqData.mPacketDescs);
}
//==============================================================================
void AudioPlayerOsX::HandleOutputBuffer (void                *aqData,
                                         AudioQueueRef       inAQ,
                                         AudioQueueBufferRef inBuffer)
{
    OSStatus status;
    AQPlayerState *pAqData = (AQPlayerState *) aqData;
    if (pAqData->mIsRunning == 0)
    {
        return;
    }
    UInt32 numBytesReadFromFile;
    UInt32 numPackets = pAqData->mNumPacketsToRead;
    status = AudioFileReadPackets (
                                   pAqData->mAudioFile,
                                   false,
                                   &numBytesReadFromFile,
                                   pAqData->mPacketDescs,
                                   pAqData->mCurrentPacket,
                                   &numPackets,
                                   inBuffer->mAudioData
                                   );
    
    if (numPackets != 0)
    {
        inBuffer->mAudioDataByteSize = numBytesReadFromFile;
        status = AudioQueueEnqueueBuffer (
                                          pAqData->mQueue,
                                          inBuffer,
                                          (pAqData->mPacketDescs ? numPackets : 0),
                                          pAqData->mPacketDescs);
        pAqData->mCurrentPacket += numPackets;
    }
    else
    {
        status = AudioQueueStop (pAqData->mQueue,false);
        pAqData->mIsRunning = false;
    }
}
//==============================================================================
bool AudioPlayerOsX::load(CFURLRef url)
{
    OSStatus status;
    memset(&aqData,0,sizeof(aqData));
    timeBase = 0;
    status = AudioFileOpenURL(url,kAudioFileReadPermission,0,&aqData.mAudioFile);
    
    //checkStatus(status);
    
    if( status != noErr )
    {
        return false;
    }
    
    UInt32 dataFormatSize = sizeof (aqData.mDataFormat);
    
    status = AudioFileGetProperty (
                                   aqData.mAudioFile,
                                   kAudioFilePropertyDataFormat,
                                   &dataFormatSize,
                                   &aqData.mDataFormat
                                   );
    //checkStatus(status);
    //======================================================================
    status = AudioQueueNewOutput (
                                  &aqData.mDataFormat,
                                  HandleOutputBuffer,
                                  &aqData,
                                  CFRunLoopGetCurrent (),
                                  kCFRunLoopCommonModes,
                                  0,
                                  &aqData.mQueue
                                  );
    //checkStatus(status);
    //======================================================================
    UInt32 maxPacketSize;
    UInt32 propertySize = sizeof (maxPacketSize);
    status = AudioFileGetProperty (
                                   aqData.mAudioFile,
                                   kAudioFilePropertyPacketSizeUpperBound,
                                   &propertySize,
                                   &maxPacketSize
                                   );
    //checkStatus(status);
    //======================================================================
    deriveBufferSize (
                      aqData.mDataFormat,
                      maxPacketSize,
                      0.5,
                      &aqData.bufferByteSize,
                      &aqData.mNumPacketsToRead);
    //======================================================================
    bool isFormatVBR = (
                        aqData.mDataFormat.mBytesPerPacket == 0 ||
                        aqData.mDataFormat.mFramesPerPacket == 0
                        );
    
    if (isFormatVBR)
    {
        aqData.mPacketDescs =
        (AudioStreamPacketDescription*) malloc (
                                                aqData.mNumPacketsToRead * sizeof (AudioStreamPacketDescription)
                                                );
    }
    else
    {
        aqData.mPacketDescs = nullptr;
    }
    //======================================================================
    UInt32 cookieSize = sizeof (UInt32);
    OSStatus couldNotGetProperty =
    AudioFileGetPropertyInfo (
                              aqData.mAudioFile,
                              kAudioFilePropertyMagicCookieData,
                              &cookieSize,
                              nullptr
                              );
    
    if (!couldNotGetProperty && cookieSize)
    {
        char* magicCookie = (char *) malloc (cookieSize);
        status = AudioFileGetProperty (
                                       aqData.mAudioFile,
                                       kAudioFilePropertyMagicCookieData,
                                       &cookieSize,
                                       magicCookie
                                       );
        //checkStatus(status);
        
        status = AudioQueueSetProperty (
                                        aqData.mQueue,
                                        kAudioQueueProperty_MagicCookie,
                                        magicCookie,
                                        cookieSize
                                        );
        //checkStatus(status);
        
        free (magicCookie);
    }
    //======================================================================
    return true;
}
//==============================================================================
bool AudioPlayerOsX::isPlaying()
{
    return aqData.mIsRunning;
}
//==============================================================================
void AudioPlayerOsX::primeBuffer()
{
    OSStatus status;
    for (int i = 0; i < kNumberBuffers; ++i)
    {
        status = AudioQueueAllocateBuffer (aqData.mQueue,aqData.bufferByteSize,&aqData.mBuffers[i]);
        //checkStatus(status);
        HandleOutputBuffer (&aqData,aqData.mQueue,aqData.mBuffers[i]);
    }
}
//==============================================================================
void AudioPlayerOsX::play()
{
    OSStatus status;
    aqData.mIsRunning = true;
    aqData.mCurrentPacket = 0;
    primeBuffer();
    Float32 gain = 1.0;
    // Optionally, allow user to override gain setting here
    status = AudioQueueSetParameter (
                                     aqData.mQueue,
                                     kAudioQueueParam_Volume,
                                     gain
                                     );
    //checkStatus(status);
    //======================================================================
    status = AudioQueueStart (aqData.mQueue,nullptr);
    //checkStatus(status);
}
//==============================================================================
double AudioPlayerOsX::duration()
{
    double dur = 0;
    unsigned int sz = sizeof(dur);
    OSStatus status = AudioFileGetProperty(aqData.mAudioFile, kAudioFilePropertyEstimatedDuration, (UInt32*)&sz, &dur);
    //checkStatus(status);
    return dur;
}

//==============================================================================
void AudioPlayerOsX::seekToPacket(uint64_t packet)
{
    AudioQueueStop(aqData.mQueue, true);
    aqData.mCurrentPacket = rand()%1000;
    primeBuffer();
    AudioQueueStart(aqData.mQueue, nullptr);
    
}
//==============================================================================
void AudioPlayerOsX::seek(double sec)
{
    double frame = sec * aqData.mDataFormat.mSampleRate;
    
    AudioFramePacketTranslation trans;
    trans.mFrame = frame;
    
    unsigned int sz = sizeof(trans);
    OSStatus status = AudioFileGetProperty(aqData.mAudioFile, kAudioFilePropertyFrameToPacket, (UInt32*)&sz, &trans);
    
    seekToPacket(trans.mPacket);
    trans.mFrameOffsetInPacket = 0; // Don't support sub packet seeking..
    
    status = AudioFileGetProperty(aqData.mAudioFile, kAudioFilePropertyPacketToFrame, (UInt32*)&sz, &trans);
    
    timeBase = trans.mFrame / aqData.mDataFormat.mSampleRate;
    
}
//==============================================================================
double AudioPlayerOsX::progress() const
{
    double p = 0;
    AudioTimeStamp timeStamp;
    AudioQueueGetCurrentTime (
                              aqData.mQueue,
                              nullptr,
                              &timeStamp,
                              nullptr
                              );
    p = timeStamp.mSampleTime/aqData.mDataFormat.mSampleRate + timeBase;
    return p;
}
//==============================================================================
void AudioPlayerOsX::checkStatus_(OSStatus status, const char* file, int line)
{
    if(status != noErr) {
        std::cerr << file << ":" << line << ": ";
        char cc[5];
        *((unsigned int*)cc) = status;
        cc[4] = 0;
        std::cerr << "Error status " << status << ": " << cc << std::endl;
    } else {
        //        std::cerr << "OK" << std::endl;
    }
}
//==============================================================================
void AudioPlayerOsX::deriveBufferSize (AudioStreamBasicDescription &ASBDesc,
                                       UInt32 maxPacketSize,
                                       Float64 seconds,
                                       UInt32 *outBufferSize,
                                       UInt32 *outNumPacketsToRead
                                       )
{
    static const int maxBufferSize = 0x50000;
    static const int minBufferSize = 0x4000;
    
    if (ASBDesc.mFramesPerPacket != 0)
    {
        Float64 numPacketsForTime =
        ASBDesc.mSampleRate / ASBDesc.mFramesPerPacket * seconds;
        *outBufferSize = numPacketsForTime * maxPacketSize;
    }
    else
    {
        *outBufferSize =
        maxBufferSize > maxPacketSize ?
        maxBufferSize : maxPacketSize;
    }
    
    if (
        *outBufferSize > maxBufferSize &&
        *outBufferSize > maxPacketSize
        )
    {
        *outBufferSize = maxBufferSize;
    }
    else
    {
        if (*outBufferSize < minBufferSize)
            *outBufferSize = minBufferSize;
    }
    
    *outNumPacketsToRead = *outBufferSize / maxPacketSize;
}
//==============================================================================
void AudioPlayerOsX::playFile(const char *fn)
{
    CFURLRef url = CFURLCreateFromFileSystemRepresentation(nullptr, (UInt8*)fn, strlen(fn), false );
    if(!this->load(url))
    {
        std::cout << "OSX Player: Something went terribly wrong when loading the file" << '\n' << '\n';
    }
    else
    {
    CFRelease(url);
    
    //==========================================================================
    this->play();
    //==========================================================================
    do
    {
        CFRunLoopRunInMode (kCFRunLoopDefaultMode,.5,false);
    }
    while (this->isPlaying());
    }
}

#endif
#endif
#endif

