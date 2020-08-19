/*
 * BurnService.h
 *
 *  Created on: 23 Jul 2020
 *      Author: Casper
 *
 * Description: Service/Task for burning the wire and deploying the satellite
 *              Object is both a service to respond on the Bus, and a task in order to track time / check the switches.
 *              Task should be periodically executed every second
 *
 */
#ifndef BURNSERVICE_H_
#define BURNSERVICE_H_

#include "Service.h"
#include "PeriodicTask.h"
#include "Console.h"
#include "MB85RS.h"
#include "FRAMBackedVar.h"
#include "FRAMMap.h"

#define BURNSERVICE                  99
#define BURNSERVICE_NO_ERROR         0
#define BURNSERVICE_UNKNOWN_CMD      55
#define BURNSERVICE_ALREADY_BURNING  56
#define BURNSERVICE_ALREADY_DEPLOYED 57
#define BURNSERVICE_INVALID_ANTENNA  58

#define MAX_BURN_TIME   30

#define FRAM_ADB_BURNTIME_1   FRAM_DEVICE_SPECIFIC_SPACE
#define FRAM_ADB_BURNTIME_2   FRAM_ADB_BURNTIME_1 + 1
#define FRAM_ADB_BURNTIME_3   FRAM_ADB_BURNTIME_2 + 1
#define FRAM_ADB_BURNTIME_4   FRAM_ADB_BURNTIME_3 + 1

#define ANTENNA_PORT GPIO_PORT_P3
#define ANTENNA1_FEEDBACK_PIN GPIO_PIN0
#define ANTENNA2_FEEDBACK_PIN GPIO_PIN2
#define ANTENNA3_FEEDBACK_PIN GPIO_PIN4
#define ANTENNA4_FEEDBACK_PIN GPIO_PIN6
#define ANTENNA1_BURN_PIN GPIO_PIN5
#define ANTENNA2_BURN_PIN GPIO_PIN1
#define ANTENNA3_BURN_PIN GPIO_PIN7
#define ANTENNA4_BURN_PIN GPIO_PIN3


class BurnService: public PeriodicTask, public Service
{
private:
    MB85RS* fram = 0;
    uint8_t burnFlag; //0:not burning, 1:burning_1, 2:burning_2 etc.

    FRAMBackedVar<uint8_t> burnTime1;
    FRAMBackedVar<uint8_t> burnTime2;
    FRAMBackedVar<uint8_t> burnTime3;
    FRAMBackedVar<uint8_t> burnTime4;

public:
    BurnService(MB85RS& fram_in);

    void init();

    bool notified( void );

    void taskFunction();

    bool process( DataMessage &command, DataMessage &workingBbuffer );

    bool isDeployed(uint8_t antenna);
};



#endif /* BURNSERVICE_H_ */
