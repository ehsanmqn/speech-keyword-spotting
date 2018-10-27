#include	"MFCC_ETSI.h"
#include	<iostream>
#include	<fstream>
#include <string.h>
//#include <conio.h>
using std::cout;
using std::cerr;
using std::endl;

using namespace std;

int FeatureExtracting::SamplingFrequency;
int FeatureExtracting::FrameLength;
int FeatureExtracting::FrameShift;
float FeatureExtracting::PreEmphasize;
float FeatureExtracting::StartFrequency;
int FeatureExtracting::FFTLength;
int FeatureExtracting::MaxWindowSize;
int FeatureExtracting::NumChannels;
int FeatureExtracting::NumCeps;
int FeatureExtracting::Feature_Dim;
int FeatureExtracting::NologE;
int FeatureExtracting::Noc0;

int FeatureExtracting::Total_Num_Frames;

//Definition of variables for CMS_Live
float* FeatureExtracting::xn_1;
float* FeatureExtracting::yn_1;
float* FeatureExtracting::cur_mean;
int FeatureExtracting::initializeCMS;
float* FeatureExtracting::CMS_Vector;
float* FeatureExtracting::CVN_Vector;

/************************************************************************
Function:     Feature_Extracting::load_feature_extraction_parameters

Description:  Loading and setting static parameters for feature extraction class.
Inputs:       std::string Feature_ConfigFileName, std::string MFCC_StatsFileName
Output:       void.
Comments:     none.
***********************************************************************/
void FeatureExtracting::loadFeatureExtractionParameters(string Feature_ConfigFileName, string MFCC_StatsFileName)
{
	Total_Num_Frames = 0;

	std::string Temp;
	std::ifstream ifs;
	ifs.open(Feature_ConfigFileName.c_str());
	if ( !ifs.good() )
	{
		std::cerr << "Unable to open model config file: " << Feature_ConfigFileName << std::endl;
		exit(-1);
	}
	while (ifs.good())
	{    
		std::string Temp;
		ifs >> Temp;
		if (Temp == "") continue;
		if (Temp == "SAMPLING_FREQ")
		{
			ifs >> Temp;
			ifs >> Temp;
			SamplingFrequency =  atoi(Temp.c_str());
		}
		else if (Temp == "FRAME_LENGTH")
		{
			ifs >> Temp;
			ifs >> Temp;
			FrameLength = atoi(Temp.c_str());
		}
		else if (Temp == "FRAME_SHIFT")
		{
			ifs >> Temp;
			ifs >> Temp;
			FrameShift = atoi(Temp.c_str());
		}
		else if (Temp == "PRE_EMPHASIS")
		{
			ifs >> Temp;
			ifs >> Temp;
			PreEmphasize =  atof(Temp.c_str());
		}
		else if (Temp == "FFT_LENGTH")
		{
			ifs >> Temp;
			ifs >> Temp;
			FFTLength =  atoi(Temp.c_str());
		}
		else if (Temp == "MAXWINDOWSIZE")
		{
			ifs >> Temp;
			ifs >> Temp;
			MaxWindowSize =  atoi(Temp.c_str());
		}
		else if (Temp == "NUM_CHANNELS")
		{
			ifs >> Temp;
			ifs >> Temp;
			NumChannels =  atoi(Temp.c_str());
		}
		else if (Temp == "STARTING_FREQ")
		{
			ifs >> Temp;
			ifs >> Temp;
			StartFrequency =  atof(Temp.c_str());
		}
		else if (Temp == "NUM_CEP_COEFF")
		{
			ifs >> Temp;
			ifs >> Temp;
			NumCeps =  atoi(Temp.c_str());
		}
		else if (Temp == "Noc0")
		{
			ifs >> Temp;
			ifs >> Temp;
			Noc0 =  atof(Temp.c_str());
		}
		else if (Temp == "NologE")
		{
			ifs >> Temp;
			ifs >> Temp;
			NologE =  atoi(Temp.c_str());
		}
		else
		{
			std::cerr << "Error: Wrong format for Feature Extraction Config File=  " << Feature_ConfigFileName << std::endl;
		}
	}
	ifs.close();
	Feature_Dim = NumCeps - (Noc0 ? 1:0) + (NologE ? 0:1);

	xn_1 = new float[Feature_Dim*3];
	yn_1 = new float[Feature_Dim*3] ();
	cur_mean = new float[Feature_Dim*3] ();
	initializeCMS = 1;
	CMS_Vector = new float[Feature_Dim*3] ();
	CVN_Vector = new float[Feature_Dim*3] ();

    //Description:  Read mfcc statistics from an external file
	ifs.open(MFCC_StatsFileName.c_str());

	if (!ifs.good())
	{
		std::cerr << "Warning: Unable to open mfcc stats File: " << MFCC_StatsFileName << std::endl;
	}
	else
	{
		// read matrix dimensions
		int file_height, file_width;
		ifs >> Temp;
		file_height = atoi(Temp.c_str());
		ifs >> Temp;
		file_width = atoi(Temp.c_str());
		if ((file_width!=(Feature_Dim*3))||(file_height!=2))
		{
			std::cerr << "Warning: Wrong Dimension for mfcc stats in File: " << MFCC_StatsFileName << std::endl;
		}
		for(int i=0; i<(Feature_Dim*3); i++)
		{    
			ifs >> Temp;
			CMS_Vector[i] = atof(Temp.c_str());
		}
		for(int i=0; i<(Feature_Dim*3); i++)
		{    
			ifs >> Temp;
			CVN_Vector[i] = atof(Temp.c_str());
		}
	}
	ifs.close();
}

