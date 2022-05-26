/*
 * SVPWM.h
 *
 *  Created on: 28 de abr de 2022
 *      Author: Hudson
 */

#ifndef SVPWM_H_
#define SVPWM_H_

#include <math.h>

void svpwm_bi(float* ,float* ,float* ,float* ,float* ,float* );

void svpwm(float* , float* , float* ,float* ,float* ,float* );

void signal_gen(float ,float , float , float , float ,float* ,float* );

typedef float float32_t;

typedef enum
{
    INCREASING,
    DOWNING,
    FINISHED,
} incStatus;

#define DCL_REFGEN_DEFAULTS { 0.0f,  0.0f, \
                              0.0f,  0.0f, \
                              0.0f,  0.0f,  1.0f, \
                              1.0f,  60.0f,  0.0f,  0.0f, 0.0f, \
                              0.0f,  0.0f,  0.0f,  0.0f, \
                              FINISHED}

typedef volatile struct dcl_refgen {
    float32_t   amtgt;      //!< Target amplitude value
    float32_t   aminc;      //!< Amplitude increment
    float32_t   fmtgt;      //!< Frequency ramp value
    float32_t   fminc;      //!< Frequency increment
    float32_t   thinc;      //!< Angular increment
    float32_t   ts;       //!< Pulse duty cycle
    float32_t   umax;       //!< Maximum allowable output
    float32_t   umin;       //!< Minimum allowable output
    float32_t   freq_nom;         //!< nominal frequency
    float32_t   ampl;       //!< Dynamic amplitude
    float32_t   freq;       //!< Dynamic frequency
    float32_t   freqtgt;       //!< Dynamic frequency target
    float32_t   theta;      //!< Normalized angle - phase A
    float32_t   thetb;      //!< Normalized angle - phase B
    float32_t   ya;         //!< Phase A output
    float32_t   yb;         //!< Phase B output
    incStatus status;             //     INCREASING, DOWNING, FINISHED

    //DCL_CSS     *css;       //!< Pointer to the common support structure
} DCL_REFGEN;

static inline void DCL_resetRefgen(DCL_REFGEN *p)
{
    p->ampl = 0.0f;
    p->freq = 0.0f;
    p->theta = 0.0f;
    p->thetb = 0.0f;
    p->thinc = 0.0f;
    p->ya = 0.0f;
    p->yb = 0.0f;
    p->status = FINISHED;
    p->umax = 0.707;
    p->umin = 0.707;
}

static inline void DCL_setRefgen(DCL_REFGEN *p,float32_t final_ampl,float32_t freq_nom, float32_t final_freq, float32_t ramp_time, float32_t sample_time)
{
    p->amtgt = final_ampl;
    p->freqtgt = final_freq;
    p->aminc = (final_ampl-(p->ampl))/(ramp_time/sample_time);
    p->fminc = (final_freq-(p->freq))/(ramp_time/sample_time);
    p->freq_nom = freq_nom;
    p->ts = sample_time;
    //p->theta = 0.0f;
    //p->thetb = 0.0f;
    //p->thinc = 0.0f;
    //p->ya = 0.0f;
    //p->yb = M_PI/2;
    p->status = FINISHED;

    //if (labs((final_ampl-(p->ampl))) > (ramp_time/sample_time)){
    if (final_ampl > p->ampl){
        p->status = INCREASING;
    }
    if (final_ampl < p->ampl){
        p->status = DOWNING;
    }

}

static inline void run_Refgen(DCL_REFGEN *p,float* V_alpha, float* V_beta){
    if ((p->status)==INCREASING){
        p->ampl = p->ampl + p->aminc;
        p->freq = p->freq + p->fminc;
        if ((p->ampl) >= p->amtgt){
            p->ampl = p->amtgt;
            p->freq = p->freqtgt;
            p->status=FINISHED;
        }
    }

    if ((p->status)==DOWNING){
        p->ampl = p->ampl + p->aminc;
        p->freq = p->freq + p->fminc;
        if ((p->ampl) <= p->amtgt){
            p->ampl = p->amtgt;
            p->freq = p->freqtgt;
            p->status=FINISHED;
        }
    }

    p->theta = p->freq*(p->ts)+p->theta;
    p->thetb = p->freq*(p->ts)+p->thetb;
    if ((p->theta)>=2.0*M_PI){
        p->theta = 0;
    }
    if ((p->thetb)>=2.0*M_PI){
        p->thetb = 0;
    }
    *V_alpha = p->ampl*cos(p->theta);
    *V_beta = p->ampl*sin(p->theta);
}

#endif /* SVPWM_H_ */
