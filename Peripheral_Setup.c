/*
 * Peripheral_Setup.c
 *
 *  Created on: 17 de mar de 2021
 *      Author: Hudson
 */


#include "Peripheral_Setup.h"

void Setup_GPIO(void){
// MESMO DO EXEMPLO GPIO SETUP DO C2000!!
    //
      // These can be combined into single statements for improved
      // code efficiency.
      //

      //
      // Enable PWM1-3
      //
      EALLOW;

      GpioCtrlRegs.GPAGMUX1.bit.GPIO0 = 0;
      GpioCtrlRegs.GPAGMUX1.bit.GPIO1 = 0;
      GpioCtrlRegs.GPAGMUX1.bit.GPIO2 = 0;
      GpioCtrlRegs.GPAGMUX1.bit.GPIO3 = 0;
      GpioCtrlRegs.GPAGMUX1.bit.GPIO4 = 0;
      GpioCtrlRegs.GPAGMUX1.bit.GPIO5 = 0;

      GpioCtrlRegs.GPAMUX1.bit.GPIO0 = 1;  // GPIO0 = PWM1A
      GpioCtrlRegs.GPAMUX1.bit.GPIO1 = 1;  // GPIO1 = PWM1B
      GpioCtrlRegs.GPAMUX1.bit.GPIO2 = 1;  // GPIO2 = PWM2A
      GpioCtrlRegs.GPAMUX1.bit.GPIO3 = 1;  // GPIO3 = PWM2B
      GpioCtrlRegs.GPAMUX1.bit.GPIO4 = 1;  // GPIO4 = PWM3A
      GpioCtrlRegs.GPAMUX1.bit.GPIO5 = 1;  // GPIO5 = PWM3B

      GpioCtrlRegs.GPAPUD.bit.GPIO0 = 1;   // 0 Enable, 1 Disable pullup on GPIO0
      GpioCtrlRegs.GPAPUD.bit.GPIO1 = 1;   // Enable pullup on GPIO1
      GpioCtrlRegs.GPAPUD.bit.GPIO2 = 1;   // Enable pullup on GPIO2
      GpioCtrlRegs.GPAPUD.bit.GPIO3 = 1;   // Enable pullup on GPIO3
      GpioCtrlRegs.GPAPUD.bit.GPIO4 = 1;   // Enable pullup on GPIO4
      GpioCtrlRegs.GPAPUD.bit.GPIO5 = 1;   // Enable pullup on GPIO5


      //configurar o gpio6 para medição de tempo
      GpioCtrlRegs.GPAPUD.bit.GPIO6 = 0;   // Enable pullup on GPIO6, PIN 80, J8
      GpioDataRegs.GPASET.bit.GPIO6 = 1;   // Load output latch
      GpioCtrlRegs.GPAMUX1.bit.GPIO6 = 0;  // GPIO6 = GPIO6
      GpioCtrlRegs.GPADIR.bit.GPIO6 = 1;   // GPIO6 = output

      //configures eQEP GPIO10/pin67
      GpioCtrlRegs.GPAPUD.bit.GPIO10 = 1;   //disable pull-up on GPIO10
      GpioCtrlRegs.GPAQSEL1.bit.GPIO10 = 0; //sync GPIO10 to SYSCLK
      GpioCtrlRegs.GPAGMUX1.bit.GPIO10 = 1; //configures GPIO10 as eQEP1A
      GpioCtrlRegs.GPAGMUX1.bit.GPIO10 = 1; //configures GPIO10 as eQEP1A


      EDIS;

}