/************************************************************************
Function:     Feature_Extracting::delete_feature_extraction_parameters

Description:  deleting memory allocated for feature extraction static parameters 
Inputs:       void
Output:       void.
Comments:     none.
***********************************************************************/
void FeatureExtracting::delete_feature_extraction_parameters(void)
{
	initializeCMS = 1;
	delete[] xn_1;
	delete[] yn_1;
	delete[] cur_mean;
	delete[] CMS_Vector;
	delete[] CVN_Vector;
	
}
/************************************************************************
Function:     Feature_Extracting::Feature_Extracting

Description:  Constructor
Inputs:       none.
Output:       none.
Comments:     none.
***********************************************************************/
FeatureExtracting::FeatureExtracting(void)
{
	Prev_Wav_samples = new float[FrameLength+5*FrameShift] (); // 5 is set because of delta coefficients which are computed accros -3 to +3 frames over the central frame 
	PrevBuff_Size = FrameLength+5*FrameShift;
}
/************************************************************************
Function:     Feature_Extracting::~Feature_Extracting

Description:  Destructor
Inputs:       none.
Output:       none.
Comments:     none.
***********************************************************************/
FeatureExtracting::~FeatureExtracting(void)
{
	delete[] Prev_Wav_samples;
}
/*----------------------------------------------------------------------------
* FUNCTION NAME: ReadWaveFromBuffer
*
* PURPOSE:       Reads waveform into memory
*
* INPUT:
*   InBuff         Input waveform buffer
*   InBuffSize     Buffer size
*   InBuffPointer  Buffer pointer
*   CircBuff       Circular float buffer
*	CircBuffSize   int CircBuffPointer
*   nSamples       Number of samples to be read
*   
*
* OUTPUT
*                Waveform stored in circular float buffer. Buffer pointer
*                remains untouched (points to the first sample read)
*
* RETURN VALUE
*   0            In case of any errors
*   1            Otherwise
*

*---------------------------------------------------------------------------*/
int FeatureExtracting::ReadWaveFromBuffer (float *InBuff, int InBuffSize, int *InBuffPointer,
	float *CircBuff, int CircBuffSize, int CircBuffPointer, int nSamples)
{
	for (int i=0; i<nSamples; i++ )
	{
		if ((*InBuffPointer)>=InBuffSize) return 0;
		CircBuff[CircBuffPointer] = InBuff[*InBuffPointer];

		CircBuffPointer = (CircBuffPointer+1)%CircBuffSize;
		*InBuffPointer = *InBuffPointer+1;
	}
	return 1;
}
/*---------------------------------------------------------------------------*/
float * FeatureExtracting::maindtdt(int n_frame,float *cep)
{


	register int  i,j;


	/* output info */
	//fprintf(stderr,"Delta Cepstrums -> ");
	float M1 = 3;
	float M2 = 3;
	float TM1, TM2, t4;
	int t;
	float temp1, temp2;
	TM1 = M1*(M1+1)*(2*M1+1)/3; 
	TM2 = M2*(M2+1)*(2*M2+1)/3; 

	t4 = M2*(M2+1)*(2*M2+1)*(3*M2*M2+(3*M2)-1)/15;

	for(i=0; i<n_frame; i++){
		if( i-M1 < 0 || i+M1 > n_frame-1 ){
			for(j=0; j<Feature_Dim; j++)
				cep[(i*Feature_Dim*3)+j+Feature_Dim] = 0.0;
		}else{
			for(j=0; j<Feature_Dim; j++){
				temp1 = 0;
				for( t=(int)-M1; t<=M1; t++)
					temp1 += (t*cep[(i+t)*Feature_Dim*3+j]);
				cep[(i*Feature_Dim*3)+j+Feature_Dim] = temp1/TM1;
			}
		}
		if( i-M2 < 0 || i+M2 > n_frame-1 ){
			for(j=0; j<Feature_Dim; j++)
				cep[(i*Feature_Dim*3)+j+(2*Feature_Dim)] = 0.0;
		}else{
			for(j=0; j<Feature_Dim; j++){
				temp1 = temp2 = 0;
				for( t=(int)-M2; t<=M2; t++){
					temp1 += cep[(i+t)*Feature_Dim*3+j];
					temp2 += t*t*cep[(i+t)*Feature_Dim*3+j];
				}
				cep[(i*Feature_Dim*3)+j+(2*Feature_Dim)] =2*(TM2 * temp1 - (2*M2+1) * temp2) / ((TM2*TM2) - (2*M2+1)*t4);
			}
		}	
	}

	return cep;
}
/*---------------------------------------------------------------------------*/
void FeatureExtracting::CMS_Live(float *incep, int nfr)
{
	float temp;
//	int isBlockSilence;
	int i,j;

	if (initializeCMS==1) { //init value
		memcpy(xn_1,CMS_Vector,sizeof(float)*Feature_Dim*3);
		initializeCMS = 0;
	}
	//////////////////////////////////////////////////////////////////////////
	/*temp=0;//calc buffer energy
	for (j=0; j<nfr; j++)
	temp += incep[j*ceplen];
	temp/=nfr;
	isBlockSilence = (temp < SilThreshold )? 1 : 0;
	if (isBlockSilence)//don't update cur_mean
	{
	for (j=0; j<nfr; j++)
	{
	for(i=0; i<ceplen; i++)
	{
	incep[j*ceplen+i] -= cur_mean[i];
	}
	}
	}else{ //update cur_mean*/
	for (j=0; j<nfr; j++)
	{
		for(i=0; i<(Feature_Dim*3); i++)
		{
			/* y[n]=x[n]-x[n-1]+0.996*y[n-1] */
			temp        = incep[j*Feature_Dim*3+i];
			incep[j*Feature_Dim*3+i] = incep[j*Feature_Dim*3+i] - xn_1[i] + 0.996*yn_1[i];
			//save for next
			xn_1[i]     = temp;
			yn_1[i]     = incep[j*Feature_Dim*3+i];
		}
	}
	//calc current mean
	for (i=0; i<Feature_Dim*3; i++)
	{
		// xnew=xold-dc , dc=xold-xnew, dc= temp-yn_1 for an element
		cur_mean[i]=xn_1[i]-yn_1[i];
	}
	memcpy(CMS_Vector,cur_mean,sizeof(float)*Feature_Dim*3);
	//}
}

