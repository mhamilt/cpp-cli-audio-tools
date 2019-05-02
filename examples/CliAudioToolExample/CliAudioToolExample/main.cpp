//
//  main.cpp
//  CliAudioToolExample
//
//  Created by mhamilt7 on 21/03/2019.
//  Copyright © 2019 mhamilt7. All rights reserved.
//

#include <iostream>
#include "../../../src/CliAudioTools.h"

int main(int argc, const char * argv[])
{
    //--------------------------------------------------------------------
    const char *wavFileName = "/path/to/file.wav";
    AudioPlayerOpenAL ap;
    ap.playFile(wavFileName);
    AudioPlayerOsX apOsX;
    apOsX.playFile(wavFileName);
    //--------------------------------------------------------------------
    // Load Audio Data from file then play
    WavCodec wavReader;
    int sampsPerChan, sampleRate;
    double *y = wavReader.readWav(wavFileName, &sampsPerChan, &sampleRate);
    float *x = new float[sampsPerChan];
    for (int i = 0 ; i < sampsPerChan; i++)
    {
        x[i] = (float) y[i];
    }
    ap.playAudioData(x, sampsPerChan, 1, sampleRate, 16);
    //--------------------------------------------------------------------
    // Creat sine wave, then play
    int n = 44100;
    int channels = 1;
    int fs = 44100;
    int bitDeph = 16;
    float *sineWave = new float[n];
    const float radPerSec = 440.f * 2.f * 3.1415926536f / float(fs);
    for (int i = 0; i < n; ++i)
    {
        sineWave[i] = sin(float(i) * radPerSec);
    }
    ap.playAudioData(sineWave, n, channels, fs, bitDeph);
    //--------------------------------------------------------------------
    return 0;
}
