/************************************************************************
Project:  Phoneme Alignment
Module:   Dataset Definition
Purpose:  Defines the data structs of the instances and the labels


Function List:
SpeechUtterance
read - Read Instance from file stream

StartTimeSequence
read - Read Label from file stream

**************************** INCLUDE FILES *****************************/
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <iomanip>
#include <string>
#include <QDebug>

#include "Dataset_KWS.h"
#include "HtkFile.h"

#define NORM_SCORES_0_1

using namespace std;

// SpeechUtterance_KWS members definitions
int SpeechUtterance_KWS::mfcc_dim;
infra::vector SpeechUtterance_KWS::mfcc_mean((unsigned long)0);
infra::vector SpeechUtterance_KWS::mfcc_std((unsigned long)0);

// PhonemeSequence static memebers definitions
unsigned int PhonemeSequence::numOfPhonemes;
std::map<std::string, int> PhonemeSequence::phoneme2index;
std::map<int,std::string> PhonemeSequence::index2phoneme;
std::string PhonemeSequence::silenceSymbol;
	int Dataset::Ind_Filled;
	infra::matrix * Dataset::scores;
	
	infra::matrix * Dataset::distances;
	int Dataset::distances_index;

	int Dataset::Start_Speech;
	int Dataset::End_Speech;

/************************************************************************
Function:     PhonemeSequence::load_phoneme_map

Description:  Load the phonemes file andbuild maps
Inputs:       string &filename
Output:       void.
Comments:     none.
***********************************************************************/
void PhonemeSequence::constructPhonemeMap()
{
	// Generate phoneme mapping
    index2phoneme[0] = "%"; phoneme2index["%"] = 0;
    index2phoneme[1] = "i"; phoneme2index["i"] = 1;
    index2phoneme[2] = "d"; phoneme2index["d"] = 2;
    index2phoneme[3] = "r"; phoneme2index["r"] = 3;
    index2phoneme[4] = "o"; phoneme2index["o"] = 4;
    index2phoneme[5] = "Z"; phoneme2index["Z"] = 5;
    index2phoneme[6] = "e"; phoneme2index["e"] = 6;
    index2phoneme[7] = "n"; phoneme2index["n"] = 7;
    index2phoneme[8] = "A"; phoneme2index["A"] = 8;
    index2phoneme[9] = "u"; phoneme2index["u"] = 9;
    index2phoneme[10] = "t"; phoneme2index["t"] = 10;
    index2phoneme[11] = "l"; phoneme2index["l"] = 11;
    index2phoneme[12] = "$"; phoneme2index["$"] = 12;
    index2phoneme[13] = "b"; phoneme2index["b"] = 13;
    index2phoneme[14] = "h"; phoneme2index["h"] = 14;
    index2phoneme[15] = "a"; phoneme2index["a"] = 15;
    index2phoneme[16] = "g"; phoneme2index["g"] = 16;
    index2phoneme[17] = "m"; phoneme2index["m"] = 17;
    index2phoneme[18] = "y"; phoneme2index["y"] = 18;
    index2phoneme[29] = "p"; phoneme2index["p"] = 19;
    index2phoneme[20] = "v"; phoneme2index["v"] = 20;
    index2phoneme[21] = "j"; phoneme2index["j"] = 21;
    index2phoneme[22] = "c"; phoneme2index["c"] = 22;
    index2phoneme[23] = "k"; phoneme2index["k"] = 23;
    index2phoneme[24] = "x"; phoneme2index["x"] = 24;
    index2phoneme[25] = "z"; phoneme2index["z"] = 25;
    index2phoneme[26] = "s"; phoneme2index["s"] = 26;
    index2phoneme[27] = "f"; phoneme2index["f"] = 27;
    index2phoneme[28] = "q"; phoneme2index["q"] = 28;
    index2phoneme[29] = "sil"; phoneme2index["sil"] = 29;

    numOfPhonemes = 30;
}

/***********************************************************************/
PhonemeSequence::PhonemeSequence(string &keyword, int numOfPhonemes)
{
    string keyword_phoneme, keyword_phoneme_check;
	int index;

    this->constructPhonemeMap();

    for(int i=0; i < numOfPhonemes; i++)
	{
        keyword_phoneme = keyword[i];
		index = phoneme2index[keyword_phoneme];
		keyword_phoneme_check =  index2phoneme[index];
		if (keyword_phoneme != keyword_phoneme_check)
			std::cerr << "Error: /" << keyword_phoneme << "/ is not a legal phoneme" << std::endl;
        keywordPhonemesIndices.push_back(index);
        keywordPhonemes.push_back(keyword_phoneme);
	}
}

