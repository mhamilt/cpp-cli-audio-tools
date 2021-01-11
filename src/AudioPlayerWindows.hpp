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
    AudioPlayerWindows();
    ~AudioPlayerWindows();
    //==========================================================================
    /**
     Send list of available audio devices to stdout
     */
    static void listAudioDevices();
    //==========================================================================
    /**
     Play a given .wav file

     @param inputfname path to wav file
     */
    void playFile(const char *inputfname);
    //==========================================================================
    /**
     Print metadata found in header of MIDI file

     @param filename path to midi file
     */
    void printMidiHeader(const char *filename);
    //==========================================================================
    /**
     play audio data in an array unsigned ints representing byte data or a float array
     with values beteen -1 and 1

     @param audioData audio data array
     @param channelCount number of channels
     @param bitDepth bit depth of output
     */
    void playAudioData(uint32_t *audioData,
                       unsigned int numSamples,
                       uint8_t channelCount,
                       unsigned int samplingRate,
                       uint8_t bitDepth);
    /**
     play audio data in an array of floats

     @param audioData audio data as array of floats between -1 and 1
     @param numSamples total number of samples
     @param channelCount number of channels
     @param samplingRate sampling rate in hz
     @param bitDepth bit depth of output (only 8 and 16 supported by OpenAL)
     */
    void playAudioData(float *audioData,
                       unsigned int numSamples,
                       uint8_t channelCount,
                       unsigned int samplingRate,
                       uint8_t bitDepth);
private:
    static void testError(const char *message);
    void playAudio(void *data,
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
    static uint8_t audioFloat2Byte(float val, float maxValue, uint8_t byteNum);
    
private:
    /// internal file reader
    WavCodec wavReadWrite;
};

#endif /* Windows Compile guard */
#endif /* AudioPlayerWindows_hpp */
