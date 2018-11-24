
/************************************************************************
Project:  Discriminative data minner
Module:   --
Purpose:  Main entry point
Date:     --
Function List:
main - Main entry point

**************************** INCLUDE FILES *****************************/
//#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <time.h>
#include <unistd.h>

#ifndef KWS_EXPORT_CPP_CLASS_DECL
#define KWS_EXPORT_CPP_CLASS_DECL __declspec(dllexport)
#define KWS_IMPORT_CPP_CLASS_DECL __declspec(dllimport)
#endif

#ifdef KWS_EXPORTS
#define KWS_API_CLASS KWS_EXPORT_CPP_CLASS_DECL
#else
#define KWS_API_CLASS KWS_IMPORT_CPP_CLASS_DECL
#endif

#include "cmd_line.h"
#include "Classifier_KWS.h"
#include "Dataset_KWS.h"
#include "MFCC_ETSI.h"
#include "KWS.h"
#include <QDebug>

using namespace std;

/************************************************************************
Function:     Initialize

Description:  Initializes the fixed parameters for data minning
Inputs:       None
Output:       void.
Comments:     none.
***********************************************************************/
void Initialize(string Feature_ConfigFile, string mfcc_stats_file)
{
    // Load feature extraction configuration
    FeatureExtracting::loadFeatureExtractionParameters(Feature_ConfigFile, mfcc_stats_file);
}

/************************************************************************
Function:     Feature_Extractor

Description:  Extracts features
Inputs:       Input Wave Buffer
Output:       int - always 0.
Comments:     none.
***********************************************************************/
int extractFeatures(PhonemeClassifier &phonemeClassifier,
                       Dataset &dataset,
                       FeatureExtracting &featureExtracting,
                       float *inputSignal,
                       int inputSignalLength,
                       int firstBufferIndicator,
                       int lastBufferIndicator)
{
    // Feature extraction phase for input buffer
    float *featuresBuffer;

    int Status;

    int temporaryBufferSize = featureExtracting.PrevBuff_Size + inputSignalLength;

    int Num_Frames = 1+((temporaryBufferSize - featureExtracting.FrameLength) / featureExtracting.FrameShift);

    int Temp = featureExtracting.Feature_Dim * 3;

    featuresBuffer = new float[Num_Frames * Temp];

    float *Wav_Buffer_Temp;
    Wav_Buffer_Temp = new float[temporaryBufferSize];
    memcpy(&Wav_Buffer_Temp[0], featureExtracting.Prev_Wav_samples, sizeof(float)*featureExtracting.PrevBuff_Size);
    memcpy(&Wav_Buffer_Temp[featureExtracting.PrevBuff_Size], inputSignal, sizeof(float)*inputSignalLength);

    Status = featureExtracting.MFCC_Extractor(Wav_Buffer_Temp, temporaryBufferSize, featuresBuffer);
    featureExtracting.Total_Num_Frames += Status - 3 - 3; //the number 3 is set because of the delta coefficients which are computer across 3 frames on the left& right side of the central frame

    int Num_Prev_remained_samples = (temporaryBufferSize-featureExtracting.FrameLength)%featureExtracting.FrameShift;
    featureExtracting.PrevBuff_Size = (featureExtracting.FrameLength + (5 * featureExtracting.FrameShift))+Num_Prev_remained_samples;

    int ind_Prev_samples = inputSignalLength-featureExtracting.PrevBuff_Size;

    delete []featureExtracting.Prev_Wav_samples;

    featureExtracting.Prev_Wav_samples = new float[featureExtracting.PrevBuff_Size];
    memcpy(featureExtracting.Prev_Wav_samples, &inputSignal[ind_Prev_samples], sizeof(float)*featureExtracting.PrevBuff_Size);

    delete []Wav_Buffer_Temp;

    // Writing extracted features in required data arrays of the class  FeatureSegments_KWS
    if (firstBufferIndicator)
    {
//        if (Dataset::scores != NULL)
//            delete Dataset::scores;
        dataset.scores = new infra::matrix;
        dataset.scores->resize(dataset.Seg_Length,dataset.Num_Phns);

//        if (Dataset::distances != NULL)
//            delete Dataset::distances;
        dataset.distances = new infra::matrix;
        dataset.distances->resize(dataset.Seg_Length,dataset.Dist_dim);

        Num_Frames -= 9;
        dataset.Get_Frame(&featuresBuffer[6 * Temp], Num_Frames,  firstBufferIndicator, lastBufferIndicator);
    }
    else if (lastBufferIndicator)
    {
        dataset.distances_index = dataset.distances_index - 3;
        Num_Frames -= 3;
        dataset.Get_Frame(&featuresBuffer[3 * Temp], Num_Frames,  firstBufferIndicator, lastBufferIndicator);
    }
    else
    {
        dataset.distances_index = dataset.distances_index - 3;
        Num_Frames -= 6;
        dataset.Get_Frame(&featuresBuffer[3 * Temp], Num_Frames,  firstBufferIndicator, lastBufferIndicator);
    }
    delete[] featuresBuffer;

    // Calculating Phoneme Classifier Score for new Input Buffer
    phonemeClassifier.predict(dataset);

    // Calculating Distance Matrix for new Input Buffer
    phonemeClassifier.cepestralsDistance(dataset);

    dataset.startOfSpeechIndicator = firstBufferIndicator;
    dataset.endOfSpeechIndicator = lastBufferIndicator;

    return 1;
}

