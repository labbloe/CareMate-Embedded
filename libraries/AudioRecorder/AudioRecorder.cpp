/*
    AudioRecorder.cpp
*/

#include "AudioRecorder.h"

void recordAudio(const int record_time, const String filename)
{
    int waveDataSize = record_time * 88000;
    byte header[headerSize];
    char communicationData[numCommunicationData];
    char partWavData[numPartWavData];
    File file;
    Serial.println("Begining audio capture");

    CreateWavHeader(header, waveDataSize);
    SD.remove(filename);
    file = SD.open(filename, FILE_WRITE);
    
    if (!file) 
        return;

    file.write(header, headerSize);
    I2S_Init(I2S_MODE, I2S_BITS_PER_SAMPLE_32BIT);
    for (int j = 0; j < waveDataSize/numPartWavData; ++j) 
    {
        I2S_Read(communicationData, numCommunicationData);
        for (int i = 0; i < numCommunicationData/8; ++i) 
        {
            partWavData[2*i] = communicationData[8*i + 2];
            partWavData[2*i + 1] = communicationData[8*i + 3];
        }
        file.write((const byte*)partWavData, numPartWavData);
    }
    file.close();
    Serial.println("Finished writing audio data to " + filename);
}