/************************************************************************
Function:     PhonemeSequence::set_silence_symbol

Description:  Set the silence symbol
Inputs:       string &symbol
Output:       void.
Comments:     none.
***********************************************************************/
void PhonemeSequence::setSilenceSymbol(string &symbol)
{
    silenceSymbol = symbol;
    index2phoneme[0] = silenceSymbol;
    phoneme2index[silenceSymbol] = 0;
}

/************************************************************************
Function:     operator << for StartTimeSequence

Description:  Write PhonemeSequence& vector to output stream
Inputs:       std::ostream&, const StringVector&
Output:       std::ostream&
Comments:     none.
***********************************************************************/
std::ostream& operator<< (std::ostream& os, std::vector<int> y)
{
	for (uint i=0; i < y.size(); i++)
		os <<  y[i]  << " ";

	return os;
}

/************************************************************************
Function:     FeatureSegments_KWS::~FeatureSegments_KWS

Description:  Destructor
Inputs:       
Output:       
Comments:     
***********************************************************************/
SpeechUtterance_KWS::~SpeechUtterance_KWS()
{
	delete[] mfcc2;
}



/************************************************************************
Function:     SpeechUtterance_KWS::read

Description:  Read InstanceType from file stream
Inputs:       string &filename
Output:       void.
Comments:     none.
***********************************************************************/
void SpeechUtterance_KWS::read(std::string &filename, bool print_header)
{
	FILE *instream;
	if ( (instream = fopen(filename.c_str(),"rb")) == NULL) {
		std::cerr << "Error: Cannot open input file " << filename  << "." << std::endl;
		exit(-1);
	}

	// construct HTK file
	HtkFile my_htk(instream); 
	my_htk.read_header();
	num_frames = my_htk.nSamples();
	mfcc_dim = my_htk.num_coefs();
	mfcc.resize(num_frames, mfcc_dim);
	mfcc2 = new double [num_frames*mfcc_dim];
	if (print_header)
		my_htk.print_header();

	// read HTK file
	double *data;
	data = (double*)malloc(sizeof(double)*mfcc_dim);

	for (int i=0; i < num_frames; i++)
	{
		int rc = my_htk.read_next_vector(data);
		if ( rc != mfcc_dim ) {
			std::cerr << "Error: Cannot read sample number " << i << " from " << filename  << "." << std::endl;
			exit(-1);
		}
		// copy data to infra matrix
		if (mfcc_mean.size())
		{
			for (int j=0; j < mfcc_dim; j++)
			{
				//mfcc(i,j) = (data[j] - mfcc_mean[j])/mfcc_std[j];
				mfcc(i,j) = data[j] - mfcc_mean[j]; // Cepstral Mean Subtraction 
				mfcc2[i*mfcc_dim+j] = mfcc(i,j);
			}
		}
		else
		{
			for (int j=0; j < mfcc_dim; j++)
			{
				mfcc(i,j) = data[j];
				mfcc2[i*mfcc_dim+j] = mfcc(i,j);
			}
		}
	}

	// close file
	fclose(instream);
	In_Buffer_Ind = num_frames;
	free(data);
}

/************************************************************************
Function:     FeatureSegments_KWS::FeatureSegments_KWS

Description:  Constructor
Inputs:       int Num_Frames, int Num_Features
Output:       void.
Comments:     Intializes Feautres Matrix for using inside Keyword Alignment.
***********************************************************************/
Dataset::Dataset(int _Num_Features, int _Num_PHNs, int _Last_s):
Num_Features(_Num_Features), Num_Phns(_Num_PHNs), Dist_dim(_Last_s)
{
	
	Max_InBuff_Length = 50000;
	Seg_Length = 500;
	Ind_Allocated = 0;
	Ind_Filled = 0;
	Ind_Filled_Circular =0;
	Features_Circular = new double *[Max_InBuff_Length];
	Features.resize(Seg_Length,Num_Features);
	//scores.resize(Seg_Length,Num_Phns);
	//distances.resize(Seg_Length,Dist_dim);

	Start_Speech = 0;
	End_Speech = 0;
	//scores_index = -1;
	distances_index = -1;
	
	First_Frame = 0;
	Last_Frame = 0;

}

