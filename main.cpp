#include "ADB.h"

// I2C bus
DWire I2Cinternal(0);
INA226 powerBus(I2Cinternal, 0x40);
TMP100 temp(I2Cinternal, 0x48);

DSPI spi(3);
MB85RS fram(spi, GPIO_PORT_P1, GPIO_PIN0, MB85RS::MB85RS1MT );

// HardwareMonitor
HWMonitor hwMonitor(&fram);
// Bootloader
Bootloader bootLoader = Bootloader(fram);

// CDHS bus handler
PQ9Bus pq9bus(3, GPIO_PORT_P9, GPIO_PIN0);


// services running in the system
PingService ping;
ResetService reset( GPIO_PORT_P4, GPIO_PIN0, GPIO_PORT_P4, GPIO_PIN2 );
BurnService burnService(powerBus, fram);
HousekeepingService<ADBTelemetryContainer> hk;
FRAMService framService(fram);

#ifndef SW_VERSION
SoftwareUpdateService SWupdate(fram);
#else
SoftwareUpdateService SWupdate(fram, (uint8_t*)xtr(SW_VERSION));
#endif

Service* services[] = { &burnService, &framService, &ping, &reset, &hk, &SWupdate};

// ADCS board tasks
CommandHandler<PQ9Frame, PQ9Message> cmdHandler(pq9bus, services, 6);
PeriodicTask timerTask(1000, periodicTask);
PeriodicTask* periodicTasks[] = {&timerTask, &burnService};
PeriodicTaskNotifier taskNotifier = PeriodicTaskNotifier(periodicTasks, 2);
Task* tasks[] = { &timerTask, &cmdHandler, &burnService };

// system uptime
unsigned long uptime = 0;
FRAMVar<unsigned long> totalUptime;


// TODO: remove when bug in CCS has been solved
void receivedCommand(DataFrame &newFrame)
{
    cmdHandler.received(newFrame);
}

void validCmd(void)
{
    reset.kickInternalWatchDog();
}

void periodicTask()
{
    // increase the timer, this happens every second
    uptime++;
    totalUptime += 1;

    // collect telemetry
    hk.acquireTelemetry(acquireTelemetry);

    // refresh the watch-dog configuration to make sure that, even in case of internal
    // registers corruption, the watch-dog is capable of recovering from an error
    reset.refreshConfiguration();

    // kick hardware watch-dog after every telemetry collection happens
    reset.kickExternalWatchDog();

}

void acquireTelemetry(ADBTelemetryContainer *tc)
{
    unsigned short v;
    signed short i, t;
    unsigned char uc;

    //HouseKeeping Header:
    tc->setStatus(Bootloader::getCurrentSlot());
    fram.read(FRAM_RESET_COUNTER + Bootloader::getCurrentSlot(), &uc, 1);
    tc->setBootCounter(uc);
    tc->setResetCause(hwMonitor.getResetStatus());
    tc->setUptime(uptime);
    tc->setTotalUptime((unsigned long) totalUptime);
    tc->setVersionNumber(2);
    tc->setMCUTemp(hwMonitor.getMCUTemp());

    // acquire board temperature
    tc->setTMPStatus(!temp.getTemperature(t));
    tc->setTemperature(t);

    // measure the power bus
    tc->setINAStatus((!powerBus.getVoltage(v)) & (!powerBus.getCurrent(i)));
    tc->setVoltage(v);
    tc->setCurrent(i);

}

/**
 * main.c
 */
void main(void)
{
    // initialize the MCU:
    // - clock source
    // - clock tree
    DelfiPQcore::initMCU();

    // initialize the ADC
    // - ADC14 and FPU Module
    // - MEM0 for internal temperature measurements
    ADCManager::initADC();

    // Initialize I2C master
    I2Cinternal.setFastMode();
    I2Cinternal.begin();

    // initialize the shunt resistor
    powerBus.setShuntResistor(40);

    // initialize temperature sensor
    temp.init();

    // Initialize SPI master
    spi.initMaster(DSPI::MODE0, DSPI::MSBFirst, 1000000);

    // Initialize fram and fram-variables
    fram.init();
    totalUptime.init(fram, FRAM_TOTAL_UPTIME);

    // initialize the console
    Console::init(115200);
    pq9bus.begin(115200, ADB_ADDRESS);     // baud rate: 115200 bps
                                            // address ADB (3)

    //InitBootLoader!
    bootLoader.JumpSlot();

    // initialize the reset handler:
    // - prepare the watch-dog
    // - initialize the pins for the hardware watch-dog
    // - prepare the pin for power cycling the system
    reset.init();

    // initialize Task Notifier
    taskNotifier.init();

    // initialize HWMonitor readings
    hwMonitor.readResetStatus();
    hwMonitor.readCSStatus();

    // link the command handler to the PQ9 bus:
    // every time a new command is received, it will be forwarded to the command handler
    // TODO: put back the lambda function after bug in CCS has been fixed
    //pq9bus.setReceiveHandler([](PQ9Frame &newFrame){ cmdHandler.received(newFrame); });
    pq9bus.setReceiveHandler(&receivedCommand);

    // every time a command is correctly processed, call the watch-dog
    // TODO: put back the lambda function after bug in CCS has been fixed
    //cmdHandler.onValidCommand([]{ reset.kickInternalWatchDog(); });
    cmdHandler.onValidCommand(&validCmd);

    Console::log("ADB booting...SLOT: %d", (int) Bootloader::getCurrentSlot());

    if(HAS_SW_VERSION == 1){
        Console::log("SW_VERSION: %s", (const char*)xtr(SW_VERSION));
    }

    TaskManager::start(tasks, 3);
}
