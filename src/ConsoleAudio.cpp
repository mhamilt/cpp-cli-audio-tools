#include <cstdio>
#include <Windows.h>
#include <iostream>

// Windows multimedia device
#include <Mmdeviceapi.h>
#include <Functiondiscoverykeys_devpkey.h>

// WASAPI
#include <Audiopolicy.h>
#include <Audioclient.h>

#include <random>

class Noise_Gen 
{
public:
	Noise_Gen() : format(), engine(__rdtsc()), float_dist(-1.f, 1.f) 
	{
		float radsPerSec = 2 * 3.1415926536 * 110 / 48000.0;
		for (unsigned long i = 0; i < 48000; i++)
		{
			float sampleValue = sin(radsPerSec * (float)i);
			a440[i] = sampleValue;	
		}
	};

	void SetFormat(WAVEFORMATEX* wfex) 
	{

		if (wfex->wFormatTag == WAVE_FORMAT_EXTENSIBLE) 
		{
			format = *reinterpret_cast<WAVEFORMATEXTENSIBLE*>(wfex);
		}
		else 
		{
			format.Format = *wfex;
			format.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
			INIT_WAVEFORMATEX_GUID(&format.SubFormat, wfex->wFormatTag);
			format.Samples.wValidBitsPerSample = format.Format.wBitsPerSample;
			format.dwChannelMask = 0;
		}
		const UINT16 formatTag = EXTRACT_WAVEFORMATEX_ID(&format.SubFormat);
		
		std::cout << format.Format.nChannels << '\n';
		switch (formatTag)
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

	/// (The size of an audio frame = nChannels * wBitsPerSample)
	void FillBuffer(UINT32 bufferFrameCount, BYTE* pData, DWORD* flags) 
	{
		const UINT16 formatTag = EXTRACT_WAVEFORMATEX_ID(&format.SubFormat);
		if (formatTag == WAVE_FORMAT_IEEE_FLOAT) 
		{
			float* fData = (float*)pData;
			for (UINT32 i = 0; i < format.Format.nChannels * bufferFrameCount; i++) 
			{
				fData[i] = float_dist(engine);
			}
		}
		else if (formatTag == WAVE_FORMAT_PCM) 
		{
			using rndT = decltype(engine)::result_type;
			UINT32 iterations = format.Format.nBlockAlign * bufferFrameCount / sizeof(rndT);
			UINT32 leftoverBytes = format.Format.nBlockAlign * bufferFrameCount % sizeof(rndT);
			rndT* iData = (rndT*)pData;
			UINT32 i = 0;
			for (; i < iterations; i++) 
			{
				iData[i] = engine();
			}
			if (leftoverBytes != 0) 
			{
				rndT lastRnd = engine();
				BYTE* pLastBytes = pData + i * sizeof(rndT);
				for (UINT32 j = 0; j < leftoverBytes; ++j) 
				{
					pLastBytes[j] = lastRnd >> (j * 8) & 0xFF;
				}
			}
		}
		else 
		{
			//memset(pData, 0, wfex.Format.nBlockAlign * bufferFrameCount);
			*flags = AUDCLNT_BUFFERFLAGS_SILENT;
		}
	}

	/*
	**/
	void fillBufferWithSin(UINT32 bufferFrameCount, BYTE* pData, DWORD* flags)
	{
		float* fData = (float*)pData;
	
		for (UINT32 i = 0; i < bufferFrameCount; i+=4)
		{
			fData[i] = a440[i];
		}
	}
private:
	WAVEFORMATEXTENSIBLE format;

	float a440[48000];
	std::mt19937_64 engine;
	std::uniform_real_distribution<float> float_dist;
};

// REFERENCE_TIME time units per second and per millisecond
#define REFTIMES_PER_SEC  5000000ull // in 100-nanoseconds (or 10ths of a microsecond)
#define REFTIMES_PER_MILLISEC  10000	

#define EXIT_ON_ERROR(hres)  \
              if (FAILED(hres)) { goto Exit; }
#define SAFE_RELEASE(punk)  \
              if ((punk) != NULL)  \
                { (punk)->Release(); (punk) = NULL; }

HRESULT PlayAudioStream(Noise_Gen* pMySource) 
{
	unsigned long totalFrames = 0;
	printf("START\n");
	HRESULT hr;
	REFERENCE_TIME hnsRequestedDuration = REFTIMES_PER_SEC;
	REFERENCE_TIME hnsActualDuration;
	IMMDeviceEnumerator* pEnumerator = NULL;
	IPropertyStore* pPropertyStore = NULL;
	IMMDevice* pDevice = NULL;
	IAudioClient* pAudioClient = NULL;
	IAudioRenderClient* pRenderClient = NULL;
	WAVEFORMATEX* pwfx = NULL;
	UINT32 bufferFrameCount;
	BYTE* pData;
	DWORD flags = 0;
	PROPVARIANT name;
	UINT32 numFramesPadding;
	UINT32 numFramesAvailable;

	hr = CoCreateInstance(__uuidof(MMDeviceEnumerator),
		NULL,
		CLSCTX_ALL, IID_PPV_ARGS(&pEnumerator));
	EXIT_ON_ERROR(hr);
	hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice);
	EXIT_ON_ERROR(hr);

