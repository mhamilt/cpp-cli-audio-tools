//
//  AudioPlayerWindows.cpp
//  CliAudioToolExample
//
//  Created by mhamilt7 on 03/02/2020.
//  Copyright © 2020 mhamilt7. All rights reserved.
//

#if defined _WIN32 || defined _WIN64
#include "AudioPlayerWindows.hpp"

AudioPlayerWindows::AudioPlayerWindows ()
{
    HRESULT hr = CoInitialize (nullptr);
    if ( FAILED (hr) ) 
    {
        exit;
    }
    printDebugMessage ("CoCreateInstance");
    hr = CoCreateInstance (CLSID_MMDeviceEnumerator,
                           NULL,
                           CLSCTX_ALL, IID_IMMDeviceEnumerator,
                           (void**) &pEnumerator);
    printDebugMessage ("GetDefaultAudioEndpoint");

    hr = pEnumerator->GetDefaultAudioEndpoint (
        eRender, eConsole, &pDevice);

    printDebugMessage ("Activate");
    hr = pDevice->Activate (IID_IAudioClient, 
                            CLSCTX_ALL,
                            NULL, 
                            (void**) &pAudioClient);

    printDebugMessage ("GetMixFormat");
    hr = pAudioClient->GetMixFormat (&pwfx);
    
    printDebugMessage ("SetFormat");
    hr = SetFormat (pwfx); // Tell the audio source which format to use.
    
}

AudioPlayerWindows::~AudioPlayerWindows ()
{
    CoUninitialize ();
}

HRESULT AudioPlayerWindows::PlayAudioStream ()
{
    printDebugMessage ("Initialize");
    hr = pAudioClient->Initialize (
        AUDCLNT_SHAREMODE_SHARED,
        0,
        hnsRequestedDuration,
        0,
        pwfx,
        NULL);
     EXIT_ON_ERROR (hr);

    // Get the actual size of the allocated buffer.
    printDebugMessage ("GetBufferSize");
    hr = pAudioClient->GetBufferSize (&bufferFrameCount);
    EXIT_ON_ERROR (hr);

    printDebugMessage ("GetService");

    hr = pAudioClient->GetService (
        IID_IAudioRenderClient,
        (void**) &pRenderClient);
    EXIT_ON_ERROR (hr);

    // Grab the entire buffer for the initial fill operation.
    hr = pRenderClient->GetBuffer (bufferFrameCount, &pData);
    EXIT_ON_ERROR (hr);

    // Load the initial data into the shared buffer.
    hr = LoadData (bufferFrameCount, pData, &flags);
    EXIT_ON_ERROR (hr);

    hr = pRenderClient->ReleaseBuffer (bufferFrameCount, flags);
    EXIT_ON_ERROR (hr);

    // Calculate the actual duration of the allocated buffer.
    hnsActualDuration = (double) REFTIMES_PER_SEC *
        bufferFrameCount / pwfx->nSamplesPerSec;

    hr = pAudioClient->Start ();  // Start playing.
    EXIT_ON_ERROR (hr);

    // Each loop fills about half of the shared buffer.
    while ( flags != AUDCLNT_BUFFERFLAGS_SILENT )
    {
        // Sleep for half the buffer duration.
        Sleep ((DWORD) ( hnsActualDuration / REFTIMES_PER_MILLISEC / 2 ));

        // See how much buffer space is available.
        hr = pAudioClient->GetCurrentPadding (&numFramesPadding);
        EXIT_ON_ERROR (hr);

        numFramesAvailable = bufferFrameCount - numFramesPadding;

        // Grab all the available space in the shared buffer.
        hr = pRenderClient->GetBuffer (numFramesAvailable, &pData);
        EXIT_ON_ERROR (hr);

        // Get next 1/2-second of data from the audio source.
        hr = LoadData (numFramesAvailable, pData, &flags);
        EXIT_ON_ERROR (hr);

        hr = pRenderClient->ReleaseBuffer (numFramesAvailable, flags);
        EXIT_ON_ERROR (hr);
    }

    // Wait for last data in buffer to play before stopping.
    Sleep ((DWORD) ( hnsActualDuration / REFTIMES_PER_MILLISEC / 2 ));

    hr = pAudioClient->Stop ();  // Stop playing.
    EXIT_ON_ERROR (hr);

Exit:
    CoTaskMemFree (pwfx);
    SAFE_RELEASE (pEnumerator);
    SAFE_RELEASE (pDevice);
    SAFE_RELEASE (pAudioClient);
    SAFE_RELEASE (pRenderClient);

    return hr;
}

