/*****************************************************************************/
/**
 * @file PiController.c
 * @brief Source file of ProportionalIntegral controller
 *
 * Implementation of the a Proportional Integral controller
 *
 * @author Tomas Correa (TPC)
 *
 * @project Smart Battery
 *
 * @copyright Universidade Federal de Minas Gerais. All rights reserved
 *****************************************************************************/


#include "PIController.h"

/*****************************************************************************/
/**
 * This function initializes the app instance, including all instances that it uses.
 *
 * @param   self is the PiController instance we are working on.
 *
 * @param   Kp is the proportional gain.
 *
 * @param   Ki is the integral gain.
 *
 * @param   Kc is the anti-windup gain.
 *
 * @param   outMin is the output minimum value.
 *
 * @param   outMax is the output maximum value.
 *
 * @param   Ts is the calculation rate in seconds.
 *
 * @return  None.
 *
 * @note    None.
 *
 *****************************************************************************/
void PI_init(PiController* self, const float Kp, const float Ki, const float Kc, const float outMin, const float outMax, const float Ts)
{
    self->setKp = Kp;
    self->setKi = Ki;
    self->setKc = Kc;
    self->setOutMin = outMin;
    self->setOutMax = outMax;
    self->Ts = Ts;
    self->integrInit = 0;
    self->integr = 0;
    self->feedforward = 0.0;
    self->Kp = self->setKp;
    self->Ki = self->setKi;
    self->Kc = self->setKc;
    self->outMin = self->setOutMin;
    self->outMax = self->setOutMax;
    self->integr = self->integrInit;

}

/*****************************************************************************/
/**
 * This function calculates the output according to the reference, feedback and
 * feedforward, if the controller is enabled. It should be called with the rate
 * specified by Ts.
 *
 * @param   Self is the instance we are working on.
 *
 * @return  None.
 *
 * @note    While disabled, the output is still calculated according to integrInit
 * and the feedforward.
 *
 *****************************************************************************/
#pragma CODE_SECTION(PI_run, ".TI.ramfunc")
uint16_t PI_run(PiController* self)
{
    uint16_t is_sat = PI_NOT_SAT;

    if(self->enab == 1)
    {
        self->error = self->setpoint -  self->feedback;
        self->prop = self->Kp * self->error;
        self->integr = self->integr + self->Ki * self->Ts * self->error  + self->Kc * self->Ts * self->satError; // Anti-Windup: + Kc_* self->Ts_* self->satError_

            if (self->integr > self->outMax) // Hard limits when Anti-Windup is insufficient
            {
                self->integr = self->outMax;
            }
            else if (self->integr < self->outMin)
            {
                self->integr = self->outMin;
            }
    }
    else
    {
        self->error = 0.0;
        self->prop = 0.0;
        self->integr = self->integrInit;
        self->satError = 0.0;
    }

    self->preSat = self->integr + self->prop + self->feedforward;

    if (self->preSat > self->outMax)
    {
        self->output = self->outMax;
        is_sat = PI_POSITIVE_SAT;
    }
    else if (self->preSat < self->outMin)
    {
        self->output = self->outMin;
        is_sat = PI_NEGATIVE_SAT;
    }
    else
    {
        self->output = self->preSat;
    }


    self->satError = self->output - self->preSat;

    return is_sat;
}

/*****************************************************************************/
/**
 * This function enables the controller.
 *
 * @param   self is the instance we are working on.
 *
 * @return  None.
 *
 * @note    Only when this functions is called, the gains and limits are updated.
 *
 *****************************************************************************/
void PI_enable(PiController* self)
{
    if (self->enab) return;
    self->Kp = self->setKp;
    self->Ki = self->setKi;
    self->Kc = self->setKc;
    self->outMin = self->setOutMin;
    self->outMax = self->setOutMax;
    self->integr = self->integrInit;
    self->satError = 0;
    self->enab = true;
}

/*****************************************************************************/
/**
 * This function disables the controller..
 *
 * @param   self is the instance we are working on.
 *
 * @return  None.
 *
 * @note    None.
 *
 *****************************************************************************/
void PI_disable(PiController* self)
{
    self->enab = false;
}

void PI_reset(PiController* self)
{
    PI_disable(self);
    self->feedback = 0;
    self->setpoint = 0;
    self->error = 0;
    self->prop = 0;
    self->integr = self->integrInit;
    self->output = 0;
    self->preSat = 0;
    self->satError = 0;
    self->feedforward = 0;

}


