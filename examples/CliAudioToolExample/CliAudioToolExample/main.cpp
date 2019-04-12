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
    const char *wavFileName = "path/to/file.wav";
    
    AudioPlayerOpenAL ap;
    ap.playFile(wavFileName);
    
    AudioPlayerOsX apOsX;
    apOsX.playFile(wavFileName);
    return 0;
}
