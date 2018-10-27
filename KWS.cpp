
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
void Initialize(string KWS_Model_File, string KWS_Model_ConfigFile, double Treshold,
                int Loop_Step, string PhnClassi_Model_File, string PHN_StatsFile, string PHN_MapFile,
                string Feature_ConfigFile, string mfcc_stats_file)
{
//    // phoneme symbol to number mapping
//    std::string Temp = "sil";

//    // Load phnems map
//    PhonemeSequence_KWS::load_phoneme_map(PHN_MapFile, Temp);

    // Load KWS classifier
//    KeywordClassifier::loadKeywordClassifier(KWS_Model_File, KWS_Model_ConfigFile, Treshold, Loop_Step);

    // Load phonem states
//    KeywordClassifier::loadPhonemeStats(PHN_StatsFile);

    // Load phonems classifier
//    PhonemeClassifier::loadPhonemeClassifier(PhnClassi_Model_File, PhonemeSequence::numOfPhonemes);

    // Load feature extraction configuration
    FeatureExtracting::load_feature_extraction_parameters(Feature_ConfigFile, mfcc_stats_file);
}

/************************************************************************
Function:     Free_Classes

Description:  Freeing the arrays allocated for data minning
Inputs:       None
Output:       void.
Comments:     none.
***********************************************************************/
void Free_Classes(void)
{
    //DeleteCriticalSection(&ICriticalSection);


    std::cout << "Deleting Classifiers and Feature extracting arrays..." << std::endl;
//    PhonemeClassifier::deletePhonemeClassifier();
    std::cout << "Classifiers arrays Deleted." << std::endl;
    FeatureExtracting::delete_feature_extraction_parameters();
    std::cout << "Feature extracting arrays Deleted." << std::endl;
}

/************************************************************************
Function:     Feature_Extractor

Description:  Extracts features
Inputs:       Input Wave Buffer
Output:       int - always 0.
Comments:     none.
***********************************************************************/
int extractFeatures(PhonemeClassifier &phonemeClassifier,
                       Dataset &dataSet,
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
        dataSet.scores = new infra::matrix;
        dataSet.scores->resize(dataSet.Seg_Length,dataSet.Num_Phns);

//        if (Dataset::distances != NULL)
//            delete Dataset::distances;
        dataSet.distances = new infra::matrix;
        dataSet.distances->resize(dataSet.Seg_Length,dataSet.Dist_dim);

        Num_Frames -= 9;
        dataSet.Get_Frame(&featuresBuffer[6 * Temp], Num_Frames,  firstBufferIndicator, lastBufferIndicator);
    }
    else if (lastBufferIndicator)
    {
        dataSet.distances_index = dataSet.distances_index - 3;
        Num_Frames -= 3;
        dataSet.Get_Frame(&featuresBuffer[3 * Temp], Num_Frames,  firstBufferIndicator, lastBufferIndicator);
    }
    else
    {
        dataSet.distances_index = dataSet.distances_index - 3;
        Num_Frames -= 6;
        dataSet.Get_Frame(&featuresBuffer[3 * Temp], Num_Frames,  firstBufferIndicator, lastBufferIndicator);
    }
    delete[] featuresBuffer;

    // Calculating Phoneme Classifier Score for new Input Buffer
    phonemeClassifier.predict(dataSet);

    // Calculating Distance Matrix for new Input Buffer
    phonemeClassifier.ceps_dist(dataSet);

    dataSet.startOfSpeechIndicator = firstBufferIndicator;
    dataSet.endOfSpeechIndicator = lastBufferIndicator;

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
Function:     Get_result

Description:  Get Keyword Spotter Results.
Inputs:       classifier_KWS.
Output:       void.
Comments:     none.
***********************************************************************/
string Get_result(KeywordClassifier &classifier_KWS)
{
    ostringstream oss;
    string str="";
    int flag=0;
    int Siz = classifier_KWS.confidence.size();
//    cout<<"size fo result:"<<Siz<<endl;
    if (Siz==0)
        cout << "No result" << endl;
    else
        for (int i=0; i<Siz; i++)
        {
            oss<<classifier_KWS.confidence[i]<< endl<< classifier_KWS.timeAligns[i][0]/100.0 <<endl<<classifier_KWS.timeAligns[i][classifier_KWS.timeAligns[i].size()-1]/100.0 <<endl;
        }

    ofstream myfile1;
    myfile1.open ("res.txt");
    if (myfile1.is_open())
    {
        myfile1 << oss.str();
        myfile1.close();
    }
    else cout << "Unable to open file";
    return str;
}

/************************************************************************
Function:     Vajegan

Description:  Main entry point
Inputs:       int argc, char *argv[] - main input params
Output:       int - always 0.
Comments:     none.
***********************************************************************/
//extern "C" {

float Vajegan(float *inputSignal,
              int inputSignalLength,
              string word,
              KeywordClassifier *keywordClassifier,
              PhonemeClassifier phonemeClassifier)
{  
    clock_t t1,t2;
    t1=clock();

    // Load phnems map
    string silenceSymbol = "sil";
    PhonemeSequence *phonemeSequence;
    phonemeSequence = new PhonemeSequence(word, word.length());
    phonemeSequence->setSilenceSymbol(silenceSymbol);

    FeatureExtracting *featurExtracting;
    featurExtracting = new FeatureExtracting();

    Dataset *dataset;
    dataset = new Dataset(featurExtracting->Feature_Dim * 3, phonemeSequence->numOfPhonemes, phonemeClassifier.last_s);


    int featuresBufferSize, fixedBufferSize = 4096;
    int remainedSamples = inputSignalLength;
    int sampleCounter = 0;

    float temp_input[fixedBufferSize];

    int firstBufferIndicator = 1;
    int lastBufferIndicator = 0;

    while(remainedSamples > 0)
    {
        if (remainedSamples >= fixedBufferSize)
            featuresBufferSize = fixedBufferSize;
        else
        {
            featuresBufferSize = remainedSamples;
            lastBufferIndicator = 1;
        }

        remainedSamples -= featuresBufferSize;

        for (int i=0; i < featuresBufferSize; i++)
        {
            temp_input[i] = inputSignal[sampleCounter++];
        }

        extractFeatures(phonemeClassifier, *dataset, *featurExtracting, temp_input, featuresBufferSize, firstBufferIndicator, lastBufferIndicator);

        firstBufferIndicator = 0;
    }

    // Start KWS for explore input file stream
    keywordSpotter(*phonemeSequence, *keywordClassifier, *dataset);

    delete dataset;
    delete phonemeSequence;

    t2 = clock();
    float diff=((float)t2 - (float)t1);

    return diff;
}
