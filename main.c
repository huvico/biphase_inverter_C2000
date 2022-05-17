
#include "Peripheral_Setup.h"
#include "math.h"
//#include <DCL_refgen.h>
#include "MicroScope.h"

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

float adc1 = 0;
float adc2 = 0;
float adc3 = 0;
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
// velocity calculation
unsigned long delta_pos = 0;
unsigned int new_pos = 0, old_pos = 0;
float rpm = 0;
// *****************************************************************************/

unsigned char send = 0, turn_off_command = 0, turn_on_command = 1, set_new_ref = 0, calibration=0;
volatile float V_alpha = 0.0;
volatile float V_beta = 0.0;
float teta = 0;


// *****************************************************************************/
// plot variables
#define BUFFER_plot_size 400
float *p_adc = &V_alpha;
float plot[BUFFER_plot_size];
uint32_t index = 0;
float Ts=1.0/60.0/(float)BUFFER_plot_size;
//#define Ts 20.8333333333f //sample time in us,timmer interruption

// *****************************************************************************/

// Offset variables for current measurements
float offset1 = 0;
float offset2 = 0;
float offset3 = 0;

// Microscope  *****************************************************************************/
MicroScope scope;
float buffer[MS_NUMBER_OF_CHANNELS][MS_BUFFER_SIZE];
float bufferDisplay[MS_NUMBER_OF_CHANNELS][MS_BUFFER_SIZE];

// *****************************************************************************/
// personal functions


void initialization();      //initialize the MCU basic setup
void Calc_RPM();

// State machine *****************************************************************************/
//Different state of machine
typedef enum
{
    INIT,
    ON,
    OFF,
    ERROR,
    ADC_CALIBRATION,
} eSystemState;

//Different type events
typedef enum
{
    HI_Current,
    HI_DC_BUS_Voltage,
    Shutdown_command,
    turnOn_command,
    no_events,

} eSystemEvent;

//Prototype of eventhandlers

eSystemState ini(void)
{
    return INIT;
}

eSystemState end_init_goto_ON(void)
{
    //enables interruptions

    EINT; // habilita a interrupção global
    ERTM; // Habilita alteração em tempo real de variaveis
    return ON;
}
eSystemState goto_OFF(void) //shutdown PWM, for example..
{
    DINT;           // Disable CPU __interrupts
    EPwm1Regs.CMPA.bit.CMPA = 0;// fase main
    EPwm2Regs.CMPA.bit.CMPA = 0; //fase comum
    EPwm3Regs.CMPA.bit.CMPA = 0; //fase aux
    V_alpha = 0;
    V_beta = 0;
    return OFF;
}
eSystemState goto_error(void)
{
    DINT;           // Disable CPU __interrupts
    adc1 = 0;
    adc2 = 0;
    V_alpha = 0;
    V_beta = 0;
    //optional flag error, or send a message to hi supervisory, like Hi Current alarm, trip alarm, etc
    return ERROR;
}

eSystemState ReadEvent(void)
{
    if(adc1 > Imax || adc2 > Imax || adc1 < -Imax || adc2 < -Imax){
        return HI_Current;
    }
    if(turn_off_command == 1){
      //  PieCtrlRegs.PIEIER1.bit.INTx7 = 0; // Disable interruption timmer 0
        DCL_resetRefgen(&rgen);
        DCL_setRefgen(&rgen,0.707,2.0*M_PI*60.0, w_nom, START_TIME_MACHINE, TS_RefGen);
        return Shutdown_command;
    }
    if(turn_on_command == 1){
        EALLOW;
        PieCtrlRegs.PIEIER1.bit.INTx7 = 1;
        PieCtrlRegs.PIEIER1.bit.INTx1 = 1; // Enable ADC interruption
        CpuTimer0Regs.TCR.all = 0x4001; // Habilita a interrupção do timer, vide table 3-22 Register File
        PieCtrlRegs.PIEIER1.bit.INTx7 = 1; // Habilita a interrupção do timer, vide tabela 3-4 do manual
        EDIS;
        return turnOn_command;
    }
    if(set_new_ref == 1){
        DCL_setRefgen(&rgen,new_amp,2.0*M_PI*60.0, 2.0*M_PI*60.0*new_amp, START_TIME_MACHINE, TS_RefGen);
        return turnOn_command;

    }
    return no_events;
}

