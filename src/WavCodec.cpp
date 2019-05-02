//
//  AudioWavFileReader.cpp
//  FDTD_C_Plate
//
//  Created by admin on 15/12/2017.
//  Copyright © 2017 admin. All rights reserved.
//
#ifndef WavCodec_hpp // WavCodec WavCodec
#include "WavCodec.hpp"

//==============================================================================
WavCodec::~WavCodec(){}
//==============================================================================
int WavCodec::getSampleRate(){return wavReadFileSampRate;};
//==============================================================================
void WavCodec::printWavHeader(const char *filename)
{
    FILE *f;
    waveFormatHeader hdr;
    f = fopen(filename, "rb");
    
    if (!f)
    {
        printf("NO FILE FOUND\n");
    }
    else
    {
        fread(&hdr, 1, sizeof(hdr), f);
        
        printf("--- .WAV HEADER --- \n\n");
        printf("Chunk ID        : %.4s\n", hdr.chunkID);
        printf("Chunk Size      : %u\n", hdr.chunkSize);
        printf("Format          : %.4s\n", hdr.format);
        printf("Sub-Chunk 1     : %.4s\n", hdr.subChunk1ID);
        printf("Sub-Chunk 1 size: %d\n", hdr.subChunk1Size);
        printf("Audio Format    : %d\n", hdr.audioFormat);
        printf("Num Channels    : %d\n", hdr.numChannels);
        printf("Sample Rate     : %u\n", hdr.sampleRate);
        printf("Byte Rate       : %u\n", hdr.byteRate);
        printf("Block Align     : %u\n", hdr.blockAlign);
        printf("Bits Per Samp   : %u\n", hdr.bitsPerSample);
        printf("Sub-Chunk 2     : %.4s\n", hdr.subChunk2ID);
        printf("Sub-Chunk 2 size: %u\n", hdr.subChunk2Size);
        //        printf("SampsPerChan    : %u\n", hdr.subChunk2Size*8/(hdr.numChannels*hdr.bitsPerSample));
        printf("\n");
        fclose(f);
    }
}
//==============================================================================

//	virtual const char* cmdLineSetFilename(const char *cmdLineArgV[]) = 0;

//==============================================================================

void WavCodec::writeWavSS(double **audioData, const char outputFile[], int numberOfFrames, double sampleRate)
{
    normaliseStereoBuffer(audioData[0], audioData[1] ,numberOfFrames);
    FILE * file;
    file = fopen(outputFile, "w");
    if (!file)
    {
        printf("Could not open file to write: check file path\n");
        return;
    }
    else
    {
        stereo16bitWaveHeaderForLength(numberOfFrames,sampleRate);
        writeWaveHeaderToFile(file);
        int16_t sdata;
        //	const double amp = 32000.0;
        const double amp = 32767.0; // absolute peak value of 16-bit PCM
        
        for(int i = 0; i < numberOfFrames; ++i)
        {
            sdata = (int16_t)(audioData[0][i]*amp);
            fwrite(&sdata, sizeof(int16_t), 1, file); //left channel
            sdata = (int16_t)(audioData[1][i]*amp);
            fwrite(&sdata, sizeof(int16_t), 1, file); //right channel
        }
        printf("%d samples written to %s\n", numberOfFrames*2,outputFile);
    }
    fclose(file);
    
}

//==============================================================================
bool WavCodec::checkHeader(waveFormatHeader fileHeader)
{
    
    if ((strncmp(&fileHeader.chunkID[0], "RIFF", 4))	 ||
        (strncmp(&fileHeader.format[0],  "WAVE", 4))     ||
        (strncmp(&fileHeader.subChunk1ID[0], "fmt ", 4)) ||
        (strncmp(&fileHeader.subChunk2ID[0], "data", 4)))
    {
        return false;
    }
    else
    {
        return true;
    }
}
//==============================================================================

