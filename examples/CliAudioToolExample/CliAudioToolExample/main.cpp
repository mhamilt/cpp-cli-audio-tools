//
//  main.cpp
//  CliAudioToolExample
//
//  Created by mhamilt7 on 21/03/2019.
//  Copyright © 2019 mhamilt7. All rights reserved.
//

#include <iostream>
#include "../../../CliAudioTools/CliAudioTools.h"

int main(int argc, const char * argv[])
{
    const char *filename = "/Users/mhamilt7/Music/WavFiles/Godin6.wav";
    
    AudioOut ap;
    ap.playFile(filename);
    
    AudioPlayerOsX apOsX;
    apOsX.playFile(filename);
    return 0;
}
