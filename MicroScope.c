/*****************************************************************************/
/**
 * @file MicroScope.c
 * @brief Source file of internal scope.
 *
 * Implementation of a simplified scope to capture x channels according to a
 * pre-defined trigger condition and pre-trigger samples. Data is recorded
 * in the pBuffers and re-organized to be displayed in the sequencially in the
 * pOutputBuffer.
 *
 * @author Tomas Correa (TPC)
 *
 * @project Smart Battery
 *
 * @copyright Universidade Federal de Minas Gerais. All rights reserved
 *****************************************************************************/

#include "MicroScope.h"
#include <stddef.h>
#include "debug.h"

// Static function declarations
static void MS_outputData(MicroScope * self);
static void MS_updateTail(MicroScope * self);

/*****************************************************************************/
/**
 * This function initializes the scope instance.
 *
 * @param   self is the scope instance we are working on.
 *
 * @return  None.
 *
 * @note    The buffers pointers and channel configurations must be done before
 * recording data.
 *
 *****************************************************************************/
void MS_init(MicroScope * self)
{
    // check parameters
    ASSERT(NULL != self);

    self->triggerLevel = 0;
    self->tm = TM_LEVEL_POS;
    self->state = MS_IDLE;
    self->acq = MS_SINGLE;
    self->idx = 0;
    self->tail = 0;
    self->preTrigger = MS_BUFFER_SIZE >>1;

}

/*****************************************************************************/
/**
 * This function does async processing and should be called inside the main loop.
 * After the scope has finished to record data, this function copies the data to
 * the output buffer in the correct order.
 *
 * @param   self is the instance we are working on.
 *
 * @return  None.
 *
 * @note    None.
 *
 *****************************************************************************/
void MS_main(MicroScope * self)
{
    // check parameters
    ASSERT(NULL != self);

    if(MS_POSTPROCESSING == self->state)
    {
        MS_outputData(self);
        if(MS_SINGLE == self->acq)
        {
            self->state = MS_IDLE;
        }
        else
        {
            self->state = MS_WAITING;
        }
    }
}

/*****************************************************************************/
/**
 * This function records the data in the buffer.
 *
 * @param   self is the instance we are working on.
 *
 * @return  None.
 *
 * @note    None.
 *
 *****************************************************************************/
#pragma CODE_SECTION(MS_saveData, ".TI.ramfunc")
void MS_saveData(MicroScope * self)
{
    uint16_t i = 0;

    // check parameters
    ASSERT(NULL != self);
    ASSERT(NULL != self->pBuffer);


    // save data to buffer
    for(i = 0; i < MS_NUMBER_OF_CHANNELS; i++)
    {
        *(self->pBuffer[i]+self->idx) = *(self->pChannel[i]);
    }

    MS_processTrigger(self);
}

/*****************************************************************************/
/**
 * This function verifies if a trigger condition is met to change the scope data
 * to MS_RECORDING.
 *
 * @param   self is the instance we are working on.
 *
 * @return  None.
 *
 * @note    None.
 *
 *****************************************************************************/
void MS_processTrigger(MicroScope * self)
{

    // check parameters
    ASSERT(NULL != self);

    if(MS_RECORDING == self->state)
    {
        if(self->tail == self->idx)
        {
            self->state = MS_POSTPROCESSING;
        }
    }

    if(MS_WAITING == self->state)
    {
        switch(self->tm)
        {
            case TM_AUTO:
                self->state = MS_RECORDING;
                break;
            case TM_LEVEL_POS:
                if(self->triggerLevel < *self->pTrigger && self->triggerLevel > self->pTrigger_1)
                {
                    self->state = MS_RECORDING;
                }
                break;
            case TM_LEVEL_NEG:
                if(self->triggerLevel > *self->pTrigger && self->triggerLevel < self->pTrigger_1)
                {
                    self->state = MS_RECORDING;
                }
                break;
            case TM_ERROR:
                if(*self->pError)
                {
                    self->state = MS_RECORDING;
                }
                break;
        }
        // save last trigger value for level triggering
        self->pTrigger_1 = *self->pTrigger;

        MS_updateTail(self);
    }

    if(MS_POSTPROCESSING != self->state)
    {
        self->idx++;
        if(MS_BUFFER_SIZE == self->idx)
        {
            self->idx = 0;
        }
    }

}


