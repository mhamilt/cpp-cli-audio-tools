// Simple Blocking WASAPI Playback

#include <iostream>
#include "../../../src/AudioPlayerWindows.hpp"

int main ()
{
    AudioPlayerWindows ap = AudioPlayerWindows();
    std::cout << "Sample Rate: " << ap.getSystemSampleRate() << '\n';

    float durationSeconds = 2.0;
    float sampleRate = ap.getSystemSampleRate ();
    unsigned long durationSamples = ap.getSystemSampleRate () * durationSeconds;
    float* sineWave = new float[durationSamples];
    float frequency = 440;

    float radsPerSec = 2 * 3.1415926536 * frequency / sampleRate;

    for ( unsigned long i = 0; i < durationSamples; i++ )
    {
        float sampleValue = sin (radsPerSec * (float) i) ;
        sineWave[i] = sampleValue;
    }

    ap.playAudioData (sineWave, durationSamples, 1);

    delete[] sineWave;
    return 0;
}
