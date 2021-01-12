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

## Adding OpenAL

Since there is no standard audio library for C++, we will need to add an external library in order to play audio from the command line. In this case we will be using OpenAL.

OpenAL is an SDK primarily for Game Audio and simple acoustics modelling. However, it can be used to just play a `wav` file or indeed an array of type `float`. Instructions for installation and setup are given below

### Visual Studio

[Download the OpenAL SDK and Windows Installer](https://www.OpenAL.org/downloads/). Unzip and run both.

- **Create a new console project**

![](gifs/MakeVSproject.gif)

- **Add includes folder for headers**

![](gifs/OpenALAddHeaderInclude.gif)

- **Add OpenAL library in linker**

![](gifs/OpenALAddLibs.gif)

- **Add `OpenAL32.lib` to linker input**

![](gifs/OpenALInputLib.gif)

You should now be able to include the `<al.h>` and `<alc.h>` headers

- **Add Existing Files**

![](gifs/AddExisting.gif)

Now press <kbd>F7</kbd> to build

### Xcode

- **Create a new project**

![](gifs/XcodeCli.gif)

- **Add OpenAL, CoreFoundation and AudioToolbox frameworks to target**

![](gifs/XcodeAddOpenAL.gif)

- **Add Existing Files**

![](gifs/AddExistingXcode.gif)

Now press <kbd>⌘</kbd> + <kbd>B</kbd> to build


### TroubleShooting

`linker command failed with exit code 1 (use -v to see invocation)`

**What does this mean?**

- This typically means you have included a header but not the framework or the `cpp` file. Have a look at your project again and make sure you have included all the required files and frameworks.
