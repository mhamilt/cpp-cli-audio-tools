//
//  main.cpp
//  CliAudioToolExample
//
//  Created by mhamilt7 on 21/03/2019.
//  Copyright © 2019 mhamilt7. All rights reserved.
//

#include <iostream>
#include "../../../src/MattsAudioTools.h"


int main(int argc, const char * argv[])
{
    //--------------------------------------------------------------------
    AudioPlayerOpenAL ap;
    //--------------------------------------------------------------------
    // Create sine wave, then play
    int channels = 2;
    int fs = 44100;
//    int millis = 1000;
    int n = 44100 * 5;
    int bitDeph = 16; // 8 or 16
    float fHz = atof(argv[1]);
    float *sineWave = new float[n * channels];
    
    float radsPerSample = 440.f * 2.f * 3.1415926536f / float(fs);
    
    for (int i = 0; i < n; ++i)
    {
        sineWave[i] = 0;
        for (int channel = 0; channel < channels; ++channel)
        {
            int frame = i*channels + channel;
            switch (channel)
            {
                case 0:
                    sineWave[frame] += sin(float(i) * radsPerSample);
                    break;
                case 1:
                    sineWave[frame] += sin(float(i) * radsPerSample * 1.5);
                    break;
            }
            sineWave[frame] *= 0.9;
            
        }
    }
    
    ap.playAudioData(sineWave, n, channels, fs, bitDeph);
    //--------------------------------------------------------------------
    delete [] sineWave;
    return 0;
}
