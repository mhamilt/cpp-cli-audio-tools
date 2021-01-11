//-----------------------------------------------------------
// Play an audio stream on the default audio rendering
// device. The PlayAudioStream function allocates a shared
// buffer big enough to hold one second of PCM audio data.
// The function uses this buffer to stream data to the
// rendering device. The inner loop runs every 1/2 second.
//-----------------------------------------------------------

#include <iostream>
#include <windows.h>

// Windows multimedia device
#include <Mmdeviceapi.h>

// WASAPI
#include <audioclient.h>
//-----------------------------------------------------------
class MyAudioSource
{
public:
    MyAudioSource ();
    MyAudioSource (float freq) {
        frequency = freq;
    };
    ~MyAudioSource ();

    HRESULT SetFormat (WAVEFORMATEX*);
    HRESULT LoadData (UINT32, BYTE*, DWORD*);

private:
    void init ();
    bool initialised = false;
    WAVEFORMATEXTENSIBLE  format;
    unsigned int pcmPos = 0;
    UINT32 bufferSize;
    UINT32 bufferPos = 0;
    static const unsigned int sampleCount = 96000 * 10;
    float frequency = 440;
    float* pcmAudio = nullptr;
};
//-----------------------------------------------------------
MyAudioSource::MyAudioSource ()
{

}
//-----------------------------------------------------------

MyAudioSource::~MyAudioSource ()
{
    if ( pcmAudio )
    {
        delete[] pcmAudio;
    }
}
//-----------------------------------------------------------

void MyAudioSource::init ()
{
    pcmAudio = new float[sampleCount];
    const float radsPerSec = 2 * 3.1415926536 * frequency / (float) format.Format.nSamplesPerSec;
    for ( unsigned long i = 0; i < sampleCount; i++ )
    {
        float sampleValue = sin (radsPerSec * (float) i);
        pcmAudio[i] = sampleValue;
    }
    initialised = true;
}
//-----------------------------------------------------------

HRESULT MyAudioSource::SetFormat (WAVEFORMATEX* wfex)
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
    return 0;
}
//-----------------------------------------------------------

HRESULT MyAudioSource::LoadData (UINT32 totalFrames, BYTE* dataOut, DWORD* flags)
{
    float* fData = (float*) dataOut;
    UINT32 totalSamples = totalFrames * format.Format.nChannels;
    if ( !initialised )
    {
        init ();
        bufferSize = totalFrames * format.Format.nChannels;
        std::cout << "bufferSize: " << bufferSize << '\n';
        std::cout << "sampsPerChan: " << totalFrames / format.Format.nChannels << '\n';
        std::cout << "fData[totalSamples]: " << fData[totalFrames] << '\n';
        std::cout << "fData[bufferSize]: " << fData[bufferSize] << '\n';
        std::cout << "buffer address: " << int (dataOut) << '\n';
    }
    else
    {
        std::cout << "Frames to Fill: " << totalFrames << '\n';
        std::cout << "Samples to Fill: " << totalSamples << '\n';
        std::cout << "bufferPos: " << bufferPos << '\n';
        std::cout << "buffer address: " << int (dataOut) << '\n';

    }

    if ( pcmPos < sampleCount )
    {
        for ( UINT32 i = 0; i < totalSamples; i += format.Format.nChannels )
        {
            for ( size_t chan = 0; chan < format.Format.nChannels; chan++ )
            {
                fData[i + chan] = ( pcmPos < sampleCount ) ? pcmAudio[pcmPos] : 0.0f;
            }
            pcmPos++;
        }
        bufferPos += totalSamples;
        bufferPos %= bufferSize;
    }
    else
    {
        *flags = AUDCLNT_BUFFERFLAGS_SILENT;
    }
    return 0;
}
//-----------------------------------------------------------
// REFERENCE_TIME time units per second and per millisecond
#define REFTIMES_PER_SEC  2500000
#define REFTIMES_PER_MILLISEC  10000
//-----------------------------------------------------------

#define EXIT_ON_ERROR(hres)  \
              if (FAILED(hres)) {std::cout << hres<< '\n'; goto Exit; }
#define SAFE_RELEASE(punk)  \
              if ((punk) != NULL)  \
                { (punk)->Release(); (punk) = NULL; }

//-----------------------------------------------------------
const CLSID CLSID_MMDeviceEnumerator = __uuidof( MMDeviceEnumerator );
const IID IID_IMMDeviceEnumerator = __uuidof( IMMDeviceEnumerator );
const IID IID_IAudioClient = __uuidof( IAudioClient );
const IID IID_IAudioRenderClient = __uuidof( IAudioRenderClient );
//-----------------------------------------------------------
HRESULT PlayAudioStream (MyAudioSource* pMySource)
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
//-----------------------------------------------------------
int main ()
{
    std::cout << "start main\n";
    HRESULT hr = CoInitialize (nullptr);
    if ( FAILED (hr) ) {
        return hr;
    }

    MyAudioSource audio (440);
    PlayAudioStream (&audio);

    CoUninitialize ();
}
