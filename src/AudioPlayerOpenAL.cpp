//==============================================================================
#include "AudioPlayerOpenAL.hpp"
//==============================================================================

AudioPlayerOpenAL::AudioPlayerOpenAL()
{
    
}

AudioPlayerOpenAL::~AudioPlayerOpenAL()
{
    
}
//==============================================================================

void AudioPlayerOpenAL::listAudioDevices()
{
    const ALCchar *device = alcGetString(nullptr, ALC_DEVICE_SPECIFIER);
    const ALCchar *next = device + 1;
    size_t len = 0;
    
    fprintf(stdout, "Devices list:\n");
    fprintf(stdout, "----------\n");
    while (device && *device != '\0' && next && *next != '\0')
    {
        fprintf(stdout, "%s\n", device);
        len = strlen(device);
        device += (len + 1);
        next += (len + 2);
    }
    fprintf(stdout, "----------\n\n");
}

//==============================================================================
void AudioPlayerOpenAL::playAudio(ALvoid *data,
                                  uint8_t channelCount,
                                  ALsizei numberOfBytes,
                                  ALsizei samplingRate,
                                  uint8_t bitDepth)
{
    
    ALboolean enumeration = alcIsExtensionPresent(nullptr, "ALC_ENUMERATION_EXT");
    if (enumeration == AL_FALSE)
        fprintf(stderr, "enumeration extension not available\n");
    
    listAudioDevices();
    
    const ALCchar *defaultDeviceName = alcGetString(nullptr, ALC_DEFAULT_DEVICE_SPECIFIER);
    
    ALCdevice *device = alcOpenDevice(defaultDeviceName);
    if (!device)
    {
        testError("unable to open default device\n");
    }
    
    fprintf(stdout, "Device: %s\n\n", alcGetString(device, ALC_DEVICE_SPECIFIER));
    
    alGetError();
    
    ALCcontext *context = alcCreateContext(device, nullptr);
    if (!alcMakeContextCurrent(context))
    {
        fprintf(stderr, "failed to make default context\n");
    }
    
    testError("make default context");
    
    ALuint source;
    alGenSources((ALuint)1, &source);
    testError("source generation");
    
    ALuint buffer;
    alGenBuffers(1, &buffer);
    testError("buffer generation");
    
    if (!data)
    {
        std::cout << "LOAD ERROR: check the file name is correct" << '\n' << '\n';
    }
    
    alBufferData(buffer, getAlFormat(channelCount,bitDepth), data, numberOfBytes, samplingRate);
    testError("Fail at alBufferData\n");
    
    alSourcei(source, AL_BUFFER, buffer);
    testError("buffer binding\n");
    
    alSourcePlay(source);
    testError("source playing");
    
    ALint source_state;
    alGetSourcei(source, AL_SOURCE_STATE, &source_state);
    testError("source state get");
    while (source_state == AL_PLAYING)
    {
        alGetSourcei(source, AL_SOURCE_STATE, &source_state);
        testError("source state get");
    }
    
    /* exit context */
    alDeleteSources(1, &source);
    alDeleteBuffers(1, &buffer);
    device = alcGetContextsDevice(context);
    alcMakeContextCurrent(nullptr);
    alcDestroyContext(context);
    alcCloseDevice(device);
}
//==============================================================================
void AudioPlayerOpenAL::playFile(const char *inputfname)
{
    ALsizei numberOfBytes, samplingRate;
    ALvoid *data = wavReadWrite.readRawData(inputfname, &numberOfBytes, &samplingRate);
    playAudio(data, wavReadWrite.getFileChannelNumber(), numberOfBytes, samplingRate, wavReadWrite.getFileBitDepth());
}
//==============================================================================
void AudioPlayerOpenAL::playAudioData(float *audioData,
                                      unsigned int numSamples,
                                      uint8_t channelCount,
                                      unsigned int samplingRate,
                                      uint8_t bitDepth)
{
    const uint8_t bytesPerSample = bitDepth / 8;
    const unsigned int numberOfBytes = numSamples * channelCount * bytesPerSample;
    char *audioDataConversion = new char[numberOfBytes];
    const float maxValue = pow(2., bitDepth - 1);

    for (int i = 0; i < numberOfBytes; ++i)
    {
        audioDataConversion[i] = audioFloat2Byte(audioData[i/bytesPerSample],
                                                 maxValue,
                                                 (i % bytesPerSample));
    }
    
    playAudio(audioDataConversion, channelCount, numberOfBytes, samplingRate, bitDepth);
}

uint8_t AudioPlayerOpenAL::audioFloat2Byte(float val, float maxValue, uint8_t byteNum)
{
    uint8_t bitShift = byteNum * 8;
    uint8_t byteVal = (uint32_t((val + 1.) * 0.5 * maxValue) >> bitShift);
    return byteVal;
}
//==============================================================================
void AudioPlayerOpenAL::printMidiHeader(const char *filename)
{
    
}
//==============================================================================

void AudioPlayerOpenAL::testError(const char *message)
{
    ALCenum error = alGetError();
    if (error != AL_NO_ERROR)
    {
        fprintf(stderr, message, "\n");
    }
}


ALenum AudioPlayerOpenAL::getAlFormat(uint8_t channelCount, uint8_t bitDepth)
{
    bool stereo = (channelCount > 1);
    
    switch (bitDepth)
    {
        case 16:
            if (stereo)
                return AL_FORMAT_STEREO16;
            else
                return AL_FORMAT_MONO16;
        case 8:
            if (stereo)
                return AL_FORMAT_STEREO8;
            else
                return AL_FORMAT_MONO8;
        default:
            return -1;
    }
}