double* WavCodec::readWav(const char *filename, int *sampsPerChan, int *sampleRate)
{
    FILE *f;
    f = fopen(filename, "rb");
    if (!f){return NULL;}
    fread(&wavReadFileHeader, 1, sizeof(wavReadFileHeader), f);
    
    if(!checkHeader(wavReadFileHeader))
    {
        fclose(f);
        printf("NOT A WAV FILE\n");
        return NULL;
    }
    
    const int totalSamples = wavReadFileHeader.subChunk2Size * 8 /(wavReadFileHeader.bitsPerSample);
    *sampsPerChan = totalSamples / (wavReadFileHeader.numChannels);
    printf("Length: %d\tSamples: %d \n",totalSamples,*sampsPerChan);
    
    double *data = new double[*sampsPerChan];
    
    parseWavMonoFile(data, f);
    fclose(f);
    printf("%d samples read from %s\n",*sampsPerChan,filename);
    *sampleRate = wavReadFileHeader.sampleRate;
    return data;
}

//==============================================================================
bool WavCodec::parseWavMonoFile(double* data, FILE *f)
{
    const int byteNum = wavReadFileHeader.bitsPerSample/8;
    uint8_t *const buf = new uint8_t[byteNum];
    const double wavBitScale = 1/pow(2,31);
    const double wav8BitScale = 2/pow(2,wavReadFileHeader.bitsPerSample);
    const int numberOfFrames = wavReadFileHeader.subChunk2Size * 8 /(wavReadFileHeader.bitsPerSample * wavReadFileHeader.numChannels);
    const int byteOffset = (4-byteNum);
    
    for (int sample = 0; sample < numberOfFrames; ++sample)
    {
        for (int channel = 0; channel < wavReadFileHeader.numChannels; ++channel)
        {
            int32_t data_in_channel = 0;
            size_t readCheck = fread(buf, byteNum, 1, f);
            if(readCheck == 1)
            {
                if (channel==0)
                {
                    if(byteNum==1)
                    {
                        data[sample] = ((double)buf[0]-127.5) * wav8BitScale;
                    }
                    else
                    {
                        for(int k = 0; k < byteNum; ++k)
                        {
                            data_in_channel |= (buf[k] << (k+byteOffset)*8);
                        }
                        data[sample] = ((double)data_in_channel) * wavBitScale;
                    }
                }
            }
            else
            {
                printf("FAILED FILE READ\n");
                return false;
            }
        }
    }
    
    delete[] buf;
    return true;
}

//==============================================================================
bool WavCodec::parseWavFile(double** data, FILE *f)
{
    const int byteNum = wavReadFileHeader.bitsPerSample/8;
    uint8_t *const buf = new uint8_t[byteNum];
    const double wavBitScale = 1/pow(2,31);
    const double wav8BitScale = 2/pow(2,wavReadFileHeader.bitsPerSample);
    const int numberOfFrames = wavReadFileHeader.subChunk2Size * 8 /(wavReadFileHeader.bitsPerSample * wavReadFileHeader.numChannels);
    const int byteOffset = (4-byteNum);
    
    for (int sample = 0; sample < numberOfFrames; ++sample)
    {
        for (int channel = 0; channel < wavReadFileHeader.numChannels; ++channel)
        {
            int32_t data_in_channel = 0;
            size_t readCheck = fread(buf, byteNum, 1, f);
            if(readCheck == 1)
            {
                if(byteNum==1)
                {
                    data[channel][sample] = ((double)buf[0]-127.5) * wav8BitScale;
                }
                else
                {
                    for(int k = 0; k < byteNum; ++k)
                    {
                        data_in_channel |= (buf[k] << (k+byteOffset)*8);
                    }
                    
                    data[channel][sample] = ((double)data_in_channel) * wavBitScale;
                }
            }
            else
            {
                printf("FAILED FILE READ\n");
                return false;
            }
        }
    }
    delete[] buf;
    return true;
}

//==============================================================================

double** WavCodec::readStereoWav(const char *filename, int *sampsPerChan, int *sampleRate)
{
    FILE *f;
    f = fopen(filename, "rb");
    if (!f)
    {
        return NULL;
    }
    
    fread(&wavReadFileHeader, sizeof(wavReadFileHeader), 1 , f);
    
    if(!checkHeader(wavReadFileHeader))
    {
        fclose(f);
        printf("NOT A WAV FILE\n");
        return NULL;
    }
    
    if ((wavReadFileHeader.numChannels != 2))
    {
        fclose(f);
        printf("NOT A STEREO FILE\n");
        return NULL;
    }
    
    const int totalSamples = wavReadFileHeader.subChunk2Size * 8 /(wavReadFileHeader.bitsPerSample);
    *sampsPerChan	 = totalSamples/(wavReadFileHeader.numChannels);
    double** data = new double*[wavReadFileHeader.numChannels];
    if (!data)
    {
        return NULL;
    }
    
    for(int i = 0; i < wavReadFileHeader.numChannels; ++i)
    {
        data[i] = new double[*sampsPerChan];
    }
    
    parseWavFile(data, f);
    fclose(f);
    printf("%d samples read from %s\n",totalSamples,filename);
    *sampleRate = wavReadFileHeader.sampleRate;
    return data;
}
//==============================================================================

