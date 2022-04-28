/*****************************************************************************/
/**
 * @file PiController.h
 * @brief PiController header file.
 *
 * @author Tomas Correa (TPC)
 *
 * @project Smart Battery
 *
 * @copyright Universidade Federal de Minas Gerais. All rights reserved
 *****************************************************************************/

#ifndef APPLICATION_CONTROL_PI_CONTROLLER_H_
#define APPLICATION_CONTROL_PI_CONTROLLER_H_

#include <stdbool.h>
#include <stdint.h>

typedef struct {

    bool enab;

    float feedback;
    float setpoint;
    float feedforward;
    float output;

    float prop;
    float integr;

    float preSat;
    float satError;
    float integrInit;

    float error;

    float setOutMin;
    float setOutMax;

    float outMin;
    float outMax;

    float Kp;
    float Ki;
    float Kc;

    float setKp;
    float setKi;
    float setKc;

    float Ts;

} PiController;

typedef enum
{
    PI_NOT_SAT = 0,
    PI_POSITIVE_SAT,
    PI_NEGATIVE_SAT
} PI_SAT;

void PI_init(PiController*, const float, const float, const float, const float, const float, const float);
uint16_t PI_run(PiController*);
void PI_enable(PiController*);
void PI_disable(PiController*);
void PI_reset(PiController*);

#endif /* APPLICATION_CONTROL_PI_CONTROLLER_H_ */
