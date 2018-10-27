#ifndef KWS_EXPORT_CPP_CLASS_DECL
#define KWS_EXPORT_CPP_CLASS_DECL __declspec(dllexport)
#define KWS_IMPORT_CPP_CLASS_DECL __declspec(dllimport)
#endif

#ifdef KWS_EXPORTS
#define KWS_API_CLASS KWS_EXPORT_CPP_CLASS_DECL
#else
#define KWS_API_CLASS KWS_IMPORT_CPP_CLASS_DECL
#endif

#include <string>
#include "Classifier_KWS.h"

using namespace std;


float Vajegan(float *inputSignal, int inputSignalLength, string word, KeywordClassifier *keywordClassifier, PhonemeClassifier phonemeClassifier);
void Initialize(string KWS_Model_File, string KWS_Model_ConfigFile, double Treshold,
                int Loop_Step, string PhnClassi_Model_File, string PHN_StatsFile, string PHN_MapFile,
                string Feature_ConfigFile, string mfcc_stats_file);
