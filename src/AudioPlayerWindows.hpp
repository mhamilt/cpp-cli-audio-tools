//
//  AudioPlayerWindows.hpp
//  CliAudioToolExample
//
//  Created by mhamilt7 on 03/02/2020.
//  Copyright © 2020 mhamilt7. All rights reserved.
//

#ifndef AudioPlayerWindows_hpp
#define AudioPlayerWindows_hpp
#if defined _WIN32 || defined _WIN64
//==============================================================================
#include <iostream>
#include <windows.h>
//#include <mmsystem.h>
#include <Mmdeviceapi.h>
#include <audioclient.h>
#include "WavCodec.hpp"

//==============================================================================

class AudioPlayerWindows
{
public:
    AudioPlayerWindows ();
    ~AudioPlayerWindows ();
    //==========================================================================
    /**
     Send list of available audio devices to stdout
     */
    //static void listAudioDevices ();
    //==========================================================================
    /**
     Play a given .wav file

     @param inputfname path to wav file
     */
    //void playFile (const char* inputfname);
    //==========================================================================
    /**
     Print metadata found in header of MIDI file

     @param filename path to midi file
     */
    //void printMidiHeader (const char* filename);
    //==========================================================================
    /**
     play audio data in an array unsigned ints representing byte data or a float array
     with values beteen -1 and 1

     @param audioData audio data array
     @param channelCount number of channels
     @param bitDepth bit depth of output
     */
    void playAudioData (uint32_t* audioData,
                        unsigned int numSamples,
                        uint8_t channelCount,
                        unsigned int samplingRate,
                        uint8_t bitDepth);
    /**
     play audio data in an array of floats

     @param audioData audio data as array of floats between -1 and 1
     @param numSamples total number of samples
     @param channelCount number of channels
     */
    HRESULT playAudioData (float* audioData,
                           unsigned long numSamples,
                           uint8_t channelCount);
    //==========================================================================
    // Getters and Setters

    float getSystemSampleRate ();

    /// <summary>
    /// Set Verbose mode on or off. When on, debug mesages will be sent to std cout.
    /// </summary>
    /// <param name="debugOn">verbose mode on (true) or off (false)</param>
    void setVerboseMode (bool verboseOn);

    
private:
    static void testError (const char* message);
    void playAudio (void* data,
                    uint8_t channelCount,
                    UINT32 numberOfBytes,
                    UINT32 samplingRate,
                    uint8_t bitDepth);
    /**
     convert floating point number to a byte value

     @param val float value
     @param byteNum the byte index of number. This is only relevant for 16 bit samples and greater
     @return byte value
     */
    static uint8_t audioFloat2Byte (float val, float maxValue, uint8_t byteNum);

    /**
    * Play audio data using WASAPI, see: https://docs.microsoft.com/en-us/windows/win32/coreaudio/rendering-a-stream
    */
    HRESULT PlayAudioStream ();
    /// <summary>
    /// 
    /// </summary>
    /// <param name="wfmx"></param>
    /// <returns></returns>
    HRESULT SetFormat (WAVEFORMATEX* wfmx);

    /// <summary>
    /// 
    /// </summary>
    /// <param name="frameCount"></param>
    /// <param name="audioOutputBuffer"></param>
    /// <param name="flags"></param>
    /// <returns></returns>
    HRESULT LoadData (UINT32 frameCount, BYTE* audioOutputBuffer, DWORD* flags);

    void initAudio (float* audioData,
                    unsigned int numSamples);
    void reset ();
  
    void printDebugMessage (const char* msg);
private:
    /// internal file reader            
    // WavCodec wavReadWrite;
    /// <summary>
    /// Audio Output Format
    /// </summary>
    WAVEFORMATEXTENSIBLE  format;
    /// <summary>
    /// pointer to audio data: pcm float format range -1.0f to 1.0f
    /// </summary>
    float* audioData;
    /// <summary>
    /// number of audio sample in audioData
    /// </summary>
    unsigned long numAudioSamples;
    /// <summary>
    /// 
    /// </summary>
    unsigned int audioDataBufferPos = 0;
    bool initialised = false;
    bool VERBOSE = true;
    //------------------------------------------------------------------------------------------------------
    // Windows specific constants
    static constexpr REFERENCE_TIME REFTIMES_PER_SEC = 5000000;
    static constexpr REFERENCE_TIME REFTIMES_PER_MILLISEC = 10000;
    const CLSID CLSID_MMDeviceEnumerator = __uuidof( MMDeviceEnumerator );
    const IID IID_IMMDeviceEnumerator = __uuidof( IMMDeviceEnumerator );
    const IID IID_IAudioClient = __uuidof( IAudioClient );
    const IID IID_IAudioRenderClient = __uuidof( IAudioRenderClient );

    // WASAPI Stream Render Variables
    HRESULT hr;
    REFERENCE_TIME hnsRequestedDuration = REFTIMES_PER_SEC;
    REFERENCE_TIME hnsActualDuration;
    IMMDeviceEnumerator* pEnumerator = NULL;
    IMMDevice* pDevice = NULL;
    IAudioClient* pAudioClient = NULL;
    IAudioRenderClient* pRenderClient = NULL;
    WAVEFORMATEX* pwfx = NULL;
    UINT32 bufferFrameCount;
    UINT32 numFramesAvailable;
    UINT32 numFramesPadding;
    BYTE* pData;
    DWORD flags = 0;
};


// Ugly define macros I will get rid of at some point
#define EXIT_ON_ERROR(hres)  \
              if (FAILED(hres)) {std::cout << hres<< '\n'; goto Exit; }
#define SAFE_RELEASE(punk)  \
              if ((punk) != NULL)  \
                { (punk)->Release(); (punk) = NULL; }


#endif /* Windows Compile guard */
#endif /* AudioPlayerWindows_hpp */
