/*
    AudioRecorder.h
*/

#ifndef AUDIO_RECORDER_H
#define AUDIO_RECORDER_H

#include "Arduino.h"
#include <FS.h>
#include "Wav.h"
#include "I2S.h"
#include <SD.h>

#define I2S_MODE I2S_MODE_ADC_BUILT_IN

//To change microphone pin edit I2S.cpp line 37
//Standard setting for CareMate is ADC1_CHANNEL7

const int headerSize = 44;
const int numCommunicationData = 8000;
const int numPartWavData = numCommunicationData/4;

void recordAudio(const int record_time, const String filename);

#endif /* AUDIO_RECORDER_H */