	hr = pDevice->OpenPropertyStore(STGM_READ, &pPropertyStore);
	EXIT_ON_ERROR(hr);
	PropVariantInit(&name);
	hr = pPropertyStore->GetValue(PKEY_Device_FriendlyName, &name);
	EXIT_ON_ERROR(hr);
	printf("%S", name.pwszVal);
	printf("\n");
	hr = pDevice->Activate(__uuidof(pAudioClient), 
							CLSCTX_ALL,
							NULL, 
							(void**)&pAudioClient);
	EXIT_ON_ERROR(hr);
	hr = pAudioClient->GetMixFormat(&pwfx);
	EXIT_ON_ERROR(hr);
	
	std::cout << "hnsRequestedDuration (100-nanosecond units): " << hnsRequestedDuration << '\n';
	std::cout << "hnsRequestedDuration: " << hnsRequestedDuration/(10000 * 1000) << '\n';
	hr = pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED,
									0, 
									hnsRequestedDuration,
									0, 
									pwfx, 
									NULL);
	EXIT_ON_ERROR(hr);
	
	pMySource->SetFormat(pwfx); // Tell the audio source which format to use.
	
	hr = pAudioClient->GetBufferSize(&bufferFrameCount); // Get the actual size of the allocated buffer.
	std::cout << "bufferFrameCount: " << bufferFrameCount << '\n';
	std::cout << "buffer length (seconds): " << bufferFrameCount / pwfx->nSamplesPerSec << '\n';
	std::cout << "pwfx->nSamplesPerSec: " << pwfx->nSamplesPerSec << '\n';

	EXIT_ON_ERROR(hr);
	hr = pAudioClient->GetService(IID_PPV_ARGS(&pRenderClient));
	EXIT_ON_ERROR(hr);
	hr = pRenderClient->GetBuffer(bufferFrameCount, &pData); // Grab the entire buffer for the initial fill operation.
	EXIT_ON_ERROR(hr);

	pMySource->fillBufferWithSin(bufferFrameCount, pData, &flags); // Load the initial data into the shared buffer.

	hr = pRenderClient->ReleaseBuffer(bufferFrameCount, flags);
	EXIT_ON_ERROR(hr);
	
	hnsActualDuration = REFTIMES_PER_SEC * bufferFrameCount / pwfx->nSamplesPerSec; // Calculate the actual duration of the allocated buffer.
	std::cout << "hnsActualDuration: " << hnsActualDuration << '\n';
	hr = pAudioClient->Start();  // Start playing.
	EXIT_ON_ERROR(hr);
	
	DWORD sleepTime; // Each loop fills about half of the shared buffer.
	//while (flags != AUDCLNT_BUFFERFLAGS_SILENT) 
	//{
		// Sleep for half the buffer duration.
		Sleep((DWORD)(hnsActualDuration / REFTIMES_PER_MILLISEC / 2));

		// See how much buffer space is available.
		hr = pAudioClient->GetCurrentPadding(&numFramesPadding);
		std::cout << "numFramesPadding: " << numFramesPadding << '\n';
		EXIT_ON_ERROR(hr);
		
		numFramesAvailable = bufferFrameCount - numFramesPadding;
		hr = pRenderClient->GetBuffer(numFramesAvailable, &pData); // Grab all the available space in the shared buffer.
		EXIT_ON_ERROR(hr);

		// Get next 1/2-second of data from the audio source.
		pMySource->fillBufferWithSin(numFramesAvailable, pData, &flags);
		totalFrames += numFramesAvailable;
		std::cout << "numFramesAvailable: " << numFramesAvailable<< ' ' << totalFrames <<'\n';

		hr = pRenderClient->ReleaseBuffer(numFramesAvailable, flags);
		EXIT_ON_ERROR(hr);
	
	sleepTime = (DWORD)(hnsActualDuration / REFTIMES_PER_MILLISEC / 2); // Wait for last data in buffer to play before stopping.

	//if (sleepTime != 0)
	//{
	//	Sleep(sleepTime);
	//}
	hr = pAudioClient->Stop();  // Stop playing.
	EXIT_ON_ERROR(hr);

Exit:
	CoTaskMemFree(pwfx);
	SAFE_RELEASE(pRenderClient);
	SAFE_RELEASE(pAudioClient);
	SAFE_RELEASE(pDevice);
	SAFE_RELEASE(pPropertyStore); // you forgot to free the property store
	SAFE_RELEASE(pEnumerator);
	return hr;
}

int main()
{
	std::cout << "start main\n";
	HRESULT hr = CoInitialize(nullptr);
	if (FAILED(hr)) { return hr; }

	Noise_Gen ng;
	PlayAudioStream(&ng);

	CoUninitialize();
}