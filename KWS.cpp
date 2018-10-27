
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
                       Dataset &DataSet,
                       FeatureExtracting &featureExtracting,
                       float *inputSignal,
                       int inputSignalLength,
                       int firstBufferIndicator,
                       int lastBufferIndicator)
{
    // Feature extraction phase for input buffer
    float *Feat_Buffer;

    int Status;

    int TmpBuff_Size = featureExtracting.PrevBuff_Size+inputSignalLength;

    int Num_Frames = 1+((TmpBuff_Size-FeatureExtracting::FrameLength)/FeatureExtracting::FrameShift);

    int Temp = FeatureExtracting::Feature_Dim*3;

    Feat_Buffer = new float[Num_Frames*Temp];

    float *Wav_Buffer_Temp;
    Wav_Buffer_Temp = new float[TmpBuff_Size];
    memcpy(&Wav_Buffer_Temp[0], featureExtracting.Prev_Wav_samples, sizeof(float)*featureExtracting.PrevBuff_Size);
    memcpy(&Wav_Buffer_Temp[featureExtracting.PrevBuff_Size], inputSignal, sizeof(float)*inputSignalLength);

    Status = featureExtracting.MFCC_Extractor(Wav_Buffer_Temp, TmpBuff_Size, Feat_Buffer);

    FeatureExtracting::Total_Num_Frames += Status-3-3; //the number 3 is set because of the delta coefficients which are computer across 3 frames on the left& right side of the central frame
    int Num_Prev_remained_samples = (TmpBuff_Size-featureExtracting.FrameLength)%featureExtracting.FrameShift;
    featureExtracting.PrevBuff_Size = (FeatureExtracting::FrameLength+(5*FeatureExtracting::FrameShift))+Num_Prev_remained_samples;
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
        Dataset::scores = new infra::matrix;
        Dataset::scores->resize(DataSet.Seg_Length,DataSet.Num_Phns);

//        if (Dataset::distances != NULL)
//            delete Dataset::distances;
        Dataset::distances = new infra::matrix;
        Dataset::distances->resize(DataSet.Seg_Length,DataSet.Dist_dim);

        Num_Frames -= 9;
        DataSet.Get_Frame(&Feat_Buffer[6*Temp], Num_Frames,  firstBufferIndicator, lastBufferIndicator);
    }
    else if (lastBufferIndicator)
    {
        Dataset::distances_index = Dataset::distances_index-3;
        Num_Frames -= 3;
        DataSet.Get_Frame(&Feat_Buffer[3*Temp], Num_Frames,  firstBufferIndicator, lastBufferIndicator);
    }
    else
    {
        Dataset::distances_index = Dataset::distances_index-3;
        Num_Frames -= 6;
        DataSet.Get_Frame(&Feat_Buffer[3*Temp], Num_Frames,  firstBufferIndicator, lastBufferIndicator);
    }
    delete[] Feat_Buffer;

    // Calculating Phoneme Classifier Score for new Input Buffer
    phonemeClassifier.predict(DataSet);

    // Calculating Distance Matrix for new Input Buffer
    phonemeClassifier.ceps_dist(DataSet);


    Dataset::Start_Speech = firstBufferIndicator;
    Dataset::End_Speech = lastBufferIndicator;

    return 1;
}

/************************************************************************/
int keywordSpotter(PhonemeSequence &phonemeSequence, KeywordClassifier &keywordClassifier)
{
    int Temp = Dataset::distances_index;
    while (1)
    {
        int Start_Buff = Dataset::Start_Speech;
        int Final_Buff = Dataset::End_Speech;
        Temp = Dataset::distances_index;
        if (keywordClassifier.Prev_Buff_Analyzed<Temp)
        {
            // predict
            keywordClassifier.alignKeyword(phonemeSequence, Temp, Start_Buff, Final_Buff);
            keywordClassifier.Prev_Buff_Analyzed = Temp;
        }
        if (Final_Buff==1)
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

    Dataset *Data;
    Data = new Dataset(FeatureExtracting::Feature_Dim*3, phonemeSequence->numOfPhonemes, phonemeClassifier.last_s);

    FeatureExtracting * Feat;
    Feat = new FeatureExtracting();


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

        extractFeatures(phonemeClassifier, *Data, *Feat, temp_input, featuresBufferSize, firstBufferIndicator, lastBufferIndicator);

        firstBufferIndicator = 0;
    }

    // Start KWS for explore input file stream
    keywordSpotter(*phonemeSequence, *keywordClassifier);

    delete Data;
    delete phonemeSequence;

    t2 = clock();
    float diff=((float)t2 - (float)t1);

    return diff;
}
