#include <SVPWM.h>
#include "Peripheral_Setup.h"
#include "math.h"

//interrupt function of ADC
__interrupt void isr_adc(void);         //ADC interruption function

// SWITCH CONFIGURATIONS
//SWITCH_PERIOD = freq. De clock do processador / 2x freq. do PWM (x2 se for up and down)
//float SWITCH_PERIOD=5000;//switch period
float f_switch=2;//switch frequency in kHz

float adc1 = 0;
float adc2 = 0;
float w_nom = 0;
float Imax = 500000;
float soma = 0;
float *p_adc = &adc1;
float frequencia = 0;
uint32_t index = 0;
float wma = 0;
float wmb = 0;
float wmc = 0;

// *****************************************************************************/

unsigned char send = 0, turn_off_command = 0, turn_on_command = 1;
volatile float V_alpha = 0;
volatile float V_beta = 0;
float teta = 0;
float Ts = 100; //sample time
float constant_vf=0;



// personal functions

void initialization();      //initialize the MCU basic setup

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
    EINT; // habilita a interrupção global
    ERTM; // Habilita alteração em tempo real de variaveis
    return ON;
}
eSystemState goto_OFF(void) //shutdown PWM, for example..
{
    DINT;           // Disable CPU __interrupts
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
        return Shutdown_command;
    }
    if(turn_on_command == 1){
   //     PieCtrlRegs.PIEIER1.bit.INTx7 = 1; // Habilita a interrupção do timer, vide tabela 3-4 do manual
        return turnOn_command;
    }
    return no_events;
}


int main(void){

    eSystemState eNextState = INIT;
    eSystemEvent eNewEvent;
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
            w_nom = 2*M_PI*60;

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
            }
     //       MS_saveData(&scope);
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

    //load the adc result to the variables adc1 and adc2
    adc1 = 2*w_nom*AdcaResultRegs.ADCRESULT0/4096;      //ADC INA3 - leitura da frequencia desejada
    adc2 = 0;//2*Imax*AdcaResultRegs.ADCRESULT1/3400 - Imax ;   //ADC INA4

    index = (index == 62800) ? 0 : (index+1);


    teta = w_nom*index*(1/10000.0); //angle in function of time
    V_alpha = cosf(teta);//2500;//cosf(2*M_PI*60*Ts*index*(1/1000000.0)-M_PI/2);
    V_beta = cosf(teta+M_PI/2);


    svpwm_bi(&teta,&V_alpha,&V_beta,&wma,&wmb,&wmc);

    EPwm1Regs.CMPA.bit.CMPA = wma*SWITCH_PERIOD;// fase main
    EPwm2Regs.CMPA.bit.CMPA = wmb*SWITCH_PERIOD; //fase comum
    EPwm3Regs.CMPA.bit.CMPA = wmc*SWITCH_PERIOD; //fase aux


    AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1;
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
         PieVectTable.ADCA1_INT = &isr_adc; // Memory Address of interruption function ADC
         EDIS;
         PieCtrlRegs.PIEIER1.bit.INTx1 = 1; // Enable ADC interruption
         IER |= M_INT1;

         //SWITCH_PERIOD=200000/(4*f_switch));//calculates PRD to configures switch period. If isnt up and down, chage to 200000/(2*f_switch)

}
