#include "ADB.h"

// I2C bus
DWire I2Cinternal(0);
INA226 powerBus(I2Cinternal, 0x40);
TMP100 temp(I2Cinternal, 0x48);

// SPI bus
// DSPI spi(3);
//MB85RS fram(spi, GPIO_PORT_P1, GPIO_PIN0 );

// Bootloader
//Bootloader bootLoader = Bootloader(fram);

// CDHS bus handler
PQ9Bus pq9bus(3, GPIO_PORT_P9, GPIO_PIN0);


// services running in the system
PingService ping;
//ResetService reset( GPIO_PORT_P4, GPIO_PIN0, &fram );
ResetService reset( GPIO_PORT_P4, GPIO_PIN0);
HousekeepingService<ADBTelemetryContainer> hk;
//SoftwareUpdateService SWupdate(fram);
Service* services[] = { &ping, &reset, &hk};

// ADCS board tasks
CommandHandler<PQ9Frame> cmdHandler(pq9bus, services, 3);
PeriodicTask timerTask(10, periodicTask);
PeriodicTask* periodicTasks[] = {&timerTask};
PeriodicTaskNotifier taskNotifier = PeriodicTaskNotifier(periodicTasks, 1);
Task* tasks[] = { &timerTask, &cmdHandler };

volatile bool cmdReceivedFlag = false;
DataFrame* receivedFrame;

// system uptime
unsigned long uptime = 0;

// TODO: remove when bug in CCS has been solved
void receivedCommand(DataFrame &newFrame)
{
    cmdReceivedFlag = true;
    receivedFrame = &newFrame;
    //cmdHandler.received(newFrame);
}

void validCmd(void)
{
    reset.kickInternalWatchDog();
}

void periodicTask()
{
    // increase the timer, this happens every second
    uptime++;

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

    // Initialize I2C master
    I2Cinternal.setFastMode();
    I2Cinternal.begin();

    // Initialize SPI master
    //spi.initMaster(DSPI::MODE0, DSPI::MSBFirst, 1000000);
    //fram.init();

    // initialize the shunt resistor
    powerBus.setShuntResistor(40);

    // initialize temperature sensor
    temp.init();

    // initialize the console
    Console::init(115200);
    pq9bus.begin(115200, 3);     // baud rate: 115200 bps
                                            // address ADB (3)

    //InitBootLoader!
    //bootLoader.JumpSlot();

    // initialize the reset handler:
    // - prepare the watch-dog
    // - initialize the pins for the hardware watch-dog
    // - prepare the pin for power cycling the system
    reset.init();

    // link the command handler to the PQ9 bus:
    // every time a new command is received, it will be forwarded to the command handler
    // TODO: put back the lambda function after bug in CCS has been fixed
    //pq9bus.setReceiveHandler([](PQ9Frame &newFrame){ cmdHandler.received(newFrame); });
    pq9bus.setReceiveHandler(&receivedCommand);

    // every time a command is correctly processed, call the watch-dog
    // TODO: put back the lambda function after bug in CCS has been fixed
    //cmdHandler.onValidCommand([]{ reset.kickInternalWatchDog(); });
    //cmdHandler.onValidCommand(&validCmd);

    Console::log("ADB booting...SLOT: %d", (int) Bootloader::getCurrentSlot());

    if(HAS_SW_VERSION == 1){
        Console::log("SW_VERSION: %s", (const char*)xtr(SW_VERSION));
    }

    TaskManager::start(tasks, 2);
}
