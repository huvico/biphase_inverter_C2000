/*
 * Variables.h
 *
 *  Created on: 25 de mai de 2022
 *      Author: Hudson
 */

#ifndef VARIABLES_H_
#define VARIABLES_H_

// *****************************************************************************/
// Ramp declarations
#define START_TIME_MACHINE 20.0f
float TS_RefGen=0.5/(200000000.0/(4.0*(float)SWITCH_PERIOD));

#include <SVPWM.h>
DCL_REFGEN rgen = DCL_REFGEN_DEFAULTS;

//interrupt function of ADC
__interrupt void isr_adc(void);         //ADC interruption function

// SWITCH CONFIGURATIONS
//SWITCH_PERIOD = freq. De clock do processador / 2x freq. do PWM (x2 se for up and down)
//float SWITCH_PERIOD=5000;//switch period
float f_switch=2;//switch frequency in kHz

float adc1 = 0; //Ialfa
float adc2 = 0; //Ibeta
float adc3 = 0; //Vcc
float w_nom = 0;
float new_amp = 0.5;
float Imax = 10;
float soma = 0;
float frequencia = 0;
float wma = 0;
float wmb = 0;
float wmc = 0;
float modulo = 0;

// *****************************************************************************/

// *****************************************************************************/

unsigned char send = 0, turn_off_command = 0, turn_on_command = 0, set_new_ref = 0, calibration=0;
float V_alpha = 0.0, V_beta = 0.0, teta = 0;
// velocity calculation
unsigned long delta_pos = 0;
unsigned int new_pos = 0, old_pos = 0;
float rpm = 0;
float rpm_1 = 0, rpm_2 = 0, rpm_3 = 0, avg_rpm = 0;
int index_rpm = 0;
float period = 0;

// *****************************************************************************/
// plot variables
#define BUFFER_plot_size 200
float *p_adc = &avg_rpm;
float plot[BUFFER_plot_size];
float avg_plot = 0, sum_avg = 0;
uint32_t index = 0;
float Ts=1.0/60.0/(float)BUFFER_plot_size;


//#define Ts 20.8333333333f //sample time in us,timmer interruption

// *****************************************************************************/

// Offset variables for current measurements
float offset1 = 0;
float offset2 = 0;
float offset3 = 0;


#endif /* VARIABLES_H_ */