/*---------------------------------------------------------------------------
* FUNCTION NAME: DCOffsetFilter
*
* PURPOSE:       DC offset removal from speech waveform
*
* INPUT:
*   CircBuff     Pointer to input circular buffer
*   BSize        Buffer size
*   BPointer     Pointer to buffer pointer
*   nSamples     Number of samples to be filtered
*
* OUTPUT
*                Filtered data in the circular buffer
*                last output sample pointed by the buffer pointer
*
* RETURN VALUE
*   none
*

*---------------------------------------------------------------------------*/
void FeatureExtracting::DCOffsetFilter( float *CircBuff, int BSize, int *BPointer, int nSamples )
{
	int i;

	for ( i=0; i<nSamples; i++ )
	{
		/* y[n]=x[n]-x[n-1]+0.999*y[n-1] */
		CircBuff[(*BPointer+1)%BSize]=CircBuff[(*BPointer+2)%BSize]-
			CircBuff[(*BPointer+1)%BSize]+0.999*CircBuff[*BPointer];
		*BPointer=(*BPointer+1)%BSize;
	}
}


/*---------------------------------------------------------------------------
* FUNCTION NAME: InitializeHamming
*
* PURPOSE:       Initializes Hamming window coefficients
*
* INPUT:
*   win          Pointer to window buffer
*   len          Window length
*
* OUTPUT
*                Hamming window coefficients stored in window buffer pointed
*                to by *win*
*
* RETURN VALUE
*   none
*

*---------------------------------------------------------------------------*/
void FeatureExtracting::InitializeHamming (float *win, int len)
{
	int i;

	for (i = 0; i < len / 2; i++)
		win[i] = 0.54 - 0.46 * cos (PIx2 * i / (len - 1));
}

