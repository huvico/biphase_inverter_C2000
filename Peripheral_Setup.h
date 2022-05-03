/*
 * Peripheral_Setup.h
 *
 *  Created on: 28 de abr de 2022
 *      Author: Hudson
 */

#ifndef PERIPHERAL_SETUP_H_
#define PERIPHERAL_SETUP_H_
#include "F28x_Project.h"


#define RISE_TIME 150;
#define FALL_TIME 150;
//SWITCH_PERIOD=200000/(4*f_switch));//calculates PRD to configures switch period. If isnt up and down, chage to 200000/(2*f_switch)
#define SWITCH_PERIOD 25000;

void Setup_GPIO(void);
void Setup_PWM(void);
void Setup_ADC(void);
void Setup_DAC(void);





#endif /* PERIPHERAL_SETUP_H_ */
