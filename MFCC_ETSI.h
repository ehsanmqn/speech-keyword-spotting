#ifndef _FEFUNC_H
#define _FEFUNC_H

#ifndef KWS_EXPORT_CPP_CLASS_DECL
#define KWS_EXPORT_CPP_CLASS_DECL __declspec(dllexport)
#define KWS_IMPORT_CPP_CLASS_DECL __declspec(dllimport)
#endif

#ifdef KWS_EXPORTS
 #define KWS_API_CLASS KWS_EXPORT_CPP_CLASS_DECL
 #else
 #define KWS_API_CLASS KWS_IMPORT_CPP_CLASS_DECL
 #endif

#include	<iostream>
#include	<fstream>
#include    <stdio.h>
#include    <math.h>
#include    <stdlib.h>
#include    <string>
//#include    <conio.h>


/*----------------------
* Constant Definitions
*----------------------*/
#define M_PI        3.14159265358979323846
#define M_SQRT2     1.41421356237309504880

#define PI          M_PI
#define PIx2        6.28318530717958647692

#define ENERGYFLOOR_FB        -50.0  /*0.0 */
#define ENERGYFLOOR_logE      -50.0  /*4.0 */


/*-----------------
* Data Structures
*-----------------*/
typedef struct FFT_Window_tag
{
	int StartingPoint;
	int Length;
	float *Data;
	struct FFT_Window_tag *Next;
}
FFT_Window;                     /* Structure for FFT window (one triangle in
								the chained list) */

/*class FFT_Window
{
public:
	int StartingPoint;
	int Length;
	float *Data;
	FFT_Window *Next;
};  */ 

/***********************************************************************/
class FeatureExtracting // My Added Class
{
public:
	FeatureExtracting();
	~FeatureExtracting();

	static void load_feature_extraction_parameters(std::string Feature_ConfigFileName,
		std::string MFCC_StatsFileName);
	static void delete_feature_extraction_parameters(void);
	float * maindtdt(int n_frame,float *cep);
	
	/*---------------------
	* Function Prototypes
	*---------------------*/
	void DCOffsetFilter( float *CircBuff, int BSize, int *BPointer, int nSamples );
	void InitializeHamming (float *win, int len);
	void Window (float *data, float *win, int len);
	void rfft (float *x, int n, int m);
	void InitFFTWindows (FFT_Window * FirstWin,
		float StFreq,
		float SmplFreq,
		int FFTLength,
		int NumChannels);
	void ReleaseFFTWindows (FFT_Window *FirstWin );
	void ComputeTriangle (FFT_Window * FirstWin);
	void MelFilterBank (float *SigFFT, FFT_Window * FirstWin);

	float *InitDCTMatrix (int NumCepstralCoeff, int NumChannels);
	void DCT (float *Data, float *Mx, int NumCepstralCoeff, int NumChannels);

	
	void CMS_Live(float *incep, int nfr);

	int ReadWaveFromBuffer (float *InBuff, int InBuffSize, int *InBuffPointer,
						float *CircBuff, int CircBuffSize, int CircBuffPointer, int nSamples);

	int MFCC_Extractor(float *IN_Wave_Buffer, int IN_Wave_Buffer_Length,
		float *Features);

	static int SamplingFrequency;
	static int FrameLength;
	static int FrameShift;
	static float PreEmphasize, StartFrequency;
	static int FFTLength, MaxWindowSize, NumChannels, NumCeps, Feature_Dim;
	static int NologE ,Noc0;
	
	static int Total_Num_Frames;

	//Definition of variables for CMS_Live
	static float *xn_1;
	static float *yn_1;
	static float *cur_mean;
	static int initializeCMS;
	static float *CMS_Vector, *CVN_Vector;
	float *Prev_Wav_samples;
	int PrevBuff_Size;
	
};

#endif