/*---------------------------------------------------------------------------
* FUNCTION NAME: Window
*
* PURPOSE:       Performs windowing on input speech frame (multiplies input
*                samples by the corresponding window coefficients)
*
* INPUT:
*   data         Pointer to input speech buffer
*   win          Pointer to window buffer
*   len          Window (or frame) length
*
* OUTPUT
*                Windowed speech frame stored at the same place as the
*                original speech samples (pointed by *data*)
*
* RETURN VALUE
*   none
*

*---------------------------------------------------------------------------*/
void FeatureExtracting::Window (float *data, float *win, int len)
{
	long i;

	for (i = 0; i < len / 2; i++)
		data[i] *= win[i];
	for (i = len / 2; i < len; i++)
		data[i] *= win[len - 1 - i];
}

/*---------------------------------------------------------------------------
* FUNCTION NAME: rfft
*
* PURPOSE:       Real valued, in-place split-radix FFT
*
* INPUT:
*   x            Pointer to input and output array
*   n            Length of FFT, must be power of 2
*
* OUTPUT         Output order
*                  Re(0), Re(1), ..., Re(n/2), Im(N/2-1), ..., Im(1)
*
* RETURN VALUE
*   none
*
* DESIGN REFERENCE:
*                IEEE Transactions on Acoustic, Speech, and Signal Processing,
*                Vol. ASSP-35. No. 6, June 1987, pp. 849-863.
*
*                Subroutine adapted from fortran routine pp. 858-859.
*                Note corrected printing errors on page 859:
*                    SS1 = SIN(A3) -> should be SS1 = SIN(A);
*                    CC3 = COS(3)  -> should be CC3 = COS(A3)
*

*---------------------------------------------------------------------------*/
void FeatureExtracting::rfft (float *x, int n, int m)
{
	int j, i, k, is, id;
	int i0, i1, i2, i3, i4, i5, i6, i7, i8;
	int n2, n4, n8;
	float xt, a0, e, a, a3;
	float t1, t2, t3, t4, t5, t6;
	float cc1, ss1, cc3, ss3;
	float *r0;

	/* Digit reverse counter */

	j = 0;
	r0 = x;

	for (i = 0; i < n - 1; i++)
	{

		if (i < j)
		{
			xt = x[j];
			x[j] = *r0;
			*r0 = xt;
		}
		r0++;

		k = n >> 1;

		while (k <= j)
		{
			j = j - k;
			k >>= 1;
		}
		j += k;
	}

	/* Length two butterflies */
	is = 0;
	id = 4;

	while (is < n - 1)
	{

		for (i0 = is; i0 < n; i0 += id)
		{
			i1 = i0 + 1;
			a0 = x[i0];
			x[i0] += x[i1];
			x[i1] = a0 - x[i1];
		}

		is = (id << 1) - 2;
		id <<= 2;
	}

	/* L shaped butterflies */
	n2 = 2;
	for (k = 1; k < m; k++)
	{
		n2 <<= 1;
		n4 = n2 >> 2;
		n8 = n2 >> 3;
		e = (M_PI * 2) / n2;
		is = 0;
		id = n2 << 1;
		while (is < n)
		{
			for (i = is; i <= n - 1; i += id)
			{
				i1 = i;
				i2 = i1 + n4;
				i3 = i2 + n4;
				i4 = i3 + n4;
				t1 = x[i4] + x[i3];
				x[i4] -= x[i3];
				x[i3] = x[i1] - t1;
				x[i1] += t1;

				if (n4 != 1)
				{
					i1 += n8;
					i2 += n8;
					i3 += n8;
					i4 += n8;
					t1 = (x[i3] + x[i4]) / M_SQRT2;
					t2 = (x[i3] - x[i4]) / M_SQRT2;
					x[i4] = x[i2] - t1;
					x[i3] = -x[i2] - t1;
					x[i2] = x[i1] - t2;
					x[i1] = x[i1] + t2;
				}
			}
			is = (id << 1) - n2;
			id <<= 2;
		}

		for (j = 1; j < n8; j++)
		{
			a = j * e;
			a3 = 3 * a;
			cc1 = cos (a);
			ss1 = sin (a);
			cc3 = cos (a3);
			ss3 = sin (a3);

			is = 0;
			id = n2 << 1;

			while (is < n)
			{
				for (i = is; i <= n - 1; i += id)
				{
					i1 = i + j;
					i2 = i1 + n4;
					i3 = i2 + n4;
					i4 = i3 + n4;
					i5 = i + n4 - j;
					i6 = i5 + n4;
					i7 = i6 + n4;
					i8 = i7 + n4;
					t1 = x[i3] * cc1 + x[i7] * ss1;
					t2 = x[i7] * cc1 - x[i3] * ss1;
					t3 = x[i4] * cc3 + x[i8] * ss3;
					t4 = x[i8] * cc3 - x[i4] * ss3;
					t5 = t1 + t3;
					t6 = t2 + t4;
					t3 = t1 - t3;
					t4 = t2 - t4;
					t2 = x[i6] + t6;
					x[i3] = t6 - x[i6];
					x[i8] = t2;
					t2 = x[i2] - t3;
					x[i7] = -x[i2] - t3;
					x[i4] = t2;
					t1 = x[i1] + t5;
					x[i6] = x[i1] - t5;
					x[i1] = t1;
					t1 = x[i5] + t4;
					x[i5] = x[i5] - t4;
					x[i2] = t1;
				}
				is = (id << 1) - n2;
				id <<= 2;
			}
		}
	}
}

