//
//  AudioPlayerWindows.cpp
//  CliAudioToolExample
//
//  Created by mhamilt7 on 03/02/2020.
//  Copyright © 2020 mhamilt7. All rights reserved.
//

#if defined _WIN32 || defined _WIN64
#include "AudioPlayerWindows.hpp"

HRESULT AudioPlayerWindows::PlayAudioStream ()
{
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

    std::cout << "CoCreateInstance\n";
    hr = CoCreateInstance (CLSID_MMDeviceEnumerator,
                           NULL,
                           CLSCTX_ALL, IID_IMMDeviceEnumerator,
                           (void**) &pEnumerator);
    EXIT_ON_ERROR (hr);
    std::cout << "GetDefaultAudioEndpoint\n";

    hr = pEnumerator->GetDefaultAudioEndpoint (
        eRender, eConsole, &pDevice);
    EXIT_ON_ERROR (hr);

    std::cout << "Activate\n";
    hr = pDevice->Activate (
        IID_IAudioClient, CLSCTX_ALL,
        NULL, (void**) &pAudioClient);
    EXIT_ON_ERROR (hr);

    std::cout << "GetMixFormat\n";
    hr = pAudioClient->GetMixFormat (&pwfx);
    EXIT_ON_ERROR (hr);

    std::cout << "Initialize\n";
    hr = pAudioClient->Initialize (
        AUDCLNT_SHAREMODE_SHARED,
        0,
        hnsRequestedDuration,
        0,
        pwfx,
        NULL);
    EXIT_ON_ERROR (hr);

    // Tell the audio source which format to use.
    std::cout << "pMySource->SetFormat\n";
    hr = pMySource->SetFormat (pwfx);
    EXIT_ON_ERROR (hr);

    // Get the actual size of the allocated buffer.
    std::cout << "GetBufferSize\n";
    hr = pAudioClient->GetBufferSize (&bufferFrameCount);
    EXIT_ON_ERROR (hr);

    std::cout << "GetService\n";

    hr = pAudioClient->GetService (
        IID_IAudioRenderClient,
        (void**) &pRenderClient);
    EXIT_ON_ERROR (hr);

    // Grab the entire buffer for the initial fill operation.
    hr = pRenderClient->GetBuffer (bufferFrameCount, &pData);
    EXIT_ON_ERROR (hr);

    // Load the initial data into the shared buffer.
    hr = pMySource->LoadData (bufferFrameCount, pData, &flags);
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
        hr = pMySource->LoadData (numFramesAvailable, pData, &flags);
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

#endif /* Windows Compile guard */