/************************************************************************
Function:     FeatureSegments_KWS::~FeatureSegments_KWS

Description:  Destructor
Inputs:       
Output:       
Comments:     
***********************************************************************/

Dataset::~Dataset()
{
	int Temp;
	if (Ind_Allocated<Max_InBuff_Length)
		Temp = Ind_Allocated;
	else
		Temp = Max_InBuff_Length;
	for (int j=0; j < Temp; j++)
		delete[] Features_Circular[j];
	delete[] Features_Circular;
}

/************************************************************************
Function:     Get_Frame

Description:  Function for Getting Frames from Input Buffer
Inputs:       Input Buffer=
Output:       void
Comments:     none.
***********************************************************************/
void Dataset::Get_Frame(float *InBuff, int InBuff_Length,  int Start_InBuff, int End_InBuff)
{
	infra::matrix Matrix_Tmp;
	int Ind_Needed_Buff = InBuff_Length+Ind_Filled;
	int Seg_Alocation_Length;
	// Memory Allocation of buffers: Features_Circular, Features, scores, distances 
	if (Ind_Needed_Buff>Ind_Allocated)
	{
		if (!End_InBuff)
			Seg_Alocation_Length = Seg_Length;
		else
			Seg_Alocation_Length = Ind_Needed_Buff-Ind_Allocated;

		int Seg_Circular_Alocation_Length = (Max_InBuff_Length-Ind_Allocated)<(Seg_Alocation_Length) ? (Max_InBuff_Length-Ind_Allocated) : Seg_Alocation_Length;
		if (Seg_Circular_Alocation_Length>0)
		{
			for (int i = 0; i<Seg_Circular_Alocation_Length; i++)
				Features_Circular[Ind_Allocated+i] = new double[Num_Features];
		}
		
		int Num_Feat_Vectors_New = Ind_Allocated+Seg_Alocation_Length;

		int Num_Feat_Vectors = Features.height();
		Matrix_Tmp.resize(Num_Feat_Vectors, Num_Features);
		Matrix_Tmp = Features;

		Features.resize(Num_Feat_Vectors_New,Num_Features);
		Features.submatrix(0,0,Num_Feat_Vectors,Num_Features) = Matrix_Tmp;

		Num_Feat_Vectors = scores->height();
		Matrix_Tmp.resize(Num_Feat_Vectors, Num_Phns);
		Matrix_Tmp = (*scores);
		scores->resize(Num_Feat_Vectors_New,Num_Phns);
		scores->submatrix(0,0,Num_Feat_Vectors,Num_Phns) = Matrix_Tmp;

		Num_Feat_Vectors = distances->height();
		Matrix_Tmp.resize(Num_Feat_Vectors, Dist_dim);
		Matrix_Tmp = *distances;
		distances->resize(Num_Feat_Vectors_New,Dist_dim);
		distances->submatrix(0,0,Num_Feat_Vectors,Dist_dim) = Matrix_Tmp;
		Ind_Allocated += Seg_Alocation_Length;
	}



	First_Frame = Ind_Filled;
	for (int i=0; i<InBuff_Length; i++)
	{
		for (int j=0; j < Num_Features; j++)
		{
			Features_Circular[Ind_Filled_Circular][j] = InBuff[i*Num_Features+j];

			Features(Ind_Filled,j) = InBuff[i*Num_Features+j];		
		}
		Ind_Filled++;
		Ind_Filled_Circular = Ind_Filled%Max_InBuff_Length; // This line is done to make a circular Buffer for Features_Circular
	}
	Last_Frame = Ind_Filled;


}


/************************************************************************
Function:     operator << for IntVector_KWS

Description:  Write PhonemeSequence& vector to output stream
Inputs:       std::ostream&, const StringVector&
Output:       std::ostream&
Comments:     none.
***********************************************************************/
std::ostream& operator<< (std::ostream& os, const IntVector_KWS& v)
{
	IntVector_KWS::const_iterator iter = v.begin();
	IntVector_KWS::const_iterator end = v.end();

	while(iter < end) {
		os << *iter << " ";
		++iter;
	}
	return os;
}

// --------------------------  EOF ------------------------------------//