/*---------------------------------------------------------------------------
* FUNCTION NAME: InitFFTWindows
*
* PURPOSE:       Initializes data structure for FFT windows (mel filter bank).
*                Computes starting point and length of each window, allocates
*                memory for window coefficients.
*
* INPUT:
*   FirstWin     Pointer to first FFT window structure
*   StFreq       Starting frequency of mel filter bank
*   SmplFreq     Sampling frequency
*   FFTLength    FFT length
*   NumChannels  Number of channels
*
* OUTPUT
*                Chained list of FFT window data structures. NOTE FFT window
*                coefficients are not computed yet.
*
* RETURN VALUE
*   none
*

*---------------------------------------------------------------------------*/
void FeatureExtracting::InitFFTWindows (FFT_Window * FirstWin,
	float StFreq,
	float SmplFreq,
	int FFTLength,
	int NumChannels)

{
	int i, TmpInt;
	float freq, start_mel, fs_per_2_mel;
	FFT_Window *p1, *p2;

	/* Constants for calculation */
	start_mel = 2595.0 * log10 (1.0 + (float) StFreq / 700.0);
	fs_per_2_mel = 2595.0 * log10 (1.0 + (SmplFreq / 2) / 700.0);

	p1 = FirstWin;

	for (i = 0; i < NumChannels; i++)
	{
		/* Calculating mel-scaled frequency and the corresponding FFT-bin */
		/* number for the lower edge of the band                          */
		freq = 700 * (pow (10, (start_mel + (float) i / (NumChannels + 1) *
			(fs_per_2_mel - start_mel)) / 2595.0) - 1.0);
		TmpInt = (int) (FFTLength * freq / SmplFreq + 0.5);

		/* Storing */
		p1->StartingPoint = TmpInt;

		/* Calculating mel-scaled frequency for the upper edge of the band */
		freq = 700 * (pow (10, (start_mel + (float) (i + 2) / (NumChannels + 1)
			* (fs_per_2_mel - start_mel)) / 2595.0) - 1.0);

		/* Calculating and storing the length of the band in terms of FFT-bins*/
		p1->Length = (int) (FFTLength * freq / SmplFreq + 0.5) - TmpInt + 1;

		/* Allocating memory for the data field */
		p1->Data = (float *) malloc (sizeof (float) * p1->Length);

		/* Continuing with the next data structure or close the last structure
		with NULL */
		if (i < NumChannels - 1)
		{
			p2 = (FFT_Window *) malloc (sizeof (FFT_Window));
			p1->Next = p2;
			p1 = p2;
		}
		else
			p1->Next = NULL;
	}
	return;
}

/*---------------------------------------------------------------------------
* FUNCTION NAME: ReleaseFFTWindows
*
* PURPOSE:       Releases memory allocated for FFT windows
*
* INPUT:
*   FirstWin     Pointer to first FFT window structure
*
* OUTPUT
*   none
*
* RETURN VALUE
*   none
*

*---------------------------------------------------------------------------*/

void FeatureExtracting::ReleaseFFTWindows (FFT_Window *FirstWin )
{
	FFT_Window *p;

	while ( FirstWin->Next!=NULL )
	{
		p=FirstWin->Next->Next;
		free(FirstWin->Next->Data);
		free(FirstWin->Next);
		FirstWin->Next=p;
	}
	free(FirstWin->Data);
}