HRESULT AudioPlayerWindows::SetFormat (WAVEFORMATEX* wfex)
{
    if ( wfex->wFormatTag == WAVE_FORMAT_EXTENSIBLE )
    {
        format = *reinterpret_cast<WAVEFORMATEXTENSIBLE*>( wfex );
    }
    else
    {
        format.Format = *wfex;
        format.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
        INIT_WAVEFORMATEX_GUID (&format.SubFormat, wfex->wFormatTag);
        format.Samples.wValidBitsPerSample = format.Format.wBitsPerSample;
        format.dwChannelMask = 0;
    }
    
    if(VERBOSE)
    {
        const UINT16 formatTag = EXTRACT_WAVEFORMATEX_ID (&format.SubFormat);
        std::cout << "Channel Count: " << format.Format.nChannels << '\n';
        std::cout << "Audio Format: ";
        switch ( formatTag )
        {
            case WAVE_FORMAT_IEEE_FLOAT:
                std::cout << "WAVE_FORMAT_IEEE_FLOAT\n";
                break;
            case WAVE_FORMAT_PCM:
                std::cout << "WAVE_FORMAT_PCM\n";
                break;
            default:
                std::cout << "Wave Format Unknown\n";
                break;
        }
    }
    return 0;
}

HRESULT AudioPlayerWindows::LoadData (UINT32 frameCount, BYTE* audioOutputBuffer, DWORD* flags)
{
    float* fData = (float*) audioOutputBuffer;
    UINT32 totalBufferSamples = frameCount * format.Format.nChannels;
    if ( !initialised )
    {
        initialised = true;
        if ( VERBOSE )
        {
            std::cout << "bufferSize: " << frameCount * format.Format.nChannels << '\n';
            std::cout << "frameCount: " << frameCount << '\n';
            std::cout << "buffer address: " << int (audioOutputBuffer) << '\n';
        }
    }

    if ( audioDataBufferPos < numAudioSamples )
    {
        for ( UINT32 i = 0; i < totalBufferSamples; i += format.Format.nChannels )
        {
            for ( size_t chan = 0; chan < format.Format.nChannels; chan++ )
            {
                fData[i + chan] = ( audioDataBufferPos < numAudioSamples ) ? audioData[audioDataBufferPos] : 0.0f;
            }
            audioDataBufferPos++;
        }
    }
    else
    {
        *flags = AUDCLNT_BUFFERFLAGS_SILENT;
    }
    return 0;
}

void AudioPlayerWindows::initAudio (float* audioInData, unsigned int numSamples)
{
    audioData = audioInData;
    numAudioSamples = numSamples;
    audioDataBufferPos = 0;
}

void AudioPlayerWindows::reset ()
{    
    audioDataBufferPos = 0;
}

HRESULT AudioPlayerWindows::playAudioData (float* audioInData, unsigned long numSamples, uint8_t channelCount)
{
    printDebugMessage (__FUNCTION__);

    if ( channelCount != 1 )
    {
        std::cout << "Sorry, Channel format must be mono. If this is problematic, please write your concern on a note, nail it to a frisbee and fling it over a rainbow\n\n";
        return 1;
    }
    initialised = false;
    initAudio (audioInData, numSamples);

    PlayAudioStream();
}

float AudioPlayerWindows::getSystemSampleRate ()
{
    return format.Format.nSamplesPerSec;
}


void AudioPlayerWindows::printDebugMessage (const char* msg)
{
    if ( VERBOSE )
        std::cout << msg << '\n';
}
#endif /* Windows Compile guard */