/************************************************************************/
int keywordSpotter(PhonemeSequence &phonemeSequence, KeywordClassifier &keywordClassifier, Dataset &dataset)
{
    while (1)
    {
        if (keywordClassifier.Prev_Buff_Analyzed < dataset.distances_index)
        {
            // predict
            keywordClassifier.alignKeyword(phonemeSequence, dataset);
            keywordClassifier.Prev_Buff_Analyzed = dataset.distances_index;
        }
        if (dataset.endOfSpeechIndicator == 1)
            return 0;
    }

    return -1;
}

/************************************************************************
Function:     Vajegan

Description:  Main entry point
Inputs:       int argc, char *argv[] - main input params
Output:       int - always 0.
Comments:     none.
***********************************************************************/
void Vajegan(float *inputSignal,
              int inputSignalLength,
              string word,
              KeywordClassifier *keywordClassifier,
              PhonemeClassifier phonemeClassifier)
{  
    // Load phnems map
    string silenceSymbol = "sil";
    PhonemeSequence *phonemeSequence;
    phonemeSequence = new PhonemeSequence(word, word.length());
    phonemeSequence->setSilenceSymbol(silenceSymbol);

    FeatureExtracting *featurExtracting;
    featurExtracting = new FeatureExtracting();

    Dataset *dataset;
    dataset = new Dataset(featurExtracting->Feature_Dim * 3, phonemeSequence->numOfPhonemes, phonemeClassifier.last_s);

    int featureExtractingBufferSize, fixedBufferSize = 4096;
    int remainedSamples = inputSignalLength;
    int sampleCounter = 0;

    float featureExtractingBuffer[fixedBufferSize];

    int firstBufferIndicator = 1;
    int lastBufferIndicator = 0;

    while(remainedSamples > 0)
    {
        if (remainedSamples >= fixedBufferSize)
            featureExtractingBufferSize = fixedBufferSize;
        else
        {
            featureExtractingBufferSize = remainedSamples;
            lastBufferIndicator = 1;
        }

        remainedSamples -= featureExtractingBufferSize;

        for (int i=0; i < featureExtractingBufferSize; i++)
        {
            featureExtractingBuffer[i] = inputSignal[sampleCounter++];
        }

        extractFeatures(phonemeClassifier, *dataset, *featurExtracting, featureExtractingBuffer, featureExtractingBufferSize, firstBufferIndicator, lastBufferIndicator);

        firstBufferIndicator = 0;
    }

    // Start KWS for explore input file stream
    keywordSpotter(*phonemeSequence, *keywordClassifier, *dataset);

    delete dataset;
    delete phonemeSequence;
    delete featurExtracting;
}