/*---------------------------------------------------------------------------
* FUNCTION NAME: ComputeTriangle
*
* PURPOSE:       Computes and stores FFT window coefficients (triangle points)
*                into initialized chained list of FFT window structures
*
* INPUT:
*   FirstWin     Pointer to first FFT window structure
*
* OUTPUT
*                Chained list of FFT window data structures with correct
*                window coefficients
*
* RETURN VALUE
*   none
*

*---------------------------------------------------------------------------*/
void FeatureExtracting::ComputeTriangle (FFT_Window * FirstWin)
{
	FFT_Window *p1;

    int low_part_length, hgh_part_length, TmpInt=0, i;

	p1 = FirstWin;
//	j = 0;
	while (p1)
	{
		low_part_length = p1->Next ?
			p1->Next->StartingPoint - p1->StartingPoint + 1 :
		TmpInt - p1->StartingPoint + 1;
		hgh_part_length = p1->Length - low_part_length + 1;

		/* Lower frequency part of the triangle */
		for (i = 0; i < low_part_length; i++)
			p1->Data[i] = (float) (i + 1) / low_part_length;

		/* Higher frequency part of the triangle */
		for (i = 1; i < hgh_part_length; i++)
			p1->Data[low_part_length + i - 1] = (float) (hgh_part_length - i) /
			hgh_part_length;

		/* Store upper edge (for calculating the last triangle) */
		TmpInt = p1->StartingPoint + p1->Length - 1;

		/* Next triangle ... */
		p1 = p1->Next;
	}
	return;
}

/*---------------------------------------------------------------------------
* FUNCTION NAME: MelFilterBank
*
* PURPOSE:       Performs mel filtering on FFT magnitude spectrum using the
*                filter bank defined by a chained list of FFT window
*                structures
*
* INPUT:
*   SigFFT       Pointer to signal FFT magnitude spectrum
*   FirstWin     Pointer to the first channel of the filter bank (first
*                element in the chained list of FFT window data structures)
*
* OUTPUT
*                Filter bank outputs stored at the beginning of input signal
*                FFT buffer pointed by *SigFFT*
*
* RETURN VALUE
*   none
*

*---------------------------------------------------------------------------*/
void FeatureExtracting::MelFilterBank (float *SigFFT, FFT_Window * FirstWin)
{
	FFT_Window *p1;
	float Sum;
	int i, j;

	p1 = FirstWin;
	j = 0;
	while (p1)
	{
		Sum = 0.0;
		for (i = 0; i < p1->Length; i++)
			Sum += SigFFT[p1->StartingPoint + i] * p1->Data[i];
		SigFFT[j] = Sum;
		j++;
		p1 = p1->Next;
	}
	return;
}

/*---------------------------------------------------------------------------
* FUNCTION NAME: InitDCTMatrix
*
* PURPOSE:       Initializes matrix for DCT computation (DCT is implemented
*                as matrix-vector multiplication). The DCT matrix is of size
*                (NumCepstralCoeff-1)-by-NumChannels. The zeroth cepstral
*                coefficient is computed separately (needing NumChannels
*                additions and only one multiplication), so the zeroth row
*                of DCT matrix corresponds to the first DCT basis vector, the
*                first one to the second one, and so on up to
*                NumCepstralCoeff-1.
*
* INPUT:
*   NumCepstralCoeff
*                Number of cepstral coeffficients
*   NumChannels  Number of filter bank channels
*
* OUTPUT
*   none
*
* RETURN VALUE
*                Pointer to the initialized DCT matrix
*

*---------------------------------------------------------------------------*/
float * FeatureExtracting::InitDCTMatrix (int NumCepstralCoeff, int NumChannels)
{
	int i, j;
	float *Mx;

	/* Allocating memory for DCT-matrix */
	Mx = (float *) malloc (sizeof (float) * (NumCepstralCoeff - 1) *
		NumChannels);

	/* Computing matrix entries */
	for (i = 1; i < NumCepstralCoeff; i++)
		for (j = 0; j < NumChannels; j++)
			Mx[(i - 1) * NumChannels + j] = cos (PI * (float) i /
			(float) NumChannels
			* ((float) j + 0.5));
	return Mx;
}

