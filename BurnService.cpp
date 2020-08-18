/*
 * BurnService.cpp
 *
 *  Created on: 23 Jul 2020
 *      Author: Casper
 */

#include "BurnService.h"
#include "PeriodicTask.h"

BurnService* _bservicestub;

BurnService::BurnService(MB85RS& fram_in) : PeriodicTask(1000, [](){_bservicestub->taskFunction();})
{
    _bservicestub = this;
    fram = &fram_in;
    burnFlag = 0;

    burnTime1.init(*fram, FRAM_ADB_BURNTIME_1);
    burnTime2.init(*fram, FRAM_ADB_BURNTIME_2);
    burnTime3.init(*fram, FRAM_ADB_BURNTIME_3);
    burnTime4.init(*fram, FRAM_ADB_BURNTIME_4);

    //set GPIO settings
    //Antenna Feedback:
    GPIO_setAsInputPin(ANTENNA_PORT,ANTENNA1_FEEDBACK_PIN);
    GPIO_setAsInputPin(ANTENNA_PORT,ANTENNA2_FEEDBACK_PIN);
    GPIO_setAsInputPin(ANTENNA_PORT,ANTENNA3_FEEDBACK_PIN);
    GPIO_setAsInputPin(ANTENNA_PORT,ANTENNA4_FEEDBACK_PIN);
    //Antenna BurnPins;
    GPIO_setOutputLowOnPin(ANTENNA_PORT, ANTENNA1_BURN_PIN);
    GPIO_setOutputLowOnPin(ANTENNA_PORT, ANTENNA2_BURN_PIN);
    GPIO_setOutputLowOnPin(ANTENNA_PORT, ANTENNA3_BURN_PIN);
    GPIO_setOutputLowOnPin(ANTENNA_PORT, ANTENNA4_BURN_PIN);

    GPIO_setAsOutputPin(ANTENNA_PORT, ANTENNA1_BURN_PIN);
    GPIO_setAsOutputPin(ANTENNA_PORT, ANTENNA2_BURN_PIN);
    GPIO_setAsOutputPin(ANTENNA_PORT, ANTENNA3_BURN_PIN);
    GPIO_setAsOutputPin(ANTENNA_PORT, ANTENNA4_BURN_PIN);
}

bool BurnService::notified( void )
{
    return (burnFlag != 0) && execute;
};

void BurnService::taskFunction()
{
    Console::log("BurnTask: Burning.. %d", burnFlag);
    switch(burnFlag)
    {
    case 1:
        if(isDeployed(1))
        {
            Console::log("Burning 1: Succes!");
            burnFlag = 2;
            //set burn LOW
            GPIO_setOutputLowOnPin(ANTENNA_PORT, ANTENNA1_BURN_PIN);
            break;
        }
        else
        {
            //set burn HIGH
            GPIO_setOutputHighOnPin(ANTENNA_PORT, ANTENNA1_BURN_PIN);
            burnTime1 += 1;
            Console::log("Burning 1: %d seconds", (uint8_t)burnTime1);
            if(burnTime1 >= MAX_BURN_TIME){
                Console::log("Burning 1: reached MAX_BURN_TIME");
                //set burn Low
                GPIO_setOutputLowOnPin(ANTENNA_PORT, ANTENNA1_BURN_PIN);
                burnFlag = 2;
            }
            break;
        }
    case 2:
        if(isDeployed(2))
        {
            Console::log("Burning 2: Succes!");
            burnFlag = 3;
            //set burn LOW
            GPIO_setOutputLowOnPin(ANTENNA_PORT, ANTENNA2_BURN_PIN);
            break;
        }
        else
        {
            //set burn HIGH
            GPIO_setOutputHighOnPin(ANTENNA_PORT, ANTENNA2_BURN_PIN);
            burnTime2 += 1;
            Console::log("Burning 2: %d seconds", (uint8_t)burnTime2);
            if(burnTime2 >= MAX_BURN_TIME){
                Console::log("Burning 1: reached MAX_BURN_TIME");
                //set burn Low
                GPIO_setOutputLowOnPin(ANTENNA_PORT, ANTENNA2_BURN_PIN);
                burnFlag = 3;
            }
            break;
        }
    case 3:
        if(isDeployed(3))
        {
            Console::log("Burning 3: Succes!");
            //set burn LOW
            GPIO_setOutputLowOnPin(ANTENNA_PORT, ANTENNA3_BURN_PIN);
            burnFlag = 4;
            break;
        }
        else
        {
            //set burn HIGH
            GPIO_setOutputHighOnPin(ANTENNA_PORT, ANTENNA3_BURN_PIN);
            burnTime3 += 1;
            Console::log("Burning 3: %d seconds", (uint8_t)burnTime3);
            if(burnTime3 >= MAX_BURN_TIME){
                Console::log("Burning 1: reached MAX_BURN_TIME");
                //set burn Low
                GPIO_setOutputLowOnPin(ANTENNA_PORT, ANTENNA3_BURN_PIN);
                burnFlag = 4;
            }
            break;
        }
    case 4:
        if(isDeployed(4))
        {
            Console::log("Burning 4: Succes!");
            burnFlag = 0;
            //set burn LOW
            GPIO_setOutputLowOnPin(ANTENNA_PORT, ANTENNA4_BURN_PIN);
            break;
        }
        else
        {
            //set burn HIGH
            GPIO_setOutputHighOnPin(ANTENNA_PORT, ANTENNA4_BURN_PIN);
            burnTime4 += 1;
            Console::log("Burning 4: %d seconds", (uint8_t)burnTime4);
            if(burnTime4 >= MAX_BURN_TIME){
                Console::log("Burning 4: reached MAX_BURN_TIME");
                //set burn Low
                GPIO_setOutputLowOnPin(ANTENNA_PORT, ANTENNA4_BURN_PIN);
            }
            break;
        }
    default:
        Console::log("Unknown burnFlag");
        burnFlag = 0;
    }
}