void Setup_PWM(void){
    EALLOW;
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 0; // disable the sync clock
    CpuSysRegs.PCLKCR2.bit.EPWM1 = 1; // Enables PWM1 clock
    CpuSysRegs.PCLKCR2.bit.EPWM2 = 1; // Enables PWM2 clock
    CpuSysRegs.PCLKCR2.bit.EPWM3 = 1; // Enables PWM3 clock

    //Setup PWM1 ============================================================================================
    EPwm1Regs.TBPRD = SWITCH_PERIOD; // Set the timer period
    EPwm1Regs.CMPA.bit.CMPA = EPwm1Regs.TBPRD >> 1;//set the width pulse
    EPwm1Regs.TBPHS.bit.TBPHS = 0; // set the phase, is zero, TBPHS=theta*TBPRD/360º
    EPwm1Regs.TBCTL.bit.SYNCOSEL = TB_CTR_ZERO; // enable sinc pulse, this PWM is our ref, when CTR reachs zero
    EPwm1Regs.TBCTR = 0x0000; // reset the counter
    EPwm1Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN; //set up and down pwm type
    EPwm1Regs.TBCTL.bit.PHSEN = TB_DISABLE; // disable the signal sinc in, phase B and C have to config
    EPwm1Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1; //clock ratio to sysclkout
    EPwm1Regs.TBCTL.bit.CLKDIV = TB_DIV1; // divide the clock, usual to slow pwm... like 20 Hz for example

    EPwm1Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW; // set when the width is altered
    EPwm1Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO_PRD; //CC_CTR_ZERO_PRD: zero and prd
    EPwm1Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;
    EPwm1Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO_PRD;

    // sets when the PWM value is altered
    EPwm1Regs.AQCTLA.bit.PRD = AQ_NO_ACTION;
    EPwm1Regs.AQCTLA.bit.ZRO = AQ_NO_ACTION;
    EPwm1Regs.AQCTLA.bit.CAU = AQ_CLEAR;
    EPwm1Regs.AQCTLA.bit.CAD = AQ_SET;

    EPwm1Regs.DBCTL.bit.OUT_MODE = DB_FULL_ENABLE; // enable Dead-band module (turn-on delay + rise time)
    EPwm1Regs.DBCTL.bit.POLSEL = DB_ACTV_HIC; // active hi complementary
    EPwm1Regs.DBFED.bit.DBFED = FALL_TIME; // rise time FED = 20 TBCLKs
    EPwm1Regs.DBRED.bit.DBRED = RISE_TIME; // fall

    // activate the PWM1 trigger to ADC********************************
    EPwm1Regs.ETSEL.bit.SOCAEN = 1;//enable SOC on A group
    EPwm1Regs.ETSEL.bit.SOCASEL = ET_CTR_PRDZERO;//ET_CTR_PRDZERO; // TRIGGER ZERO AND PRD, onde dispara o evento
    EPwm1Regs.ETPS.bit.SOCAPRD = ET_1ST; // trigger on every event, this way, the sample frequency is 2xFpwm
    // ****************************************************************

    //Setup PWM2 ============================================================================================
    EPwm2Regs.TBPRD = SWITCH_PERIOD; // Set the timer period
    EPwm2Regs.CMPA.bit.CMPA = EPwm2Regs.TBPRD >> 1;//set the width pulse
    EPwm2Regs.TBPHS.bit.TBPHS = 0; // set the phase, is zero, TBPHS=theta*TBPRD/360º
    EPwm2Regs.TBCTL.bit.SYNCOSEL = TB_CTR_ZERO; // enable sinc pulse, this PWM is our ref, when CTR reachs zero
    EPwm2Regs.TBCTR = 0x0000; // reset the counter
    EPwm2Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN; //set up and down pwm type
    EPwm2Regs.TBCTL.bit.PHSEN = TB_DISABLE; // disable the signal sinc in, phase B and C have to config
    EPwm2Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1; //clock ratio to sysclkout
    EPwm2Regs.TBCTL.bit.CLKDIV = TB_DIV1; // divide the clock, usual to slow pwm... like 20 Hz for example

    EPwm2Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW; // set when the width is altered
    EPwm2Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO_PRD;
    EPwm2Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;
    EPwm2Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO_PRD;

    // sets when the PWM value is altered
    EPwm2Regs.AQCTLA.bit.PRD = AQ_NO_ACTION;
    EPwm2Regs.AQCTLA.bit.ZRO = AQ_NO_ACTION;
    EPwm2Regs.AQCTLA.bit.CAU = AQ_CLEAR;
    EPwm2Regs.AQCTLA.bit.CAD = AQ_SET;

    EPwm2Regs.DBCTL.bit.OUT_MODE = DB_FULL_ENABLE; // enable Dead-band module (turn-on delay + rise time)
    EPwm2Regs.DBCTL.bit.POLSEL = DB_ACTV_HIC; // active hi complementary
    EPwm2Regs.DBFED.bit.DBFED = FALL_TIME; // rise time FED = 20 TBCLKs
    EPwm2Regs.DBRED.bit.DBRED = RISE_TIME; // fall

    //Setup PWM3 ============================================================================================
    EPwm3Regs.TBPRD = SWITCH_PERIOD; // Set the timer period
    EPwm3Regs.CMPA.bit.CMPA = EPwm3Regs.TBPRD >> 1;//set the width pulse
    EPwm3Regs.TBPHS.bit.TBPHS = 0; // set the phase, is zero, TBPHS=theta*TBPRD/360º
    EPwm3Regs.TBCTL.bit.SYNCOSEL = TB_CTR_ZERO; // enable sinc pulse, this PWM is our ref, when CTR reachs zero
    EPwm3Regs.TBCTR = 0x0000; // reset the counter
    EPwm3Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN; //set up and down pwm type
    EPwm3Regs.TBCTL.bit.PHSEN = TB_DISABLE; // disable the signal sinc in, phase B and C have to config
    EPwm3Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1; //clock ratio to sysclkout
    EPwm3Regs.TBCTL.bit.CLKDIV = TB_DIV1; // divide the clock, usual to slow pwm... like 20 Hz for example

    EPwm3Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW; // set when the width is altered
    EPwm3Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO_PRD;
    EPwm3Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;
    EPwm3Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO_PRD;

    // sets when the PWM value is altered
    EPwm3Regs.AQCTLA.bit.PRD = AQ_NO_ACTION;
    EPwm3Regs.AQCTLA.bit.ZRO = AQ_NO_ACTION;
    EPwm3Regs.AQCTLA.bit.CAU = AQ_CLEAR;
    EPwm3Regs.AQCTLA.bit.CAD = AQ_SET;

    EPwm3Regs.DBCTL.bit.OUT_MODE = DB_FULL_ENABLE; // enable Dead-band module (turn-on delay + rise time)
    EPwm3Regs.DBCTL.bit.POLSEL = DB_ACTV_HIC; // active hi complementary
    EPwm3Regs.DBFED.bit.DBFED = FALL_TIME; // rise time FED = 20 TBCLKs
    EPwm3Regs.DBRED.bit.DBRED = RISE_TIME; // fall


    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1;
    EDIS;


}