/*---------------------------------------------------------------------------
* FUNCTION NAME: DCT
*
* PURPOSE:       Computes DCT transformation of filter bank outputs, results
*                in cepstral coefficients. The DCT transformation is
*                implemented as matrix-vector multiplication. The zeroth
*                cepstral coefficient is computed separately and appended.
*                Final cepstral coefficient order is c1, c2, ...,c12, c0. The
*                output is stored right after the input values in the memory.
*                Since the mel filter bank outputs are stored at the beginning
*                of the FFT magnitude array it shouldn`t cause any problems.
*                Some memory saving can be done this way.
*
* INPUT:
*   Data         Pointer to input data buffer (filter bank outputs)
*   Mx           Pointer to DCT matrix
*   NumCepstralCoeff
*                Number of cepstral coefficients
*   NumChannels  Number of filter bank channels
*
* OUTPUT
*                Cepstral coefficients stored after the input filter bank
*                values pointed to by *Data*
*
* RETURN VALUE
*   none
*

*---------------------------------------------------------------------------*/
void FeatureExtracting::DCT (float *Data, float *Mx, int NumCepstralCoeff, int NumChannels)
{
	int i, j;

	/* Computing c1..c/NumCepstralCoeff-1/, storing result after the incoming
	data vector */
	for (i = 1; i < NumCepstralCoeff; i++)
	{
		Data[NumChannels + (i - 1)] = 0.0;
		for (j = 0; j < NumChannels; j++)
			Data[NumChannels + (i - 1)] += Data[j]
		* Mx[(i - 1) * NumChannels + j];
	}

	/* Computing c0, as the last element of output vector */
	Data[NumChannels + NumCepstralCoeff - 1] = 0.0;
	for (i = 0; i < NumChannels; i++)
		Data[NumChannels + NumCepstralCoeff - 1] += Data[i];
	return;
}

