# cpp-cli-audio-tools
Command Line Tools for Audio Playback in C++

Load a wav file or create a signal in array and listen back.

## Contents

- [AudioPlaybackOpenAL]()
- [AudioPlaybackOsX]()
- [AudioWavFileReadWrite]()

## AudioPlaybackOpenAL

OpenAL powered audio play back, either from `.wav` or from audio generated in `C`. For the latter use the class method

```cpp
void playAudioData(float *audioData,
                       unsigned int numSamples,
                       uint8_t channelCount,
                       unsigned int samplingRate,
                       uint8_t bitDepth);
```

## AudioPlaybackOsX

AudioToolbox playback tool. This currently only works with play audio files.

## Dependencies

### AudioPlaybackOsX

- AudioToolbox.framework
- CoreFoundation.framework

### AudioPlaybackOpenAL

- OpenAL
