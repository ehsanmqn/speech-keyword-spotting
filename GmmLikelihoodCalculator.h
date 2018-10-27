#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define PI   3.14159265358979
#define TPI  6.28318530717959     /* PI*2 */
#define LTPI 1.83787706640935		/* ln(2*PI) */
#define LZERO  (-1.0E10)   /* ~log(0) */
#define LSMALL (-0.5E10)   /* log values < LSMALL are set to LZERO */
#define MINLOGEXP (-23.025851)	/* -ln(-LZERO) */
#define MINEARG (-708.3)   /* lowest exp() arg  = log(MINLARG) */
#define MINLARG 2.45E-308  /* lowest log() arg  = exp(MINEARG) */
#define MINMIX  1.0E-5     /* Min usable mixture weight */
#define LMINMIX -11.5129254649702     /* log(MINMIX) */
#define PATHLEN	50
#define NAMELEN	30

typedef void * Ptr;

struct MixtureElemRec{		/* 1 of these per mixture */	
    double weight;        /* mixture weight */
    double *mean;			/* mean vector */
    double *var;			/* variance vector */
    double gConst;        /* Precomputed component of b(x) */
    int mIdx;            /* MixtureElem index */
    int nUse;            /* usage counter */
    Ptr hook;            /* general hook */
};

typedef struct MixtureElemRec *MixtureElem;

typedef struct {
    int nMix;            /* num mixtures in this model */
    int maxNumMix;		/* maximum number of mixture in this GMM that we must reach in. */
    int dimention;		/* dimention of the model. */
    MixtureElem *cpdf;	/* array[0..numMixtures-1] of Mixture */
    double *logWeight;	/* array[0..numMixtures-1] of log(weight) */
    Ptr hook;			/* general hook */
} GMMDef;

/* CalculateLikelihoodOnModel : calculate likelihood of featureVectors on gmm model g and return it. */
double CalculateLikelihoodOnModel(GMMDef *g, double *featureVectors, int nRows)
{
    int i, t, m, nMix, dim;
    double sum, xmm, *mean, *var, *logWeight, score;
    double prob, x, diff, temp;
    double *obs;
    MixtureElem me;
    double wght = 0.0;

    MixtureElem *cpdf = g->cpdf;
    nMix = g->nMix;
    dim = g->dimention;
    logWeight = g->logWeight;
    score = 0;

    /* calculate logWeight for all mixtuers. */
    for (m = 0; m < nMix; m++)
        logWeight[m] = log(cpdf[m]->weight);

    obs = featureVectors;
    /* loop on all frame */
    for (t = 0; t < nRows; t++)
    {
        prob = LZERO;
        /* loop on all mix.*/
        for (m = 0; m < nMix; m++)
        {
            /* select mixture m th.*/
            me = cpdf[m];
            wght = me->weight;
            mean = me->mean;
            var = me->var;
            if (wght > MINMIX)
            {
                sum = me->gConst;
                /* loop on dimentions. */
                for (i = 0; i < dim; i++)
                {
                    xmm = obs[i] - mean[i];
                    sum += xmm * xmm / var[i];
                }
                x = -0.5 * sum;
                x += logWeight[m];

                if (prob < x)
                {
                    temp = x;
                    diff = prob - x;
                }
                else
                {
                    temp = prob;
                    diff = x - prob;
                }
                if (diff < MINLOGEXP)
                    prob = temp < LSMALL ? LZERO : temp;
                else
                    prob = temp + log(1.0 + exp(diff));
            }
        }
        score += prob;
        obs += dim;
    }
    return score / nRows;
}

/* CreateGMM : this function create a gmm model with nMix mixture and dimention of dimention. */ 
GMMDef *CreateGMM(int nMix, int dimention)
{
    int ds = sizeof(double);
    GMMDef *g = NULL;
    if ((g = (GMMDef*) malloc(sizeof(GMMDef))) == NULL)
        throw "CreateGMM : Cannot allocate memory for gmm.";
    g->maxNumMix = g->nMix = nMix;
    g->dimention = dimention;
    if ((g->logWeight = (double*) malloc(nMix * ds)) == NULL)
        throw "CreateGMM : Cannot allocate memory for g->logWeight.";
    if ((g->cpdf = (MixtureElem *) malloc(nMix * sizeof(void*))) == NULL)
        throw "CreateGMM : Cannot allocate memory for g->cpdf.";
    for(int i = 0; i < nMix; i++)
    {
        if ((g->cpdf[i] = (MixtureElem) malloc(sizeof(MixtureElemRec))) == NULL)
            throw "CreateGMM : Cannot allocate memory for g->cpdf[i].";
        if ((g->cpdf[i]->mean = (double*) malloc(dimention * ds)) == NULL)
            throw "CreateGMM : Cannot allocate memory for gmm->cpdf[i]->mean.";
        if ((g->cpdf[i]->var = (double*) malloc(dimention * ds)) == NULL)
            throw "CreateGMM : Cannot allocate memory for gmm->cpdf[i]->var.";
    }
    return g;
}

/* DeleteGMM : this function delete all memory that allocated for gmm model g. */
void DeleteGMM(GMMDef *g)
{
    if (g == NULL)
        return;
    if (g->logWeight != NULL)
        free(g->logWeight);
    if (g->cpdf != NULL)
    {
        for(int i = 0; i < g->nMix; i++)
        {

            if (g->cpdf[i]  != NULL)
            {
                if (g->cpdf[i]->mean != NULL)
                    free(g->cpdf[i]->mean);
                if (g->cpdf[i]->var != NULL)
                    free(g->cpdf[i]->var);
                free(g->cpdf[i]);
            }
        }
        free(g->cpdf);
    }
    free(g);
}

void LoadGMMDefFromBuffer(GMMDef *g, float *buffer)
{
    int i, j, *ptr, nMix, dimention;
    float *fptr = buffer;
    ptr = (int*) buffer;
    // read two int for nMix, dimention
    g->nMix = nMix = ptr[0];
    g->dimention = dimention = ptr[1];
    // go to the first floating point
    fptr += 2;
    for (i = 0; i < nMix; i++)
    {
        g->cpdf[i]->weight = fptr[0];
        fptr++;
        for(j = 0; j < dimention; j++)
        {
            g->cpdf[i]->mean[j] = fptr[j];
        }
        fptr += dimention;
        for(j = 0; j < dimention; j++)
        {
            g->cpdf[i]->var[j] = fptr[j];
        }
        fptr += dimention;
        g->cpdf[i]->gConst = fptr[0];
        fptr++;
    }
}

char* CalculateLikelihoodUsingBufferedModels(float** modelsBuffer, int numOfModels, double *featuresArray, int numberOfFrames, double* scores)
{
    int nMix, dimention, i;
    //double score;
    int *ptr;
    GMMDef *g;
    // در این حالت مدلی برای محاسبه وجود ندارد. احتمالا اشتباها فراخوانی شده است.
    if(numOfModels <= 0)
    {
        return NULL;
    }
    ptr = (int*) modelsBuffer[0];
    try
    {
        nMix = ptr[0];
        dimention = ptr[1];
        g = CreateGMM(nMix, dimention);
        for(i = 0; i < numOfModels; i++)
        {
            LoadGMMDefFromBuffer(g, modelsBuffer[i]);
            scores[i] = CalculateLikelihoodOnModel(g, featuresArray, numberOfFrames);
        }
        DeleteGMM(g);
    }
    catch(char *s)
    {
        DeleteGMM(g);
        return s;
    }
    return NULL;
}
