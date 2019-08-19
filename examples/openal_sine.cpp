//
//  sine.cpp
//  Play A sine with OpenAL
//

#include <iostream>
#include "../../../src/MattsAudioTools.h"

int main(int argc, const char * argv[])
{
    AudioPlayerOpenAL ap;
    ap.playFile(wavFileName);
    //--------------------------------------------------------------------
    // Create sine wave, then play
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
    delete[] sineWave;
    return 0;
}
