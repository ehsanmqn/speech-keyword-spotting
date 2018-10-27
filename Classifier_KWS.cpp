

/************************************************************************
Project:  Phonetic Segmentation
Module:   Classifier
Purpose:  Segmentation classifier
**************************** INCLUDE FILES *****************************/
#include "Classifier_KWS.h"
#include "array3dim.h"
#include "array4dim.h"
#include "GmmLikelihoodCalculator.h"
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <vector>
#include <unistd.h>

//#include <conio.h>

using std::vector;
using std::cout;
using std::cerr;
using std::endl;

#define MISPAR_KATAN_MEOD (-1000000)
#define _max(x,y) ( (x)>(y) ? (x) : (y) )
#define _min(x,y) ( (x)<(y) ? (x) : (y) )

#define GAMMA_EPSILON 1
#define MISPAR_KATAN_MEOD (-1000000)

#define NORM_TYPE1 // normalize each phoneme in phi_0 by its num of frames
//#define NORM_TYPE2 // normalize phi by the number of frames 
#define NORM_TYPE3 // normalize phi by the number of keywords
#define NORM_SCORES_0_1

/************************************************************************
Function:     Classifier::Classifier

Description:  Constructor
Inputs:       none.
Output:       none.
Comments:     none.
***********************************************************************/

KeywordClassifier::KeywordClassifier(int numOfPhonemes)
{
    Prev_While_Status = 0;
    Num_Following_Searched = 0;
    s_start = 0;
    Prev_Buff_Analyzed = 0;

    D0_best = MISPAR_KATAN_MEOD;
    y_hat_best = new int[numOfPhonemes + 1];
    timeAligns.clear();
} 

/************************************************************************
Function:     Classifier::~Classifier

Description:  Destructor
Inputs:       none.
Output:       none.
Comments:     none.
***********************************************************************/

KeywordClassifier::~KeywordClassifier()
{
    delete[]  y_hat_best;
}

/************************************************************************
Function:     phi_1

Description:  calculate static part of phi for inference
Inputs:       SpeechUtterance &x - raw features
int i - phoneme index
int t - phoneme end time
int l - phoneme length
Output:       infra::vector_view
Comments:     none.
***********************************************************************/
infra::vector_view KeywordClassifier::phi_1(double Phn_Mean, double Phn_Std, int KW_size,
                                            int Phn,  // phoneme
                                            int t, // phoneme end time
                                            int l) // phoneme length
{
    infra::vector v(phi_size-1);
    //v.zeros();
    double Temp = 0;
    //int Temp = keyword.Current_KW;
    for (int tau = t-l+1; tau <= t; tau++){
        //if (x.scores_index < tau)
        //	PFB_C.predict(x, tau);
        Temp +=  (*Dataset::scores)(tau,Phn);
    }

#ifdef NORM_TYPE1
    Temp /= l;
#endif

    //if (x.distances_index < (t-l+1))
    //	ceps_dist(x,t-l+1);

    v(1) = beta1*(*Dataset::distances)(t-l+1,0)/KW_size;
    v(2) = beta1*(*Dataset::distances)(t-l+1,1)/KW_size;
    v(3) = beta1*(*Dataset::distances)(t-l+1,2)/KW_size;
    v(4) = beta1*(*Dataset::distances)(t-l+1,3)/KW_size;
    v(5) = beta2*gaussian(l, Phn_Mean, Phn_Std)/KW_size;

#ifdef NORM_TYPE3
    v(0) = Temp/KW_size;
    //v /=  KW_size;
#endif
    return v;
}

/************************************************************************
Function:     phi_2

Description:  calculate dynamic part of phi for inference
Inputs:       SpeechUtterance &x - raw features
int i - phoneme index
int t - phoneme end time
int l1 - phoneme length
int l2 - previous phoneme length
Output:       infra::vector_view
Comments:     none.
***********************************************************************/
double KeywordClassifier::phi_2(double Phn_Mean, double Prev_Phn_Mean,
                                int KW_size,
                                int t, // phoneme end time
                                int l1, // phoneme length
                                int l2) // previous phoneme len
{
    double v = 0;
    v = (double(l1)/Phn_Mean -
         double(l2)/Prev_Phn_Mean);
    v *= v;
    v *= beta3;
#ifdef NORM_TYPE3
    v /= KW_size;
#endif
    return v;
}

