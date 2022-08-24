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
#define START_TIME_MACHINE 5.0f
float TS_RefGen=0.5/(200000000.0/(4.0*(float)SWITCH_PERIOD));

#include <SVPWM.h>
DCL_REFGEN rgen = DCL_REFGEN_DEFAULTS;

//interrupt function of ADC
__interrupt void isr_adc(void);         //ADC interruption function

// SWITCH CONFIGURATIONS
//SWITCH_PERIOD = freq. De clock do processador / 2x freq. do PWM (x2 se for up and down)
//float SWITCH_PERIOD=5000;//switch period
//float f_switch=2;//switch frequency in kHz

float adc1 = 0; //Ialfa
float adc2 = 0; //Ibeta
float adc3 = 0; //Vcc
float sensor_1 = 45.371,sensor_2 = 44.62;
float w_nom = 0;
float new_amp = 0.2,new_f = 60;
float Imax = 30;
float soma = 0;
float frequencia = 0;
float wma = 0;
float wmb = 0;
float wmc = 0;
float modulo = 0;

// *****************************************************************************/

// *****************************************************************************/

unsigned char send = 0, turn_off_command = 0, turn_on_command = 0, set_new_ref = 0, calibration = 1, turnon_calibration = 0;
float V_alpha = 0.0, V_beta = 0.0, teta = 0;
// velocity calculation
unsigned long delta_pos = 0;
unsigned int new_pos = 0, old_pos = 0;
float rpm = 0, avg_rpm = 0;
float index_rpm = 0;
float period = 0;

// *****************************************************************************/
// plot variables
#define BUFFER_plot_size 200
float *p_adc = &adc1;
float plot[BUFFER_plot_size];
float avg_plot = 0, sum_avg = 0;

float *p_adc2 = &adc2;
float plot2[BUFFER_plot_size];
float avg_plot2 = 0, sum_avg2 = 0;

uint32_t index = 0;
float Ts=1.0/60.0/(float)BUFFER_plot_size; //sample timmer interruption


// *****************************************************************************/
// Offset variables for current measurements
float offset1 = 2290;
float offset2 = 2290;
float offset3 = 0;


// *****************************************************************************/
// Calibration variables
#define cal_points  3
int index_cal = 0, flag_Ra_OK = 0, flag_Rb_OK = 1, flag_AC = 0;
float ra_alpha[cal_points];
float ra_beta[cal_points],ia_cc[cal_points],ib_cc[cal_points];
float v_ref = 0.03, v_bus = 178.0;
#define delay_s 1.0f
#define delay_us 3000000



#endif /* VARIABLES_H_ */
