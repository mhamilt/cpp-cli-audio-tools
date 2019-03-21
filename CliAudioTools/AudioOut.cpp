//==============================================================================
#include "AudioOut.hpp"
//==============================================================================

AudioOut::AudioOut()
{
    
}

AudioOut::~AudioOut()
{
    
}
//==============================================================================

void AudioOut::listAudioDevices()
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

void AudioOut::playFile(const char *inputfname)
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
    
    ALsizei numberOfBytes, samplingRate;
    ALvoid *data = wavReadWrite.readRawData(inputfname, &numberOfBytes, &samplingRate);
    
    if (!data)
    {
        std::cout << "LOAD ERROR: check the file name is correct" << '\n';
    }
    
    alBufferData(buffer, getAlFormat(), data, numberOfBytes, samplingRate);
    testError("Fail at alBufferData");
    
    alSourcei(source, AL_BUFFER, buffer);
    testError("buffer binding");
    
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

void AudioOut::printMidiHeader(const char *filename)
{
    
}
//==============================================================================

void AudioOut::testError(const char *message)
{
    ALCenum error = alGetError();
    if (error != AL_NO_ERROR)
    {
        fprintf(stderr, message, "\n");
    }
}

ALenum AudioOut::getAlFormat()
{
    short channels = wavReadWrite.getFileChannelNumber();
    short samples = wavReadWrite.getFileBitDepth();
    bool stereo = (channels > 1);
    
    switch (samples)
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