void Setup_ADC(void){
    Uint16 acqps;
    // determine minimum acquisition window (in SYSCLKS) based on resolution
    if(ADC_RESOLUTION_12BIT == AdcaRegs.ADCCTL2.bit.RESOLUTION)
        acqps = 42; //75 ns, estava 14, 42 ns
    else
        acqps = 63;//320ns

    EALLOW;

    //GroupA
    CpuSysRegs.PCLKCR13.bit.ADC_A = 1; //enable the clock of group ADC A
    AdcaRegs.ADCCTL2.bit.PRESCALE = 6; //defines the ADCCLK
    AdcSetMode(ADC_ADCA, ADC_RESOLUTION_12BIT, ADC_SIGNALMODE_SINGLE); //configures the adcA
    AdcaRegs.ADCCTL1.bit.INTPULSEPOS = 1;// defines the pulse interruptions as 1 puls before
    AdcaRegs.ADCCTL1.bit.ADCPWDNZ = 1; // turns on the ADC
    DELAY_US(1000); // delay to power up the ADC

    /*
    2 is PIN29 J3(dont use!is shorted to VrefHIB,3V)
    3 is pin26 J3
    4 is PIN69 J7
    5 is the ADC IN A5 pin66 j7
    */

    AdcaRegs.ADCSOC0CTL.bit.CHSEL = 3;
    AdcaRegs.ADCSOC0CTL.bit.ACQPS = acqps; // sample window
    AdcaRegs.ADCSOC0CTL.bit.TRIGSEL = TRIG_SEL_ePWM1_SOCA;    // event that trigger the adc, vide table 11-33

    AdcaRegs.ADCSOC1CTL.bit.CHSEL = 4;
    AdcaRegs.ADCSOC1CTL.bit.ACQPS = acqps; // sample window
    AdcaRegs.ADCSOC1CTL.bit.TRIGSEL = TRIG_SEL_ePWM1_SOCA;    // event that trigger the adc, vide table 11-33

    AdcaRegs.ADCSOC2CTL.bit.CHSEL = 5;
    AdcaRegs.ADCSOC2CTL.bit.ACQPS = acqps; // sample window
    AdcaRegs.ADCSOC2CTL.bit.TRIGSEL = TRIG_SEL_ePWM1_SOCA;    // event that trigger the adc, vide table 11-33

    AdcaRegs.ADCINTSEL1N2.bit.INT1SEL = 0x02; //end of soc1 will se INT1 flag, depends how much channels in use
    AdcaRegs.ADCINTSEL1N2.bit.INT1E = 1; // enable INT1 flag
    AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1; //clear INT1 flag

    /*
    //Group B
    CpuSysRegs.PCLKCR13.bit.ADC_B = 1; //enable the clock of group ADC A
    AdcbRegs.ADCCTL2.bit.PRESCALE = 6; //defines the ADCCLK
    AdcSetMode(ADC_ADCB, ADC_RESOLUTION_12BIT, ADC_SIGNALMODE_SINGLE); //configures the adcA
    AdcbRegs.ADCCTL1.bit.INTPULSEPOS = 1;// defines the pulse interruptions as 1 puls before
    AdcbRegs.ADCCTL1.bit.ADCPWDNZ = 1; // turns on the ADC
    DELAY_US(1000); // delay to power up the ADC */

/*
    //Group C
    CpuSysRegs.PCLKCR13.bit.ADC_C = 1; //enable the clock of group ADC A
    AdccRegs.ADCCTL2.bit.PRESCALE = 1; //defines the ADCCLK
    AdcSetMode(ADC_ADCC, ADC_RESOLUTION_12BIT, ADC_SIGNALMODE_SINGLE); //configures the adcA
    AdccRegs.ADCCTL1.bit.INTPULSEPOS = 1;// defines the pulse interruptions as 1 puls before
    AdccRegs.ADCCTL1.bit.ADCPWDNZ = 1; // turns on the ADC
    DELAY_US(1000); // delay to power up the ADC
*/

    //define the channel - frequency ref group A
   // AdcaRegs.ADCSOC0CTL.bit.CHSEL = 3; // ADC IN A3 pin 26 j3
    //AdcaRegs.ADCSOC0CTL.bit.ACQPS = acqps; // sample window
    //AdcaRegs.ADCSOC0CTL.bit.TRIGSEL = TRIG_SEL_ePWM1_SOCA;
    //define the channel 4 - current measure 1
  //  AdcaRegs.ADCSOC1CTL.bit.CHSEL = 4; // ADC IN A4 pin69 j7
  //  AdcaRegs.ADCSOC1CTL.bit.ACQPS = acqps; // sample window
  //  AdcaRegs.ADCSOC1CTL.bit.TRIGSEL = TRIG_SEL_ePWM1_SOCA;    // event that trigger the adc, vide table 11-33
    //define the channel 5 - current measure 2
    /*
    //define the channel - frequency ref group B
    AdcbRegs.ADCSOC0CTL.bit.CHSEL = 4; // ADC IN A3 pin 26 j3
    AdcbRegs.ADCSOC0CTL.bit.ACQPS = acqps; // sample window
    AdcbRegs.ADCSOC0CTL.bit.TRIGSEL = TRIG_SEL_ePWM1_SOCA;

    //define the channel - frequency ref group C
    AdccRegs.ADCSOC0CTL.bit.CHSEL = 4; // ADC IN C4 pi67
    AdccRegs.ADCSOC0CTL.bit.ACQPS = acqps; // sample window
    AdccRegs.ADCSOC0CTL.bit.TRIGSEL = TRIG_SEL_ePWM1_SOCA;*/




    /*
    AdcbRegs.ADCINTSEL1N2.bit.INT1SEL = 0x01; //end of soc1 will se INT1 flag
    AdcbRegs.ADCINTSEL1N2.bit.INT1E = 1; // enable INT1 flag
    AdcbRegs.ADCINTFLGCLR.bit.ADCINT1 = 1; //clear INT1 flag


    AdccRegs.ADCINTSEL1N2.bit.INT1SEL = 0x01; //end of soc1 will se INT1 flag
    AdccRegs.ADCINTSEL1N2.bit.INT1E = 1; // enable INT1 flag
    AdccRegs.ADCINTFLGCLR.bit.ADCINT1 = 1; //clear INT1 flag
*/

    EDIS;

}

