/*****************************************************************************/
/**
 * @file MicroScope.h
 * @brief MicroScope header file.
 *
 * @author Tomas Correa (TPC)
 *
 * @project Smart Battery
 *
 * @copyright Universidade Federal de Minas Gerais. All rights reserved
 *****************************************************************************/

#ifndef SRC_MICROSCOPE_MICROSCOPE_H_
#define SRC_MICROSCOPE_MICROSCOPE_H_

#include <stdint.h>

#define MS_NUMBER_OF_CHANNELS 2
#define MS_BUFFER_SIZE (100)

typedef enum
{
    TM_AUTO = 0,
    TM_LEVEL_POS,
    TM_LEVEL_NEG,
    TM_ERROR
} TriggerMode;

typedef enum
{
    MS_IDLE = 0,
    MS_WAITING, // SM_WAITING_FOR_TRIGGER
    MS_RECORDING,
    MS_POSTPROCESSING
} ScopeState;

typedef enum
{
    MS_CONTINUOUS = 0,
    MS_SINGLE
} ScopeAcq;


typedef struct {
    float* pChannel[MS_NUMBER_OF_CHANNELS];
    float* pBuffer[MS_NUMBER_OF_CHANNELS];
    float* pOutputBuffer[MS_NUMBER_OF_CHANNELS];
    float* pTrigger;
    uint16_t* pError;
    float pTrigger_1;
    float triggerLevel;
    TriggerMode tm;
    ScopeState state;
    ScopeAcq acq;
    uint16_t idx;
    uint16_t head;
    uint16_t tail;
    uint16_t preTrigger;
    uint16_t initialized;

} MicroScope;

void MS_init(MicroScope *);
void MS_main(MicroScope *);
void MS_saveData(MicroScope *);
void MS_processTrigger(MicroScope*);
void MS_connectChannel(MicroScope *, const uint16_t, float*);
void MS_connectBuffer(MicroScope *, const uint16_t, float*);
void MS_connectOutputBuffer(MicroScope *, const uint16_t, float*);

#endif /* SRC_MICROSCOPE_MICROSCOPE_H_ */
