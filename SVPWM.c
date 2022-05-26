/*
 * SVPWM.c
 *
 *  Created on: 28 de abr de 2022
 *      Author: Hudson
 *      Space Vector PWM functions to calculate the duty cicle
 *      void svpwm_bi is for two phase motor
 *      void svpwm is for three phase motor
 *
 *
 */
#include <SVPWM.h>


static float U1=1,U2=1,teta1=2*M_PI,teta2=1.5*M_PI;
static float t1,t2,Uref,toff,k;
static int setor;
extern float V_alpha, V_beta, teta;

//svpwm_bi for two phase motor
void svpwm_bi(float* teta, float* V_alpha, float* V_beta,float* wma,float* wmb,float* wmc){
/*
    //Calculates theta *******************************
    Uref=sqrt((*V_alpha)*(*V_alpha)+(*V_beta)*(*V_beta));

    if (Uref==0)
    {
         *teta=0;
    }
         else
         {
             *teta=acosf(abs((*V_alpha)/Uref));
    }



     if ((*V_alpha<0)&(*V_beta>0))
        {
         *teta=M_PI-*teta;
        }
     if ((*V_alpha<0)&&(*V_beta<0))
     {
         *teta=M_PI+*teta;
     }
     if ((*V_alpha>0)&&(*V_beta<0))
     {
         *teta=2*M_PI-*teta;
     }
     */
//*******************************
    setor=6;
    U1=1.0;
    U2=1.0;
    teta1=0.0;
    teta2=280.0*M_PI/180.0;

    if ((*teta>=0.0)*(*teta<M_PI/4))
    {
        U1=1.0;
        U2=sqrtf(2.0);
        teta1=0;
        teta2=M_PI/4.0;
        setor=1;
    }
    if ((*teta>=M_PI/4)*(*teta<M_PI/2))
    {
        U1=1.0;
        U2=sqrtf(2.0);
        teta1=M_PI/2.0;
        teta2=M_PI/4.0;
        setor=2;
    }
    if ((*teta>=M_PI/2)*(*teta<M_PI))
            {
        U1=1.0;
        U2=1.0;
        teta1=M_PI/2.0;
        teta2=M_PI;
        setor=3;
    }
    if ((*teta>=M_PI)*(*teta<5*M_PI/4))
    {
        U1=sqrtf(2.0);
        U2=1.0;
        teta1=M_PI*1.25;
        teta2=M_PI;
        setor=4;
    }
    if ((*teta>=5*M_PI/4)*(*teta<3*M_PI/2))
        {
        U1=sqrtf(2.0);
        U2=1.0;
        teta1=M_PI*1.25;
        teta2=M_PI*1.5;
        setor=5;
        }


    Uref = sqrtf(*V_alpha*(*V_alpha)+*V_beta*(*V_beta));//powf(*V_alpha,2)+powf(*V_beta,2));//*V_alpha*(*V_alpha)+*V_beta*(*V_beta));

    t1=(Uref/U1)*(sinf(teta2-*teta)/sinf(teta2-teta1));
    t2=(Uref/U2)*(sinf(*teta-teta1)/sinf(teta2-teta1));
    toff=1.0-t1-t2;


    if (setor==1)
        {
        *wma=1-toff/2.0;
        *wmb=1-toff/2.0-t1-t2;
        *wmc=1-toff/2.0-t1;
        }
    if (setor==2)
        {
        *wma=1-toff/2.0-t1;
        *wmb=1-toff/2.0-t1-t2;
        *wmc=1-toff/2.0;
        }
    if (setor==3)
        {
        *wma=1-toff/2.0-t1-t2;
        *wmb=1-toff/2.0-t1;
        *wmc=1-toff/2.0;
        }
    if (setor==4)
        {
        *wma=1-toff/2.0-t1-t2;
        *wmb=1-toff/2.0;
        *wmc=1-toff/2.0-t1;
        }
    if (setor==5)
        {
        *wma=1-toff/2.0-t1;
        *wmb=1-toff/2.0;
        *wmc=1-toff/2.0-t1-t2;
        }
    if (setor==6)
        {
        *wma=1-toff/2.0;
        *wmb=1-toff/2.0-t1;
        *wmc=1-toff/2.0-t1-t2;
        }

}

//
void svpwm(float* teta, float* V_alpha, float* V_beta,float* wma,float* wmb,float* wmc){

    Uref=sqrt((*V_alpha)*(*V_alpha)+(*V_beta)*(*V_beta));

    if (Uref==0)
    {
         *teta=0;
    }
         else
         {
             *teta=acosf(abs((*V_alpha)/Uref));
    }


     if ((*V_alpha<0)&(*V_beta>0))
        {
         *teta=M_PI-*teta;
        }
     if ((*V_alpha<0)&&(*V_beta<0))
     {
         *teta=M_PI+*teta;
     }
     if ((*V_alpha>0)&&(*V_beta<0))
     {
         *teta=2*M_PI-*teta;
     }


    if ((*teta>=0)&(*teta<60*M_PI/180))
    {
        teta1=0;
        teta2=60*M_PI/180;
        setor=1;
    }
    if ((*teta>=60*M_PI/180)&(*teta<120*M_PI/180))
    {
        teta1=60*M_PI/180;
        teta2=120*M_PI/180;
        setor=2;
    }
    if ((*teta>=120*M_PI/180)&(*teta<M_PI))
    {
        teta1=120*M_PI/180;
        teta2=M_PI;
        setor=3;
    }
    if ((*teta>=M_PI)&(*teta<240*M_PI/180))
    {
        teta1=M_PI;
        teta2=M_PI*240/180;
        setor=4;
    }
    if ((*teta>=240*M_PI/180)&(*teta<300*M_PI/180))
    {
        teta1=M_PI*240/180;
        teta2=M_PI*300/180;
        setor=5;
    }
    if ((*teta>=300*M_PI/180)&(*teta<=2*M_PI))
    {
        teta1=M_PI*300/180;
        teta2=2*M_PI;
        setor=6;
    }


    k=cos(teta1)*sin(teta2)-cos(teta2)*sin(teta1);
    t1=(sin(teta2)**V_alpha)/k-(cos(teta2)**V_beta)/k;
    t2=-sin(teta1)**V_alpha/k+cos(teta1)**V_beta/k;
    toff=1-t1-t2;


    if (setor==1)
    {
        *wma=t1+t2+toff/2;
        *wmb=t2+toff/2;
        *wmc=toff/2;
            }
    if (setor==2)
    {
        *wma=t1+toff/2;
        *wmb=t1+t2+toff/2;
        *wmc=toff/2;
    }
    if (setor==3)
    {
        *wma=toff/2;
        *wmb=t1+t2+toff/2;
        *wmc=t2+toff/2;
    }
    if (setor==4)
    {
        *wma=toff/2;
        *wmb=t1+toff/2;
        *wmc=t1+t2+toff/2;
    }
    if (setor==5)
    {
        *wma=t2+toff/2;
        *wmb=toff/2;
        *wmc=toff/2+t1+t2;
    }
    if (setor==6)
    {
        *wma=t1+t2+toff/2;
        *wmb=toff/2;
        *wmc=toff/2+t1;
    }
}

