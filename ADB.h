/*
 * ADCS.h
 *
 *  Created on: 23 Jul 2019
 *      Author: stefanosperett
 */

#ifndef ADB_H_
#define ADB_H_

#include <ADBTelemetryContainer.h>
#include <driverlib.h>
#include "SLOT_SELECT.h"
#include "SoftwareUpdateService.h"
#include "Bootloader.h"
#include "msp.h"
#include "DelfiPQcore.h"
#include "PQ9Bus.h"
#include "PQ9Frame.h"
#include "PQ9Message.h"
#include "DWire.h"
#include "INA226.h"
#include "Console.h"
#include "CommandHandler.h"
#include "PingService.h"
#include "ResetService.h"
#include "Task.h"
#include "PeriodicTask.h"
#include "TaskManager.h"
#include "HousekeepingService.h"
#include "TMP100.h"
#include "DSPI.h"
#include "MB85RS.h"
#include "PeriodicTaskNotifier.h"
#include "HWMonitor.h"
#include "ADCManager.h"


#define FCLOCK 48000000

#define ADB_ADDRESS     3

// callback functions
void acquireTelemetry(ADBTelemetryContainer *tc);
void periodicTask();

#endif /* ADB_H_ */