eSystemEvent eNewEvent;
eSystemState eNextState = INIT;
int main(void){

    //eSystemState eNextState = INIT;
//    eSystemEvent eNewEvent;
    while(1)
    {

        //Read system Events
        eSystemEvent eNewEvent = ReadEvent();
        switch(eNextState)
        {
        case INIT:
        {
            initialization();
            Setup_GPIO();
            Setup_PWM();
            Setup_ADC();
           // Setup_eQEP();
            w_nom = 2.0*M_PI*60.0;

            DCL_resetRefgen(&rgen);
            DCL_setRefgen(&rgen,0.707,2.0*M_PI*60.0, w_nom, START_TIME_MACHINE, TS_RefGen);

            eNextState = end_init_goto_ON();

        }
        break;
        case ON:
        {
            if(HI_Current == eNewEvent)
            {
                eNextState = goto_error();
            }
            if(Shutdown_command == eNewEvent){
                eNextState = goto_OFF();
            }


        }
        break;
        case OFF:
        {
            if(turnOn_command == eNewEvent)
            {
                eNextState = end_init_goto_ON();
                //enables interruptions
                PieCtrlRegs.PIEIER1.bit.INTx7 = 1;
                PieCtrlRegs.PIEIER1.bit.INTx1 = 1; // Enable ADC interruption
                CpuTimer0Regs.TCR.all = 0x4001; // Habilita a interrupção do timer, vide table 3-22 Register File
            }
        }
        break;
        case ERROR:
        {
            if(turnOn_command == eNewEvent)
            {
               // isr_adc();      //read current values
                eNextState = end_init_goto_ON();

            }
        }
        break;
        default:
            break;
        }
    }
    return 0;
}

// Interruptio ADC function
__interrupt void isr_adc(void){
    GpioDataRegs.GPASET.bit.GPIO6=1;
    if(calibration==0){
        DELAY_US(1000000);
        offset1=AdcaResultRegs.ADCRESULT1;
        offset2=AdcaResultRegs.ADCRESULT2;
        calibration=1;
    }


    //load the adc result to the variables adc1 and adc2
    adc1 = 10.0*(float)(AdcaResultRegs.ADCRESULT1-offset1)/4096.0;      //ADC INA4 - leitura da corrente 1, PIN69
    adc2 = 10.0*(float)(AdcaResultRegs.ADCRESULT2-offset2)/4096.0;//2*Imax*AdcaResultRegs.ADCRESULT1/3400 - Imax ;   //ADC INA5, pin66
    adc3 = 355.329949*((float)AdcaResultRegs.ADCRESULT0)/4096.0;//118.64 INA3, PIN26 J3 VDC

    run_Refgen(&rgen,&V_alpha, &V_beta);
    modulo=sqrt(V_alpha*(V_alpha)+V_beta*(V_beta));
    teta=rgen.theta;

    svpwm_bi(&teta, &V_alpha,&V_beta,&wma,&wmb,&wmc);

    EPwm1Regs.CMPA.bit.CMPA = wma*SWITCH_PERIOD;// fase main_W
    EPwm2Regs.CMPA.bit.CMPA = wmb*SWITCH_PERIOD; //fase comum_V
    EPwm3Regs.CMPA.bit.CMPA = wmc*SWITCH_PERIOD; //fase aux_U

    GpioDataRegs.GPACLEAR.bit.GPIO6=1;


    AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1;
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;

}

__interrupt void isr_cpu_timer0(void){
    //Calc_RPM();
    if (index == ((int)BUFFER_plot_size)-1){
        DELAY_US(1);
    }
    index = (index == (int)BUFFER_plot_size) ? 0 : (index+1);
    plot[index]=*p_adc;

    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}

void initialization(){
         InitSysCtrl();  // Initialize System Control
         DINT;           // Disable CPU __interrupts
         InitPieCtrl();  // Initialize the PIE control registers to default state
         IER = 0x0000;   // Disable CPI interrupts
         IFR = 0x0000;   // Clear all CPU interrupt flags

         EALLOW;

         CpuSysRegs.PCLKCR0.bit.CPUTIMER0 = 1;   // Enable CPUtimer clock

         EDIS;

         InitPieVectTable(); // Initialize the PIE vector table
         IpcRegs.IPCCLR.all = 0xFFFFFFFF;        // Clear IPC Flags

         EALLOW;
         PieVectTable.TIMER0_INT = &isr_cpu_timer0; //endereço da função de interrupção
         PieVectTable.ADCA1_INT = &isr_adc; // Memory Address of interruption function ADC
         EDIS;
         //PieCtrlRegs.PIEIER1.bit.INTx7 = 1;
         //PieCtrlRegs.PIEIER1.bit.INTx1 = 1; // Enable ADC interruption
         IER |= M_INT1;

         InitCpuTimers();
         ConfigCpuTimer(&CpuTimer0, 200, Ts*1000000.0); //Escolhe o timer, frequência em MHz e periodo em us
         CpuTimer0Regs.TCR.all = 0x4001; // Habilita a interrupção do timer, vide table 3-22 Register File


}

void Calc_RPM(){
    if(EQep1Regs.QFLG.bit.UTO){             // Unit Timeout event
        new_pos = EQep1Regs.QPOSLAT;        // Latched POSCNT value
        delta_pos = (new_pos > old_pos) ? (new_pos-old_pos) : ((eQEP_max_count-old_pos) + new_pos);
        old_pos = new_pos;
        EQep1Regs.QCLR.bit.UTO = 1;
    }
    rpm = ((float)(delta_pos)) * INT_TIMMER_RPM * 60.0;
}