bool BurnService::process( DataMessage &command, DataMessage &workingBuffer )
{
    if(command.getService() == BURNSERVICE)
    {
        workingBuffer.setService(BURNSERVICE);
        workingBuffer.setMessageType(SERVICE_RESPONSE_REPLY);

        Console::log("BurnService!");
        switch(command.getDataPayload()[0])
        {
        case 0:
            Console::log("BurnService: isDeployed");
            if(command.getPayloadSize() == 1)
            {
                Console::log("Antenna1 %s", (isDeployed(1)?"YES":"NO") );
                Console::log("Antenna2 %s", (isDeployed(2)?"YES":"NO") );
                Console::log("Antenna3 %s", (isDeployed(3)?"YES":"NO") );
                Console::log("Antenna4 %s", (isDeployed(4)?"YES":"NO") );
                workingBuffer.getDataPayload()[0] = isDeployed(1) ? 1:0;
                workingBuffer.getDataPayload()[1] = isDeployed(2) ? 1:0;
                workingBuffer.getDataPayload()[2] = isDeployed(3) ? 1:0;
                workingBuffer.getDataPayload()[3] = isDeployed(4) ? 1:0;
                workingBuffer.setPayloadSize(4);
            }
            else if(command.getPayloadSize() == 2)
            {
                Console::log("Antenna%d %s", command.getDataPayload()[1], (isDeployed(command.getDataPayload()[1])?"YES":"NO") );
                workingBuffer.getDataPayload()[0] = isDeployed(command.getDataPayload()[1]) ? 1:0;
                workingBuffer.setPayloadSize(1);
            }
            else
            {
                //unknown command
                workingBuffer.getDataPayload()[0] = BURNSERVICE_UNKNOWN_CMD;
                workingBuffer.setPayloadSize(1);
            }
            break;
        case 1:
            Console::log("BurnService: StartBurn");
            if(burnFlag == 0)
            {
                if(command.getPayloadSize() == 1)
                {
                    if(!isDeployed(1))
                    {
                        burnFlag = 1;
                        workingBuffer.getDataPayload()[0] = 0;
                        workingBuffer.setPayloadSize(1);
                    }
                    else if(!isDeployed(2))
                    {
                        burnFlag = 2;
                        workingBuffer.getDataPayload()[0] = 0;
                        workingBuffer.setPayloadSize(1);
                    }
                    else if(!isDeployed(3))
                    {
                        burnFlag = 3;
                        workingBuffer.getDataPayload()[0] = 0;
                        workingBuffer.setPayloadSize(1);
                    }
                    else if(!isDeployed(4))
                    {
                        burnFlag = 4;
                        workingBuffer.getDataPayload()[0] = 0;
                        workingBuffer.setPayloadSize(1);
                    }else{
                        Console::log("BurnService: Already Fully Deployed!");
                        workingBuffer.getDataPayload()[0] = BURNSERVICE_ALREADY_DEPLOYED;
                        workingBuffer.setPayloadSize(1);
                    }
                }
                else if(command.getPayloadSize() == 2)
                {
                    if(!isDeployed(command.getDataPayload()[1]))
                    {
                        burnFlag = command.getDataPayload()[1];
                        workingBuffer.getDataPayload()[0] = 0;
                        workingBuffer.setPayloadSize(1);
                    }
                    else
                    {
                        workingBuffer.getDataPayload()[0] = BURNSERVICE_ALREADY_DEPLOYED;
                        workingBuffer.setPayloadSize(1);
                    }
                }
                else
                {
                    //unknown command
                    workingBuffer.getDataPayload()[0] = BURNSERVICE_UNKNOWN_CMD;
                    workingBuffer.setPayloadSize(1);
                }
            }
            else
            {
                workingBuffer.getDataPayload()[0] = BURNSERVICE_ALREADY_BURNING;
                workingBuffer.setPayloadSize(1);
            }
            break;
        case 2:
            Console::log("BurnService: getBurnTime");
            switch(command.getDataPayload()[1])
            {
            case 1:
                Console::log("Antenna %d burntime: %d", command.getDataPayload()[1], (uint8_t) burnTime1);
                break;
            case 2:
                Console::log("Antenna %d burntime: %d", command.getDataPayload()[1], (uint8_t) burnTime1);
                break;
            case 3:
                Console::log("Antenna %d burntime: %d", command.getDataPayload()[1], (uint8_t) burnTime1);
                break;
            case 4:
                Console::log("Antenna %d burntime: %d", command.getDataPayload()[1], (uint8_t) burnTime1);
                break;
            default:
                Console::log("Invalid Antenna");
                break;
            }
            break;
        case 3:
            Console::log("BurnService: resetBurnTime");
            switch(command.getDataPayload()[1])
            {
            case 1:
                Console::log("Antenna %d Reset Burntime", command.getDataPayload()[1]);
                burnTime1 = 0;
                break;
            case 2:
                Console::log("Antenna %d Reset Burntime", command.getDataPayload()[1]);
                burnTime2 = 0;
                break;
            case 3:
                Console::log("Antenna %d Reset Burntime", command.getDataPayload()[1]);
                burnTime3 = 0;
                break;
            case 4:
                Console::log("Antenna %d Reset Burntime", command.getDataPayload()[1]);
                burnTime4 = 0;
                break;
            default:
                Console::log("Invalid Antenna");
                break;
            }
            break;
        case 4:
            Console::log("Antenna 1 High");
            GPIO_setOutputHighOnPin(ANTENNA_PORT, ANTENNA1_BURN_PIN);
            break;
        case 5:
            Console::log("Antenna 1 Low");
            GPIO_setOutputLowOnPin(ANTENNA_PORT, ANTENNA1_BURN_PIN);
            break;
        default:
            Console::log("BurnService: InvalidCommand");
        }
        return true;
    }
    else
    {
        return false;
    }
};

bool BurnService::isDeployed(uint8_t antennaNumber)
{
    switch(antennaNumber)
    {
    case 1:
        return (GPIO_getInputPinValue(ANTENNA_PORT, ANTENNA1_FEEDBACK_PIN) == GPIO_INPUT_PIN_HIGH);
    case 2:
        return (GPIO_getInputPinValue(ANTENNA_PORT, ANTENNA2_FEEDBACK_PIN) == GPIO_INPUT_PIN_HIGH);
    case 3:
        return (GPIO_getInputPinValue(ANTENNA_PORT, ANTENNA3_FEEDBACK_PIN) == GPIO_INPUT_PIN_HIGH);
    case 4:
        return (GPIO_getInputPinValue(ANTENNA_PORT, ANTENNA4_FEEDBACK_PIN) == GPIO_INPUT_PIN_HIGH);
    default:
        return false;
    }
}