/************************************************************************
Function:     Classifier::align_keyword

Description:  Predict label of instance x
Inputs:       SpeechUtterance &x 
StartTimeSequence &y_hat
Output:       void
Comments:     none.
***********************************************************************/
void KeywordClassifier::alignKeyword(PhonemeSequence& key_w,
                                     int Buff_Length, int In_Buff_Start, int In_Buff_End)
{

    double My_tmp_Var = 0;
    //int Phn_min_Length;
    //int Phn_max_Length;

    int T;

    int L = maxNumberOfFrames+1;
    int best_s ;
    int end_frame;
    StartTimeSequence y_hat;
    std::vector < int >  y_hat_best_Tmp;
    double D1, D2, D2_max = MISPAR_KATAN_MEOD; // helper variables
    double D0_best_best = MISPAR_KATAN_MEOD;

    int *pred_l;
    int *pred_t;


    int s, s_end;

    bool End_Search = false;

    End_Search = false;
    int P = key_w.keywordPhonemesIndices.size();


    std::vector < int > KW_temp;
    int phn_temp;
    double phn0_mean, phn0_std, phn_mean, phn_std, Prev_phn_mean;


    pred_l = new int[P];
    pred_t = new int[P];


    best_s = 0;
    y_hat.resize(P);


    T = Buff_Length;


    threeDimArray<int> prev_l(P,T,L);
    threeDimArray<int> prev_t(P,T,L);
    threeDimArray<double> D0(P,T,L);

    if (In_Buff_End)
    {
        s_end = T-P*minNumberOfFrames;
        End_Search = true;
    }
    else
        s_end = T-P*maxNumberOfFrames;

    s = s_start;

    while  (s < s_end)
    {

        int t_temp;
        if (s<maxNumberOfFrames)
            t_temp = s;
        else
            t_temp = s-maxNumberOfFrames;
        // Initialization
        for (int i = 0; i < P; i++)
            for (int t = t_temp; t < T; t++)
                for (int l1 = 0; l1 < L; l1++)
                    D0(i,t,l1) = MISPAR_KATAN_MEOD;

        // Here calculate the calculation for the culculata
        /*
        Phn_min_Length = int(phoneme_length_mean[keyword.KWsPhnInds[KW_Num][0]]-phoneme_length_std[keyword.KWsPhnInds[KW_Num][0]])-1;
        Phn_max_Length = int(phoneme_length_mean[keyword.KWsPhnInds[KW_Num][0]]+phoneme_length_std[keyword.KWsPhnInds[KW_Num][0]])+1;
        if (Phn_min_Length<min_num_frames)
        Phn_min_Length = min_num_frames;
        if (Phn_max_Length>max_num_frames)
        Phn_max_Length = max_num_frames;
        int t_start = s+Phn_min_Length;
        int t_end = _min(s+Phn_max_Length,T);*/
        KW_temp = key_w.keywordPhonemesIndices;
        phn_temp =  key_w.keywordPhonemesIndices[0];
        phn0_mean = phonemeLengthMeanVector[phn_temp];
        phn0_std = phonemeLengthStdVector[phn_temp];
        for (int t = s+minNumberOfFrames; t < _min(s+maxNumberOfFrames,T); t++)
        {
            D0(0,t,t-s+1) = weigths.subvector(0,phi_size-1) * phi_1(phn0_mean, phn0_std, P, phn_temp, t, t-s+1);
            My_tmp_Var = D0(0,t,t-s+1);
        }

        // Recursion
        for	(int i = 1; i < P; i++) {
            phn_temp = KW_temp[i];
            phn_mean = phonemeLengthMeanVector[phn_temp];
            phn_std = phonemeLengthStdVector[phn_temp];
            Prev_phn_mean = phonemeLengthMeanVector[KW_temp[i-1]];

            /*
            Phn_min_Length = int(phoneme_length_mean[keyword.KWsPhnInds[KW_Num][i]]-phoneme_length_std[keyword.KWsPhnInds[KW_Num][i]])-1;
            Phn_max_Length = int(phoneme_length_mean[keyword.KWsPhnInds[KW_Num][i]]+phoneme_length_std[keyword.KWsPhnInds[KW_Num][i]])+1;
            if (Phn_min_Length<min_num_frames)
            Phn_min_Length = min_num_frames;
            if (Phn_max_Length>max_num_frames)
            Phn_max_Length = max_num_frames;*/
            for (int t = s+i*minNumberOfFrames; t < _min(s+i*maxNumberOfFrames, T); t++) {
                int stop_l1_at = (t < maxNumberOfFrames) ? t : maxNumberOfFrames;
                for (int l1 = minNumberOfFrames; l1 <= stop_l1_at; l1++) {
                    D1 = weigths.subvector(0,phi_size-1) * phi_1(phn_mean, phn_std, P, phn_temp,t,l1);
                    D2_max = MISPAR_KATAN_MEOD;
                    /*
                    int Prev_Phn_min_Length = int(phoneme_length_mean[keyword.KWsPhnInds[KW_Num][i-1]]-phoneme_length_std[keyword.KWsPhnInds[KW_Num][i-1]])-1;
                    int Prev_Phn_max_Length = int(phoneme_length_mean[keyword.KWsPhnInds[KW_Num][i-1]]+phoneme_length_std[keyword.KWsPhnInds[KW_Num][i-1]])+1;
                    if (Prev_Phn_min_Length<min_num_frames)
                    Prev_Phn_min_Length = min_num_frames;
                    if (Prev_Phn_max_Length>max_num_frames)
                    Prev_Phn_max_Length = max_num_frames;*/
                    for (int l2 = minNumberOfFrames; l2 <= maxNumberOfFrames; l2++) {
                        D2 = D0(i-1,t-l1,l2) + weigths(phi_size-1) * phi_2(phn_mean, Prev_phn_mean, P,t,l1,l2);
                        My_tmp_Var = D0(i-1,t-l1,l2);
                        //My_tmp_Var = w(phi_size-1) * phi_2(x, phn_mean, Prev_phn_mean, P, t,l1,l2);
                        if (D2 > D2_max) {
                            D2_max = D2;
                            prev_l(i,t,l1) = l2;
                            prev_t(i,t,l1) = t-l1;
                        }
                    }
                    D0(i,t,l1) = D1 + D2_max;
                    //cout << "D0= " << D1 + D2_max << endl;

                }
            }
        }

        // Termination
        D2_max = MISPAR_KATAN_MEOD;
        /*
        Phn_min_Length = int(phoneme_length_mean[keyword.KWsPhnInds[KW_Num][P-1]]-phoneme_length_std[keyword.KWsPhnInds[KW_Num][P-1]])-1;
        Phn_max_Length = int(phoneme_length_mean[keyword.KWsPhnInds[KW_Num][P-1]]+phoneme_length_std[keyword.KWsPhnInds[KW_Num][P-1]])+1;
        if (Phn_min_Length<min_num_frames)
        Phn_min_Length = min_num_frames;
        if (Phn_max_Length>max_num_frames)
        Phn_max_Length = max_num_frames;
        */
#if 1	 
        for (int t=s+(P-1)*minNumberOfFrames; t<_min(s+(P-1)*maxNumberOfFrames, T); t++)  {
#else
        { int t = T-1;
#endif
            for (int l = minNumberOfFrames; l <= maxNumberOfFrames; l++) {
                if (D0(P-1,t,l) > D2_max) {
                    D2_max = D0(P-1,t,l);
                    pred_l[P-1] = l;
                    pred_t[P-1] = t;
                }
            }
        }
        y_hat[P-1] = pred_t[P-1]-pred_l[P-1]+1;
        // Back-tracking
        for (short p = P-2; p >= 0; p--) {
            pred_l[p] = prev_l(p+1,pred_t[p+1],pred_l[p+1]);
            pred_t[p] = prev_t(p+1,pred_t[p+1],pred_l[p+1]);
            y_hat[p] = pred_t[p]-pred_l[p]+1;
        }
        y_hat[0] = s;
        end_frame = pred_t[P-1];

        // apply normalization
#ifdef NORM_TYPE2
        D2_max /= (end_frame-s+1);
#endif	
        /*
                if (D2_max > D0_best_best)
                {
                    std::cout << "s=" << s << " D2_max=" << D2_max << " D0_best=" << D0_best_best << std::endl;
                    D0_best_best = D2_max;
                    //for (int i = 0; i < P; i++)
                    //	y_hat_best_best[KW_Num][i] = y_hat[i];
                    //y_hat_best_best[KW_Num][P] = end_frame;

                }
*/
        s += s_Step;
        //s += s_Step;
        s_start = s;



        if (D2_max>=userDefinedThreshold)
            if (D2_max > D0_best)
            {
                D0_best = D2_max;
                best_s = s;
                for (int i = 0; i < P; i++)
                    y_hat_best[i] = y_hat[i];
                y_hat_best[P] = end_frame;
                Prev_While_Status = 1;
            }
            else
            {
                if (Num_Following_Searched == 3)
                {
                    y_hat_best_Tmp.clear();
                    y_hat_best_Tmp.resize(P+1);
                    for (int i = 0; i <= P; i++)
                        y_hat_best_Tmp[i] = y_hat_best[i];
                    //                    cout << "keyword=/" << key_w.KWPhnInds << "/ Found!!!!!"<< endl;
                    //                    cout << "alignment= " << y_hat_best_Tmp << endl;
                    //                    cout << "confidence= " << D0_best << endl;
                    timeAligns.push_back(y_hat_best_Tmp);
                    confidence.push_back(D0_best);
                    //cout << "T= " << T << endl;
                    D0_best = MISPAR_KATAN_MEOD;
                    s_start = y_hat_best[P];
                    Prev_While_Status = 0;
                    Num_Following_Searched = 0;
                }
                else
                    Num_Following_Searched += 1;
            }
        else
            if (Prev_While_Status==1)
            {
                //cout << "Intered Prev_While_Status" << endl;
                //cout << "Num_Following_Searched = " << Num_Following_Searched[KW_Num] << endl;
                if(Num_Following_Searched == 3)
                {
                    y_hat_best_Tmp.clear();
                    y_hat_best_Tmp.resize(P+1);
                    for (int i = 0; i <= P; i++)
                        y_hat_best_Tmp[i] = y_hat_best[i];
                    //					cout << "keyword=/" << key_w.KWPhnInds << "/ Found!!!!!"<< endl;
                    //					cout << "alignment= " << y_hat_best_Tmp << endl;
                    //					cout << "confidence= " << D0_best << endl;
                    timeAligns.push_back(y_hat_best_Tmp);
                    confidence.push_back(D0_best);
                    //cout << "T= " << T << endl;
                    D0_best = MISPAR_KATAN_MEOD;
                    s_start = y_hat_best[P];
                    Prev_While_Status = 0;
                    Num_Following_Searched = 0;
                }
                else
                    Num_Following_Searched += 1;
            }
        s = s_start;
    }


    delete [] pred_l;
    delete[] pred_t;
}

/************************************************************************
Function:     gaussian

Description:  Gaussian PDF
Inputs:       double x, double mean, double std
Output:       double.
Comments:     none.
***********************************************************************/
double KeywordClassifier::gaussian(const double x, const double mean, const double std)
{
    double d = (1/sqrt(2*3.141529)/std * exp(-((x-mean)*(x-mean)) / (2*std*std) ));
    return (d);
}


// --------------------- End of KWS Classifier ------------------------------------//

/************************************************************************
Function:     Classifier_Phn::Classifier_Phn

Description:  Constructor
Inputs:       none.
Output:       none.
Comments:     none.
***********************************************************************/
PhonemeClassifier::PhonemeClassifier()
{
    numOfModels = 30;
    maxInputBufferLength = 50000;
    instance_index = 0;
    ranks.resize(numOfModels);
} 

/************************************************************************
Function:     Classifier_Phn::~Classifier_Phn

Description:  Destructor
Inputs:       none.
Output:       none.
Comments:     none.
***********************************************************************/
PhonemeClassifier::~PhonemeClassifier()
{
}
/************************************************************************
Function:     ceps_dist

Description:  Euclidian distance between feature vectors
Inputs:       SpeechUtterance_KWS& x
Output:       void.
Comments:     this function is added from HTK-ceps-dist file
***********************************************************************/
void PhonemeClassifier::ceps_dist(Dataset& x)
{
    // Compute all distances. Run over all possible alignment landmarks y_i
    infra::vector w(last_s);

    for (int Frame_Num = x.distances_index+1; Frame_Num<x.Last_Frame; Frame_Num++)
    {


        for (int s = 1; s <= last_s; s++) {
            // define -1 to be the distance of undeined regions
            if (Frame_Num-s < 0 || Frame_Num+s-1 >= x.Ind_Filled) {
                w(s-1) = -1;
            }
            else {
                w(s-1) = 0;
                for (int j=1; j <= s; j++)
                    w(s-1) += (x.Features.row(Frame_Num-j) - x.Features.row(Frame_Num+j-1)).norm2();
                w(s-1) /= s;
            }
            (*Dataset::distances)(Frame_Num, s-1) = w(s-1);
        }

        x.distances_index = Frame_Num;
    }

}

/************************************************************************
Function:     Classifier::predict

Description:  Predict label of instance x
Inputs:       InstanceType &x 
LabelType &y_hat
Output:       void
Comments:     none.
***********************************************************************/
void PhonemeClassifier::predict(Dataset& dataset)
{
    char *Exception;
    double *Scores, *In_Frame_Adress;
    int Num_Phns_Temp = dataset.Num_Phns;

    // GMM Phoneme Classifier Prediction
    for (int Frame_Num = dataset.First_Frame; Frame_Num<dataset.Last_Frame; Frame_Num++)
    {
        Scores = new double [numOfModels];
        In_Frame_Adress = dataset.Features_Circular[Frame_Num%maxInputBufferLength];
        Exception = CalculateLikelihoodUsingBufferedModels(ModelsBuffer, numOfModels,
                                                           In_Frame_Adress, 1, &Scores[0]);
        if (Exception!=NULL)
        {
            printf("Error in TrainModelFromArr: %s\n", Exception);
            //getch();
            exit(-1);
        }
        for (int i = 0; i<Num_Phns_Temp; i++)
            ranks(i) = Scores[i];


        // Hierarchical Discriminative Phoneme Classifier Prediction
        /*
        if (averaging_enabled)
        ranks = K( support(), instance ) * scaled_alpha;
        else
        ranks = K( support(), instance ) * alpha();
        */

        double max_scores = ranks.max();
        double min_scores = ranks.min();
        //max_scores =1;
#ifdef NORM_SCORES_0_1
        for (uint j = 0; j < ranks.size(); j++)
            (*Dataset::scores)(Frame_Num, j) = (ranks(j)-min_scores)/(max_scores-min_scores);
#else
        x.scores(Frame_num, j) = ranks(j)-max_scores;
#endif
        //FeatureSegments_KWS::scores_index = Frame_num;
        delete[] Scores;
    }
}

/************************************************************************
Function:     Classifier::averaging

Description:  Prepare alpha for averaging
Inputs:       none.
Output:       none.
Comments:     none.
***********************************************************************/
void PhonemeClassifier::averaging()
{
    //    scaled_alpha.resize(alpha().height(), alpha().width());
    //    scaled_alpha = alpha();
    //    for (uint i=0; i < scaled_alpha.height(); i++) {
    //        double scaling = (alpha.num_examples()-alpha.get_logical_index(i))/double(alpha.num_examples());
    //        for (uint j=0; j < scaled_alpha.width(); j++) {
    //            scaled_alpha(i,j) *= scaling;
    //        }
    //    }
    //    averaging_enabled = true;
}

/************************************************************************
Function:     Classifier::load

Description:  Loads a classifier 
Inputs:       string & filename
Output:       none.
Comments:     none.
***********************************************************************/
void PhonemeClassifier::loadPhonemeClassifier(std::string &filename)
{
    /* GMM Phoneme Classifier loading*/
    char line[1000];
    int Model_file_size;
    FILE *modelFileName;

    ifstream ifs_Model_filename_list;
    ifs_Model_filename_list.open(filename.c_str());
    if (!ifs_Model_filename_list.is_open())
    {
        cerr << "Error: Unable to open model file list " << filename << " for reading. Aborting..." << endl;
        //getch();
        exit(-1);
    }
    // Reading model files, and writing in ModelsBuffer after memory allocation
    ModelsBuffer = new float *[numOfModels];
    char cwd[1024];
    for (int i = 0; i < numOfModels; i++)
    {
        ifs_Model_filename_list.getline(line, MAX_LINE_SIZE);
        getcwd(cwd, sizeof(cwd));
#ifdef Q_OS_LINUX
        line[strlen(line) - 1] = 0;
#else
        line[strlen(line) - 1] = 0;
#endif
        strcat(cwd, line);
        modelFileName = fopen(cwd, "rb");
        if (modelFileName == NULL)
        {
            cout << "Error: Unable to open file list " << cwd << endl;
            exit(-1);
        }

        fseek(modelFileName, 0, SEEK_END);
        Model_file_size = ftell(modelFileName)/sizeof(float);
        fseek(modelFileName, 0, SEEK_SET);
        ModelsBuffer[i] = new float[Model_file_size];
        fread(ModelsBuffer[i], sizeof(float), Model_file_size,  modelFileName);
        fclose(modelFileName);
    }

    last_s = 4;
}

/************************************************************************
Function:     Classifier::free_Arrays

Description:  Free classfier arrays
Inputs:       string & filename
Output:       none.
Comments:     none.
***********************************************************************/
void PhonemeClassifier::deletePhonemeClassifier(void)
{
    for (int i=0; i<numOfModels; i++)
        delete[] ModelsBuffer[i];

    delete[] ModelsBuffer;
}
// --------------------- EOF ------------------------------------//