/*****************************************************************************/
/**
 * This function connects the channel pointer to an external variable.
 *
 * @param   self is the instance we are working on.
 *
 * @param   channel is the channel number, between 1 and MS_NUMBER_OF_CHANNELS.
 *
 * @param   p is the pointer to the (float) variable to be captured.
 *
 * @return  None.
 *
 * @note    None.
 *
 *****************************************************************************/
void MS_connectChannel(MicroScope * self, const uint16_t channel, float* p)
{
    // check parameters
    ASSERT(NULL != self);
    ASSERT(NULL != p);

    if(MS_NUMBER_OF_CHANNELS < channel) return;


    self->pChannel[channel - 1U] = p;
}

/*****************************************************************************/
/**
 * This function connects the buffer pointer to a pre-allocated buffer.
 *
 * @param   self is the instance we are working on.
 *
 * @param   channel is the channel number, between 1 and MS_NUMBER_OF_CHANNELS.
 *
 * @param   p is the pointer to the buffer.
 *
 * @return  None.
 *
 * @note    None.
 *
 *****************************************************************************/
void MS_connectBuffer(MicroScope * self, const uint16_t channel, float* p)
{
    // check parameters
    ASSERT(NULL != self);
    ASSERT(NULL != p);

    if(MS_NUMBER_OF_CHANNELS < channel) return;

    self->pBuffer[channel - 1U] = p;
}

/*****************************************************************************/
/**
 * This function connects the output buffer pointer to a pre-allocated buffer.
 *
 * @param   self is the instance we are working on.
 *
 * @param   channel is the channel number, between 1 and MS_NUMBER_OF_CHANNELS.
 *
 * @param   p is the pointer to the output buffer.
 *
 * @return  None.
 *
 * @note    Output buffer is the data that is shown to the user.
 *
 *****************************************************************************/
void MS_connectOutputBuffer(MicroScope * self, const uint16_t channel, float* p)
{
    // check parameters
    ASSERT(NULL != self);
    ASSERT(NULL != p);

    if(MS_NUMBER_OF_CHANNELS < channel) return;

    self->pOutputBuffer[channel - 1U] = p;
}

/*****************************************************************************/
/**
 * This function copies the data to the output buffer in the correct order.
 *
 * @param   self is the instance we are working on.
 *
 * @return  None.
 *
 * @note    None.
 *
 *****************************************************************************/
static void MS_outputData(MicroScope * self)
{
    // check parameters
    ASSERT(NULL != self);

    uint16_t i = 0;
    uint16_t idx = 0, idxOut = 0;
    float* pIn, *pOut;
    // save data to buffer
    for(i = 0; i < MS_NUMBER_OF_CHANNELS; i++)
    {
        // Move from tail to end of the buffer
        for(idx = self->tail + 1; idx < MS_BUFFER_SIZE; idx++)
        {
            pIn = self->pBuffer[i]+idx;
            pOut = self->pOutputBuffer[i]+idxOut;
            *(pOut) = *(pIn);
            idxOut++;
        }
        // and continue from pos 0 till tail - 1
        for(idx = 0; idx <= self->tail; idx++)
        {
            pIn = self->pBuffer[i]+idx;
            pOut = self->pOutputBuffer[i]+idxOut;
            *(pOut) = *(pIn);
            idxOut++;
        }
        idxOut = 0;
    }
}

static void MS_updateTail(MicroScope * self)
{
    // check parameters
    ASSERT(NULL != self);

    int16_t delta = self->idx - self->preTrigger;

    if(0 > delta)
    {
        self->tail = (uint16_t)(MS_BUFFER_SIZE + delta);
    }
    else
    {
        self->tail = (uint16_t)(delta);
    }

}