void WavCodec::setBasicHeader()
{
    //    wavWriteFileHeader = *new waveFormatHeader;
    memcpy(wavWriteFileHeader.chunkID, &"RIFF", 4);
    memcpy(wavWriteFileHeader.format, &"WAVE", 4);
    memcpy(wavWriteFileHeader.subChunk1ID, &"fmt ", 4); //notice the space at the end!
    wavWriteFileHeader.subChunk1Size = 16;
}
//==============================================================================

void WavCodec::stereo16bitWaveHeader(double sampleRate)
{
    setBasicHeader();
    wavWriteFileHeader.audioFormat = 1;
    wavWriteFileHeader.numChannels = 2;
    wavWriteFileHeader.sampleRate = (uint32_t)sampleRate;
    wavWriteFileHeader.bitsPerSample = 16;
    wavWriteFileHeader.byteRate = wavWriteFileHeader.numChannels * wavWriteFileHeader.sampleRate * wavWriteFileHeader.bitsPerSample/8;
    wavWriteFileHeader.blockAlign = wavWriteFileHeader.numChannels * wavWriteFileHeader.bitsPerSample/8;
    memcpy(wavWriteFileHeader.subChunk2ID, &"data", 4);
}
//==============================================================================
void WavCodec::mono16bitWaveHeader(double sampleRate)
{
    setBasicHeader();
    wavWriteFileHeader.audioFormat = 1;
    wavWriteFileHeader.numChannels = 1;
    wavWriteFileHeader.sampleRate = (uint32_t)sampleRate;
    wavWriteFileHeader.bitsPerSample = 16;
    wavWriteFileHeader.byteRate = wavWriteFileHeader.numChannels * wavWriteFileHeader.sampleRate * wavWriteFileHeader.bitsPerSample/8;
    wavWriteFileHeader.blockAlign = wavWriteFileHeader.numChannels * wavWriteFileHeader.bitsPerSample/8;
    memcpy(wavWriteFileHeader.subChunk2ID, &"data", 4);
}
//==============================================================================
void WavCodec::setLengthForWaveFormatHeader(size_t numberOfFrames)
{
    wavWriteFileHeader.subChunk2Size = (uint32_t)numberOfFrames * wavWriteFileHeader.numChannels * wavWriteFileHeader.bitsPerSample/8;
    wavWriteFileHeader.chunkSize = 36 + wavWriteFileHeader.subChunk2Size;
}
//==============================================================================
void WavCodec::stereo16bitWaveHeaderForLength(size_t numberOfFrames,double sampleRate)
{
    stereo16bitWaveHeader(sampleRate);
    setLengthForWaveFormatHeader(numberOfFrames);
}
//==============================================================================
void WavCodec::mono16bitWaveHeaderForLength(size_t numberOfFrames,double sampleRate)
{
    mono16bitWaveHeader(sampleRate);
    setLengthForWaveFormatHeader(numberOfFrames);
}
//==============================================================================

void WavCodec::normaliseBuffer(double *audioData, int numberOfFrames)
{
    double temp;
    double maxy = 0.0; // Find max abs sample
    
    for(int n = 0; n < numberOfFrames; ++n)
    {
        if(fabs(audioData[n])>maxy) maxy = fabs(audioData[n]);
    }
    
    // Normalise
    if(maxy > 0.00001)
    {
        for(int n = 0; n < numberOfFrames; ++n)
        {
            temp     = audioData[n];
            audioData[n] = temp/maxy;
        }
    }
    
    // Smooth last 500 samples
    if(numberOfFrames > 501)
    {
        double inc  = 1.0/500.0;
        double ramp = 1.0;
        for(int n = numberOfFrames-501; n < numberOfFrames; ++n)
        {
            audioData[n] *= ramp;
            if(ramp>0)
                ramp -= inc;
        }
    }
    
    printf("Normalised by : %.5f\n", maxy);
}

