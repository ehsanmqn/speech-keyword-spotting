
#ifndef _MY_DATASET_KWS_H_
#define _MY_DATASET_KWS_H_

/************************************************************************
 Project:  Phoeneme Alignment
 Module:   Dataset Definitions
 Purpose:  Defines the data structs of instance and label

 
 *************************** INCLUDE FILES ******************************/
#include <cstdlib>
#include <fstream>
#include <vector>
#include <map>
#include <utility>
#include "infra.h"
#include <string.h>

#define MAX_LINE_SIZE 4096

using namespace std;

class IntVector_KWS : public std::vector<int> {
public:
	unsigned int read(std::string &filename) {
        std::ifstream ifs(filename.c_str());
        // check input file stream
		if (!ifs.good()) {
            std::cerr << "Error: Unable to read IntVector from " << filename
            << std::endl;
			exit(-1);
		}
        // delete the vector
		clear();
        // read size from the stream
		int value;
		int num_values;
		if (ifs.good()) 
			ifs >> num_values;
		while (ifs.good() && num_values--) {
			ifs >> value;
			push_back(value);
		}
		ifs.close();
		return size();
	}
	
};

std::ostream& operator<< (std::ostream& os, const IntVector_KWS& v);

/***********************************************************************/

class StringVector_KWS : public std::vector<std::string> {
public:
	
	unsigned int read(std::string &filename) {
        //KW_Num = 0;
        std::ifstream ifs;
		char line[MAX_LINE_SIZE];
		ifs.open(filename.c_str());
		if (!ifs.is_open()) {
            std::cerr << "Error: Unable to open file list " << filename << std::endl;
			exit(-1);
		}    
		while (!ifs.eof()) {
			ifs.getline(line,MAX_LINE_SIZE);
			if (strcmp(line,""))
			{
                push_back(std::string(line));
                //KW_Num++;
            }
		}
		ifs.close();
		return size();
	}
    //int KW_Num;
};


class PhonemeSequence
{
public:
    PhonemeSequence(string &keyword, int numOfPhonemes);

    static unsigned int numOfPhonemes;
	static std::map<std::string, int> phoneme2index;
	static std::map<int,std::string> index2phoneme; 
    static std::string silenceSymbol;
    std::vector<int> keywordPhonemesIndices;
    std::vector<std::string> keywordPhonemes;

    static void constructPhonemeMap();
    static void setSilenceSymbol(std::string &symbol);
};


class StartTimeSequence : public std::vector<int>
{
public:
	unsigned int read(std::string &filename) {
        std::ifstream ifs(filename.c_str());
        // check input file stream
		if (!ifs.good()) {
            std::cerr << "Info: unable to read StartTimeSequence from "
            << filename << std::endl;
			return 0;
		}
        // delete the vector
		clear();
        // read size from the stream
		while (ifs.good()) {
            std::string value;
			ifs >> value;
			if (value == "") break;
            push_back(int(std::atoi(value.c_str())));
		}
		ifs.close();
		return size();
	}
};

//std::ostream& operator<< (std::ostream& os, const StartTimeSequence_KWS& y);
std::ostream& operator<< (std::ostream& os, std::vector<int> y);

/***********************************************************************/

class SpeechUtterance_KWS
{
 public:
	 ~SpeechUtterance_KWS();
     void read(std::string &filename, bool print_header = false);
	 

 public:
	 int In_Buffer_Ind;
	 infra::matrix mfcc;
	 double *mfcc2;
	 int num_frames;
	 static int mfcc_dim;
	 static infra::vector mfcc_mean;
	 static infra::vector mfcc_std;

	 /*
	 infra::matrix scores;
	 int scores_index;
	 infra::matrix distances;
	 int distances_index;*/
  
};

/************************** My added class *******************************/

class Dataset
{
public:
	Dataset(int _Num_Features, int _Num_PHNs, int _Last_s);
	~Dataset();
	void Get_Frame(float *InBuff, int InBuff_Length,  int Start_InBuff, int End_InBuff);

	int Max_InBuff_Length;
	int Seg_Length;
	int Ind_Allocated;
	static int Ind_Filled;
	int Ind_Filled_Circular;
	int Num_Features;

	int Num_Phns;
	int Dist_dim;
	double **Features_Circular;
	infra:: matrix Features;

	static infra::matrix *scores;
	//static int scores_index;
	static infra::matrix *distances;
	static int distances_index;

	static int Start_Speech;
	static int End_Speech;

	int First_Frame;
	int Last_Frame;

};



#endif // _MY_DATASET_H_
