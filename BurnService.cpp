/*
 * BurnService.cpp
 *
 *  Created on: 23 Jul 2020
 *      Author: Casper
 */

#include "BurnService.h"
#include "PeriodicTask.h"

BurnService* _bservicestub;

BurnService::BurnService(INA226& currentSensor_in, MB85RS& fram_in) : PeriodicTask(1000, [](){_bservicestub->taskFunction();}, [](){_bservicestub->init();})
{
    _bservicestub = this;
    fram = &fram_in;
    currentSensor = &currentSensor_in;

    burnFlag = 0;

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

void BurnService::init()
{
    totalBurnTime1.init(*fram, FRAM_ADB_BURNTIME_1, true, true);
    totalBurnTime2.init(*fram, FRAM_ADB_BURNTIME_2, true, true);
    totalBurnTime3.init(*fram, FRAM_ADB_BURNTIME_3, true, true);
    totalBurnTime4.init(*fram, FRAM_ADB_BURNTIME_4, true, true);
}

bool BurnService::notified( void )
{
    if(execute)
    {
        execute = false;
        return (burnFlag != 0);
    }
    return false;
};

void BurnService::taskFunction()
{
    if(ignoreSwitch){
        Console::log("BurnTask: Burning.. %d !IGNORING SWITCH!", burnFlag);
    }else{
        Console::log("BurnTask: Burning.. %d ", burnFlag);
    }
    switch(burnFlag)
    {
    case 1:
        if(isDeployed(1) && !ignoreSwitch)
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
            totalBurnTime1 += 1;
            currentBurnTime1 += 1;
            Console::log("Burning 1: %d seconds", (uint8_t)currentBurnTime1);
            if(currentBurnTime1 >= MAX_BURN_TIME){
                Console::log("Burning 1: reached MAX_BURN_TIME");
                //set burn Low
                GPIO_setOutputLowOnPin(ANTENNA_PORT, ANTENNA1_BURN_PIN);
                if(!ignoreSwitch){
                    burnFlag = 2;
                }else{
                    burnFlag = 0;
                    ignoreSwitch = false;
                }
            }
            break;
        }
    case 2:
        if(isDeployed(2) && !ignoreSwitch)
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
            totalBurnTime2 += 1;
            currentBurnTime2 += 1;
            Console::log("Burning 2: %d seconds", (uint8_t)currentBurnTime2);
            if(currentBurnTime2 >= MAX_BURN_TIME){
                Console::log("Burning 1: reached MAX_BURN_TIME");
                //set burn Low
                GPIO_setOutputLowOnPin(ANTENNA_PORT, ANTENNA2_BURN_PIN);
                if(!ignoreSwitch){
                    burnFlag = 3;
                }else{
                    burnFlag = 0;
                    ignoreSwitch = false;
                }
            }
            break;
        }
    case 3:
        if(isDeployed(3) && !ignoreSwitch)
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
            totalBurnTime3 += 1;
            currentBurnTime3 += 1;
            Console::log("Burning 3: %d seconds", (uint8_t)currentBurnTime3);
            if(currentBurnTime3 >= MAX_BURN_TIME){
                Console::log("Burning 1: reached MAX_BURN_TIME");
                //set burn Low
                GPIO_setOutputLowOnPin(ANTENNA_PORT, ANTENNA3_BURN_PIN);
                if(!ignoreSwitch){
                    burnFlag = 4;
                }else{
                    burnFlag = 0;
                    ignoreSwitch = false;
                }
            }
            break;
        }
    case 4:
        if(isDeployed(4) && !ignoreSwitch)
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
            totalBurnTime4 += 1;
            currentBurnTime4 += 1;
            Console::log("Burning 4: %d seconds", (uint8_t)currentBurnTime4);
            if(currentBurnTime4 >= MAX_BURN_TIME){
                Console::log("Burning 4: reached MAX_BURN_TIME");
                burnFlag = 0;
                if(ignoreSwitch){
                    ignoreSwitch = false;
                }
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
            workingBuffer.getDataPayload()[0] = command.getDataPayload()[0];
            if(command.getPayloadSize() == 1)
            {
                Console::log("Antenna1 %s", (isDeployed(1)?"YES":"NO") );
                Console::log("Antenna2 %s", (isDeployed(2)?"YES":"NO") );
                Console::log("Antenna3 %s", (isDeployed(3)?"YES":"NO") );
                Console::log("Antenna4 %s", (isDeployed(4)?"YES":"NO") );
                workingBuffer.getDataPayload()[1] |= (isDeployed(1) ? 1:0) << 3;
                workingBuffer.getDataPayload()[1] |= (isDeployed(2) ? 1:0) << 2;
                workingBuffer.getDataPayload()[1] |= (isDeployed(3) ? 1:0) << 1;
                workingBuffer.getDataPayload()[1] |= (isDeployed(4) ? 1:0);
                workingBuffer.setPayloadSize(2);
            }
            else if(command.getPayloadSize() == 2)
            {
                if(command.getDataPayload()[1] >= 1 && command.getDataPayload()[1] <= 4)
                {
                    Console::log("Antenna%d %s", command.getDataPayload()[1], (isDeployed(command.getDataPayload()[1])?"YES":"NO") );
                    workingBuffer.getDataPayload()[1] = isDeployed(command.getDataPayload()[1]) ? 1:0;
                    workingBuffer.setPayloadSize(2);
                }
                else
                {
                    workingBuffer.getDataPayload()[1] = BURNSERVICE_INVALID_ANTENNA;
                    workingBuffer.setPayloadSize(2);
                }
            }
            else
            {
                //unknown command
                workingBuffer.getDataPayload()[0] = BURNSERVICE_UNKNOWN_CMD;
                workingBuffer.setPayloadSize(1);
            }
            break;
        case 1:
            Console::log("BurnService: isBurning?");
            workingBuffer.setPayloadSize(3);
            workingBuffer.getDataPayload()[0] = command.getDataPayload()[0];
            workingBuffer.getDataPayload()[1] = BURNSERVICE_NO_ERROR;
            workingBuffer.getDataPayload()[2] = burnFlag ? 1 : 0;
            break;
        case 2:
            Console::log("BurnService: StartBurn");
            workingBuffer.getDataPayload()[0] = command.getDataPayload()[0];
            if(burnFlag == 0)
            {
                if(command.getPayloadSize() == 1)
                {
                    currentBurnTime1 = 0;
                    currentBurnTime2 = 0;
                    currentBurnTime3 = 0;
                    currentBurnTime4 = 0;

                    if(!isDeployed(1))
                    {
                        burnFlag = 1;
                        workingBuffer.getDataPayload()[1] = BURNSERVICE_NO_ERROR;
                        workingBuffer.setPayloadSize(2);
                    }
                    else if(!isDeployed(2))
                    {
                        burnFlag = 2;
                        workingBuffer.getDataPayload()[1] = BURNSERVICE_NO_ERROR;
                        workingBuffer.setPayloadSize(2);
                    }
                    else if(!isDeployed(3))
                    {
                        burnFlag = 3;
                        workingBuffer.getDataPayload()[1] = BURNSERVICE_NO_ERROR;
                        workingBuffer.setPayloadSize(2);
                    }
                    else if(!isDeployed(4))
                    {
                        burnFlag = 4;
                        workingBuffer.getDataPayload()[1] = BURNSERVICE_NO_ERROR;
                        workingBuffer.setPayloadSize(2);
                    }else{
                        Console::log("BurnService: Already Fully Deployed!");
                        workingBuffer.getDataPayload()[1] = BURNSERVICE_ALREADY_DEPLOYED;
                        workingBuffer.setPayloadSize(2);
                    }
                }
                else if(command.getPayloadSize() == 2)
                {
                    if(command.getDataPayload()[1] >= 1 && command.getDataPayload()[1] <= 4)
                    {

                        if(!isDeployed(command.getDataPayload()[1]))
                        {
                            burnFlag = command.getDataPayload()[1];
                            workingBuffer.getDataPayload()[0] = command.getDataPayload()[0];
                            workingBuffer.getDataPayload()[1] = BURNSERVICE_NO_ERROR;
                            workingBuffer.setPayloadSize(2);
                        }
                        else
                        {
                            workingBuffer.getDataPayload()[0] = command.getDataPayload()[0];
                            workingBuffer.getDataPayload()[1] = BURNSERVICE_ALREADY_DEPLOYED;
                            workingBuffer.setPayloadSize(2);
                        }
                    }
                    else
                    {
                        workingBuffer.getDataPayload()[0] = command.getDataPayload()[0];
                        workingBuffer.getDataPayload()[1] = BURNSERVICE_INVALID_ANTENNA;
                        workingBuffer.setPayloadSize(2);
                    }
                }
                else
                {
                    //unknown command
                    workingBuffer.getDataPayload()[0] = command.getDataPayload()[0];
                    workingBuffer.getDataPayload()[1] = BURNSERVICE_UNKNOWN_CMD;
                    workingBuffer.setPayloadSize(2);
                }
            }
            else
            {
                workingBuffer.getDataPayload()[0] = command.getDataPayload()[0];
                workingBuffer.getDataPayload()[1] = BURNSERVICE_ALREADY_BURNING;
                workingBuffer.setPayloadSize(2);
            }
            break;
        case 3:
            Console::log("BurnService: getBurnTime");
            if(command.getPayloadSize() == 2)
            {
                short burnTime = 0;
                switch(command.getDataPayload()[1])
                {
                case 1:
                    Console::log("Antenna %d burntime: %d", command.getDataPayload()[1], (short) totalBurnTime1);
                    workingBuffer.getDataPayload()[0] = command.getDataPayload()[0];
                    burnTime= totalBurnTime1;
                    ((unsigned char *)&burnTime)[1] = command.getDataPayload()[1];
                    ((unsigned char *)&burnTime)[0] = command.getDataPayload()[2];
                    workingBuffer.setPayloadSize(3);
                    break;
                case 2:
                    Console::log("Antenna %d burntime: %d", command.getDataPayload()[1], (short) totalBurnTime2);
                    workingBuffer.getDataPayload()[0] = command.getDataPayload()[0];
                    burnTime = totalBurnTime2;
                    ((unsigned char *)&burnTime)[1] = command.getDataPayload()[1];
                    ((unsigned char *)&burnTime)[0] = command.getDataPayload()[2];
                    workingBuffer.setPayloadSize(3);
                    break;
                case 3:
                    Console::log("Antenna %d burntime: %d", command.getDataPayload()[1], (short) totalBurnTime3);
                    workingBuffer.getDataPayload()[0] = command.getDataPayload()[0];
                    burnTime = totalBurnTime3;
                    ((unsigned char *)&burnTime)[1] = command.getDataPayload()[1];
                    ((unsigned char *)&burnTime)[0] = command.getDataPayload()[2];
                    workingBuffer.setPayloadSize(3);
                    break;
                case 4:
                    Console::log("Antenna %d burntime: %d", command.getDataPayload()[1], (short) totalBurnTime4);
                    workingBuffer.getDataPayload()[0] = command.getDataPayload()[0];
                    burnTime = totalBurnTime4;
                    ((unsigned char *)&burnTime)[1] = command.getDataPayload()[1];
                    ((unsigned char *)&burnTime)[0] = command.getDataPayload()[2];
                    workingBuffer.setPayloadSize(3);
                    break;
                default:
                    Console::log("Invalid Antenna");
                    workingBuffer.getDataPayload()[0] = command.getDataPayload()[0];
                    workingBuffer.getDataPayload()[1] = BURNSERVICE_INVALID_ANTENNA;
                    workingBuffer.setPayloadSize(2);
                    break;
                }
            }
            else
            {
                workingBuffer.getDataPayload()[0] = command.getDataPayload()[0];
                workingBuffer.getDataPayload()[1] = BURNSERVICE_UNKNOWN_CMD;
                workingBuffer.setPayloadSize(2);
            }
            break;
        case 4:
            Console::log("BurnService: resetBurnTime");
            if(command.getPayloadSize() == 2)
            {
                switch(command.getDataPayload()[1])
                {
                case 1:
                    Console::log("Antenna %d Reset Burntime", command.getDataPayload()[1]);
                    workingBuffer.getDataPayload()[0] = 3;
                    workingBuffer.getDataPayload()[1] = BURNSERVICE_NO_ERROR;
                    workingBuffer.setPayloadSize(2);
                    totalBurnTime1 = 0;
                    break;
                case 2:
                    Console::log("Antenna %d Reset Burntime", command.getDataPayload()[1]);
                    workingBuffer.getDataPayload()[0] = 3;
                    workingBuffer.getDataPayload()[1] = BURNSERVICE_NO_ERROR;
                    workingBuffer.setPayloadSize(2);
                    totalBurnTime2 = 0;
                    break;
                case 3:
                    Console::log("Antenna %d Reset Burntime", command.getDataPayload()[1]);
                    workingBuffer.getDataPayload()[0] = 3;
                    workingBuffer.getDataPayload()[1] = BURNSERVICE_NO_ERROR;
                    workingBuffer.setPayloadSize(2);
                    totalBurnTime3 = 0;
                    break;
                case 4:
                    Console::log("Antenna %d Reset Burntime", command.getDataPayload()[1]);
                    workingBuffer.getDataPayload()[0] = 3;
                    workingBuffer.getDataPayload()[1] = BURNSERVICE_NO_ERROR;
                    workingBuffer.setPayloadSize(2);
                    totalBurnTime4 = 0;
                    break;
                default:
                    Console::log("Invalid Antenna");
                    workingBuffer.getDataPayload()[0] = 3;
                    workingBuffer.getDataPayload()[1] = BURNSERVICE_INVALID_ANTENNA;
                    workingBuffer.setPayloadSize(2);
                    break;
                }
            }
            else
            {
                workingBuffer.getDataPayload()[0] = 3;
                workingBuffer.getDataPayload()[1] = BURNSERVICE_UNKNOWN_CMD;
                workingBuffer.setPayloadSize(2);
            }
            break;
        case 5:
            Console::log("BurnService: StopBurn");
            GPIO_setOutputLowOnPin(ANTENNA_PORT, ANTENNA1_BURN_PIN);
            GPIO_setOutputLowOnPin(ANTENNA_PORT, ANTENNA2_BURN_PIN);
            GPIO_setOutputLowOnPin(ANTENNA_PORT, ANTENNA3_BURN_PIN);
            GPIO_setOutputLowOnPin(ANTENNA_PORT, ANTENNA4_BURN_PIN);
            burnFlag = 0;
            workingBuffer.getDataPayload()[0] = 4;
            workingBuffer.getDataPayload()[1] = BURNSERVICE_NO_ERROR;
            workingBuffer.setPayloadSize(2);
            break;
        case 6:
            Console::log("BurnService: measure Current");
            if(command.getPayloadSize() == 2)
            {
                switch(command.getDataPayload()[1])
                {
                case 1:
                case 2:
                case 3:
                case 4:
                    short measurement = measureCurrent(command.getDataPayload()[1]);
                    workingBuffer.getDataPayload()[1] = ((unsigned char *)&measurement)[1];
                    workingBuffer.getDataPayload()[2] = ((unsigned char *)&measurement)[0];
                    workingBuffer.setPayloadSize(3);
                    Console::log("Current Measurement: %d mA", measurement);
                    break;
                default:
                    Console::log("Invalid Antenna");
                    workingBuffer.getDataPayload()[0] = 5;
                    workingBuffer.getDataPayload()[1] = BURNSERVICE_INVALID_ANTENNA;
                    workingBuffer.setPayloadSize(2);
                    break;
                }
            }
            else
            {
                workingBuffer.getDataPayload()[0] = 5;
                workingBuffer.getDataPayload()[1] = BURNSERVICE_UNKNOWN_CMD;
                workingBuffer.setPayloadSize(2);
            }
            break;
        case 7:
            Console::log("BurnService: BURN 30 SEC OVERRIDE");
            if(command.getPayloadSize() == 2)
            {
                if(command.getDataPayload()[1] >= 1 && command.getDataPayload()[1] <= 4)
                {
                    burnFlag = command.getDataPayload()[1];
                    ignoreSwitch = true;
                    currentBurnTime1 = 0;
                    currentBurnTime2 = 0;
                    currentBurnTime3 = 0;
                    currentBurnTime4 = 0;
                    workingBuffer.getDataPayload()[0] = command.getDataPayload()[0];
                    workingBuffer.getDataPayload()[1] = BURNSERVICE_NO_ERROR;
                    workingBuffer.setPayloadSize(2);
                }
                else
                {
                    workingBuffer.getDataPayload()[0] = command.getDataPayload()[0];
                    workingBuffer.getDataPayload()[1] = BURNSERVICE_INVALID_ANTENNA;
                    workingBuffer.setPayloadSize(2);
                }
            }
            else
            {
                workingBuffer.getDataPayload()[0] = 5;
                workingBuffer.getDataPayload()[1] = BURNSERVICE_UNKNOWN_CMD;
                workingBuffer.setPayloadSize(2);
            }
            break;
        default:
            Console::log("BurnService: InvalidCommand");
            workingBuffer.getDataPayload()[0] = command.getDataPayload()[0];
            workingBuffer.getDataPayload()[1] = BURNSERVICE_UNKNOWN_CMD;
            workingBuffer.setPayloadSize(2);
            break;
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

short BurnService::measureCurrent(uint8_t antennaNumber){
    if(!burnFlag){
        switch(antennaNumber)
        {
        case 1:
            MAP_GPIO_setOutputHighOnPin(ANTENNA_PORT, ANTENNA1_BURN_PIN);
            break;
        case 2:
            MAP_GPIO_setOutputHighOnPin(ANTENNA_PORT, ANTENNA2_BURN_PIN);
            break;
        case 3:
            MAP_GPIO_setOutputHighOnPin(ANTENNA_PORT, ANTENNA3_BURN_PIN);
            break;
        case 4:
            MAP_GPIO_setOutputHighOnPin(ANTENNA_PORT, ANTENNA4_BURN_PIN);
            break;
        }
    }

    //delay like 20ms
    __delay_cycles(48000000/(1000L/20)); // @suppress("Function cannot be resolved")
    signed short current = 0;
    currentSensor->getCurrent(current);

    if(!burnFlag){
        switch(antennaNumber)
        {
        case 1:
            MAP_GPIO_setOutputLowOnPin(ANTENNA_PORT, ANTENNA1_BURN_PIN);
            break;
        case 2:
            MAP_GPIO_setOutputLowOnPin(ANTENNA_PORT, ANTENNA2_BURN_PIN);
            break;
        case 3:
            MAP_GPIO_setOutputLowOnPin(ANTENNA_PORT, ANTENNA3_BURN_PIN);
            break;
        case 4:
            MAP_GPIO_setOutputLowOnPin(ANTENNA_PORT, ANTENNA4_BURN_PIN);
            break;
        }
    }

    return current;
}