//==============================================================================

void WavCodec::normaliseStereoBuffer(double *audioL, double *audioR, int numberOfFrames)
{
    double temp;
    double maxy = 0.0; // Find max abs sample
    
    for(int n = 0; n < numberOfFrames; ++n)
    {
        if(fabs(audioL[n])>maxy) maxy = fabs(audioL[n]);
        if(fabs(audioR[n])>maxy) maxy = fabs(audioR[n]);
        
    }
    
    // Normalise
    if(maxy > 0.00001)
    {
        for(int n=0;n<numberOfFrames;++n)
        {
            temp      = audioL[n];
            audioL[n]  = temp/maxy;
            temp      = audioR[n];
            audioR[n] = temp/maxy;
        }
    }
    
    // Smooth last 500 samples
    if(numberOfFrames>501)
    {
        double inc  = 1.0/500.0;
        double ramp = 1.0;
        for(int n=numberOfFrames-501;n<numberOfFrames;++n)
        {
            audioL[n] *= ramp;
            audioR[n] *= ramp;
            if(ramp>0) ramp-=inc;
        }
    }
    
    printf("Normalised by : %.5f\n", maxy);
}
//==============================================================================

size_t WavCodec::writeWaveHeaderToFile(FILE * file)
{
    return fwrite(&wavWriteFileHeader, sizeof(waveFormatHeader), 1, file);
}

void WavCodec::writeWavMS(double* audio,const char outputFile[], int numberOfFrames, double sampleRate)
{
    normaliseBuffer(audio ,numberOfFrames);
    
    FILE * file;
    file = fopen(outputFile, "w");
    if (!file)
    {
        printf("Could not open file to write: check file path\n");
        return;
    }
    else
    {
        mono16bitWaveHeaderForLength(numberOfFrames,sampleRate);
        writeWaveHeaderToFile(file);
        int16_t sdata;
        const double amp = 32000.0;
        
        for(int i=0;i<numberOfFrames;++i)
        {
            sdata = (int16_t)(audio[i]*amp);  //set sdata to PCM 16-bit
            fwrite(&sdata, sizeof(int16_t), 1, file);
        }
        printf("%d samples written to %s\n", numberOfFrames,outputFile);
    }
    fclose(file);
}

//==============================================================================
double** WavCodec::whiteNoise(int sampsPerChan, int sampleRate)
{
    const double lo = -1.;
    const double hi =  1.;
    double** data = new double*[2];
    
    for(int i = 0; i < 2; ++i)
    {
        data[i] = new double[sampsPerChan];
    }
    
    for(int i = 0; i < sampsPerChan; ++i)
    {
        data[0][i] = lo + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(hi-lo)));
        data[1][i] = lo + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(hi-lo)));
    }
    return data;
}
//==============================================================================
char* WavCodec::readRawData(const char *filename, int *dataSize, int *sampleRate)
{
    FILE *f = fopen(filename, "rb");
    if (!f)
    {
        return nullptr;
    }
    
    fread(&wavReadFileHeader, 1, sizeof(wavReadFileHeader), f);
    
    if(!checkHeader(wavReadFileHeader))
    {
        fclose(f);
        printf("NOT A WAV FILE\n");
        return nullptr;
    }
    
    const int totalSamples = wavReadFileHeader.subChunk2Size * 8 /(wavReadFileHeader.bitsPerSample);
    *dataSize = wavReadFileHeader.subChunk2Size;
    printf("Number of Sample Size: %d\tData: %d KB \n",totalSamples, *dataSize/1000);
    char *data = new char[*dataSize];
    
    for (int sample = 0; sample < *dataSize; ++sample)
    {
        size_t readCheck = fread(&data[sample], 1, 1, f);
    }
    
    fclose(f);
    *sampleRate = wavReadFileHeader.sampleRate;
    return data;
}
//==============================================================================
uint16_t WavCodec::getFileSampleRate()
{
    return wavReadFileHeader.sampleRate;
}
uint16_t WavCodec::getFileChannelNumber()
{
    return wavReadFileHeader.numChannels;
}

uint16_t WavCodec::getFileBitDepth()
{
    return wavReadFileHeader.bitsPerSample;
}

#endif /* AudioWavFileReader_hpp */
//EOF