void Setup_DAC(void){
    /*
    EALLOW;
    CpuSysRegs.PCLKCR16.bit.DAC_A = 1; //
    DacaRegs.DACCTL.bit.SYNCSEL = 0x00; //PWM
    DacaRegs.DACCTL.bit.LOADMODE = 0x01; //
    DacaRegs.DACCTL.bit.DACREFSEL = 0x01; // 0 a 3V
    DacaRegs.DACVALS.bit.DACVALS = 0; // 12bits, valuer to convert
    DacaRegs.DACOUTEN.bit.DACOUTEN = 1;
    DacaRegs.DACLOCK.all = 0x00;

    EDIS;
    */

}

void Setup_eQEP(void){
    // pg 1978 spruhm8i.pdf - Technical reference

    EALLOW;
    CpuSysRegs.PCLKCR4.bit.EQEP1 = 1;   //enables the clock
    EDIS;

    EQep1Regs.QDECCTL.bit.QSRC = 2; // Up count mode (freq. measurement)
    EQep1Regs.QDECCTL.bit.XCR = 0;  // 1x resolution: Count the rising edge only
    EQep1Regs.QEPCTL.bit.FREE_SOFT = 2; // Position counter is unaffected by emulation suspend
    //EQep1Regs.QEPCTL.bit.PCRM = 0;    // Position counter reset on an index event
    EQep1Regs.QEPCTL.bit.QCLM = 1;      // Latch on unit time out
    EQep1Regs.QEPCTL.bit.UTE = 1;       // Unit Timer Enable
    EQep1Regs.QPOSMAX = eQEP_max_count;     // max count value
    EQep1Regs.QEPCTL.bit.QPEN = 1;      // QEP enable
   // EQep1Regs.QCAPCTL.bit.UPPS = 0x3;     // 1/8 for unit position
   // EQep1Regs.QCAPCTL.bit.CCPS = 7;       // 1/128 for CAP clock
    EQep1Regs.QCAPCTL.bit.CEN = 1;          // QEP Capture Enable
}
