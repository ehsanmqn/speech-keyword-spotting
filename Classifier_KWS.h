#ifndef _CLASSIFIER_H
#define _CLASSIFIER_H

#ifndef KWS_EXPORT_CPP_CLASS_DECL
#define KWS_EXPORT_CPP_CLASS_DECL __declspec(dllexport)
#define KWS_IMPORT_CPP_CLASS_DECL __declspec(dllimport)
#endif

#ifdef KWS_EXPORTS
 #define KWS_API_CLASS KWS_EXPORT_CPP_CLASS_DECL
 #else
 #define KWS_API_CLASS KWS_IMPORT_CPP_CLASS_DECL
 #endif

/************************************************************************
Project:  Segmentation and Alignment
Module:   Classifier
Purpose:  Alignment discriminative algorithm

*************************** INCLUDE FILES ******************************/
#include <string>
#include <map>
#include "active_set.h"
#include "active_set.imp"
#include "kernels.h"
#include "kernels.imp"
#include "infra.h"
#include "Dataset_KWS.h"
#include <fstream>

class PhonemeClassifier
{
public:
    PhonemeClassifier();
    ~PhonemeClassifier();

    infra::vector ranks;
    int maxInputBufferLength;
    int instance_index;
    int numOfModels;
    int last_s;
    float **ModelsBuffer;

    void predict(Dataset& dataset);
    void ceps_dist(Dataset& dataset);
    void loadPhonemeClassifier(std::string &filename);
    void deletePhonemeClassifier(void);
    void averaging();
};

class KeywordClassifier
{
public:
    KeywordClassifier(int KW_Num_PHNs);
    ~KeywordClassifier();
	
    std::vector < std::vector <int> >  timeAligns;
    std::vector <float> confidence;
    infra::vector phonemeLengthMeanVector;
    infra::vector phonemeLengthStdVector;
    infra::vector weigths;
    double beta1;
    double beta2;
    double beta3;
    double userDefinedThreshold;
    double D0_best;
    int phi_size;
    int minNumberOfFrames;
    int maxNumberOfFrames;
    int s_Step;
    int *y_hat_best;
	int	Prev_While_Status;
	int	Num_Following_Searched;
	int s_start;
	int Prev_Buff_Analyzed;
    unsigned int frameRate;

    void alignKeyword(PhonemeSequence& phonemeSequence, Dataset &dataset);
    infra::vector_view phi_1(Dataset &dataset, double phonemeMean, double phonemeStd, int keywordSize, int phoneme, int endTime, int phonemeLength);
    double phi_2(double phonemeMean, double previousPhonemMean, int keywordSize, int phonemeLength, int previousPhonemeLength);
    double gaussian(const double x, const double mean, const double std);
};


#endif // _CLASSIFIER_H
