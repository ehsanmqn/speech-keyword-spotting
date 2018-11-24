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


void Vajegan(float *inputSignal, int inputSignalLength, string word, KeywordClassifier *keywordClassifier, PhonemeClassifier phonemeClassifier);
void Initialize(string Feature_ConfigFile, string mfcc_stats_file);