/*---------------------------------------------------------------------------*/
int FeatureExtracting::MFCC_Extractor(float *IN_Wave_Buffer, int IN_Wave_Buffer_Length,
	float *Features)
{
	int i, TmpInt;
	int FrameCounter = 0;

	//fprintf (stderr,"\r\nFeature Extraction.\r\n");

//	int Num_Frames = IN_Wave_Buffer_Length/FrameShift;
//	int Temp = Feature_Dim*3;

	int CFBSize, CFBPointer;


	float LogEnergy, StartingFrequency, EnergyFloor_FB,
		EnergyFloor_logE, *FloatBuffer,
		*FloatWindow, *pDCTMatrix, *CircFloatBuffer;
	FFT_Window FirstWindow;

	FloatBuffer = new float[MaxWindowSize + 1];
	FloatWindow = new float[MaxWindowSize / 2];

	//fprintf (stderr,"\r\nFeature Extraction.\r\n");

	/*----------------*/
	/* Initialization */
	/*----------------*/
	EnergyFloor_FB = (float) exp ((double) ENERGYFLOOR_FB);
	EnergyFloor_logE = (float) exp ((double) ENERGYFLOOR_logE);
	StartingFrequency = StartFrequency;

	/*------------------------------------------------*/
	/* Memory allocation and initialization for input */
	/* circular float buffer                          */
	/*------------------------------------------------*/
	CFBSize=FrameLength+2;
	CircFloatBuffer=(float*)malloc(sizeof(float)*CFBSize);
	if ( !CircFloatBuffer )
	{
		fprintf (stderr, "ERROR:   Memory allocation error occured!\r\n");
		//getch();
		exit(-1);
	}
	CircFloatBuffer[0]=0.0;
	CircFloatBuffer[1]=0.0;
	CFBPointer=0;

	/*-------------------------------------------------------*/
	/* Initialization of FE data structures and input buffer */
	/*-------------------------------------------------------*/
	InitializeHamming (FloatWindow, (int) FrameLength);
	InitFFTWindows (&FirstWindow, StartingFrequency,
		(float) SamplingFrequency, FFTLength, NumChannels);
	ComputeTriangle (&FirstWindow);
	pDCTMatrix = InitDCTMatrix (NumCeps, NumChannels);

	int IN_Wave_Buffer_Pointer = 0;
	ReadWaveFromBuffer(IN_Wave_Buffer, IN_Wave_Buffer_Length, &IN_Wave_Buffer_Pointer,
		CircFloatBuffer, CFBSize, CFBPointer+2, FrameLength-FrameShift);

	DCOffsetFilter( CircFloatBuffer, CFBSize, &CFBPointer, FrameLength-FrameShift );

	/*----------------------------------------------------------------*/
	/*                       Framing                                  */
	/*----------------------------------------------------------------*/
	while(ReadWaveFromBuffer(IN_Wave_Buffer, IN_Wave_Buffer_Length, &IN_Wave_Buffer_Pointer,
		CircFloatBuffer, CFBSize, (CFBPointer+2)%CFBSize, FrameShift))
	{

		FrameCounter++;
		/*-------------------*/
		/* DC offset removal */
		/*-------------------*/
		DCOffsetFilter( CircFloatBuffer, CFBSize, &CFBPointer, FrameShift );

		/*------------------*/
		/* logE computation */
		/*------------------*/
		LogEnergy = 0.0;
		for (i = 0; i < FrameLength; i++)
			LogEnergy += CircFloatBuffer[(CFBPointer+i+3)%CFBSize] * CircFloatBuffer[(CFBPointer+i+3)%CFBSize];

		if (LogEnergy < EnergyFloor_logE)
			LogEnergy = ENERGYFLOOR_logE;
		else
			LogEnergy = (float) log ((double) LogEnergy);

		/*-----------------------------------------------------*/
		/* Pre-emphasis, moving from circular to linear buffer */
		/*-----------------------------------------------------*/
		for (i = 0; i < FrameLength; i++)
			FloatBuffer[i] = CircFloatBuffer[(CFBPointer+i+3)%CFBSize] -
			PreEmphasize * CircFloatBuffer[(CFBPointer+i+2)%CFBSize];

		/*-----------*/
		/* Windowing */
		/*-----------*/
		Window (FloatBuffer, FloatWindow, (int) FrameLength);

		/*-----*/
		/* FFT */
		/*-----*/

		/* Zero padding */
		for (i = FrameLength; i < FFTLength; i++)
			FloatBuffer[i] = 0.0;

		/* Real valued, in-place split-radix FFT */
		TmpInt = (int) (log10 (double(FFTLength)) / log10 (double(2)));
		rfft (FloatBuffer, FFTLength, TmpInt); /*TmpInt = log2(FFTLength)*/

		/* Magnitude spectrum */
		FloatBuffer[0] = (float) fabs ((double) FloatBuffer[0]);  /* DC */
		for (i = 1; i < FFTLength / 2; i++)  /* pi/(N/2), 2pi/(N/2), ...,
											 (N/2-1)*pi/(N/2) */
											 FloatBuffer[i] = (float) sqrt ((double) FloatBuffer[i] *
											 (double) FloatBuffer[i] +
											 (double) FloatBuffer[FFTLength - i] *
											 (double) FloatBuffer[FFTLength - i]);
		FloatBuffer[FFTLength / 2] = (float) fabs ((double) FloatBuffer[FFTLength / 2]);  /* pi/2 */

		/*---------------*/
		/* Mel filtering */
		/*---------------*/
		MelFilterBank (FloatBuffer, &FirstWindow);

		/*-------------------------------*/
		/* Natural logarithm computation */
		/*-------------------------------*
		for (i = 0; i < NumChannels; i++)
		if (FloatBuffer[i] < EnergyFloor_FB)
		FloatBuffer[i] = ENERGYFLOOR_FB;
		else
		FloatBuffer[i] = (float) log ((double) FloatBuffer[i]);
		/*-------------------------------*/
		/*       Root  computation       */
		/*-------------------------------*/
		for (i = 0; i < NumChannels; i++)
			if (FloatBuffer[i] < EnergyFloor_FB)
				FloatBuffer[i] = ENERGYFLOOR_FB;
			else
				FloatBuffer[i] = (float) pow ((double) FloatBuffer[i],0.2);

		/*---------------------------*/
		/* Discrete Cosine Transform */
		/*---------------------------*/
		DCT (FloatBuffer, pDCTMatrix, NumCeps, NumChannels);

		/*--------------------------------------*/
		/* Append logE after c0 or overwrite c0 */
		/*--------------------------------------*/
		FloatBuffer[NumChannels + NumCeps - (Noc0 ? 1:0)] = LogEnergy;

		/* ---- PCA ------*/
		for (i=NumChannels; i< (NumChannels+Feature_Dim); i++)
			Features[(FrameCounter-1)*Feature_Dim*3+(i-NumChannels)] = FloatBuffer[i];


	} //End of while

	/*----------------*/
	/* compute Dt-dt  */
	/*----------------*/
	Features = maindtdt(FrameCounter, Features);

	CMS_Live(Features, FrameCounter);
	//BufferAll=CMS_Normalization(BufferAll,FrameCounter,(NumCeps- (Noc0 ? 1:0) + (NologE ? 0:1))); //Bahrani comment


	/*----------------*/
	/* Memory release */
	/*----------------*/

	free(CircFloatBuffer);
	free(pDCTMatrix);
	ReleaseFFTWindows(&FirstWindow);
	delete []FloatBuffer;
	delete []FloatWindow;
	return FrameCounter;
}
