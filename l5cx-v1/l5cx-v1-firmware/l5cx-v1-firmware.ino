#include <SparkFun_VL53L5CX_Library.h>
#define L5CX SparkFun_VL53L5CX

// modify this if the USB Serial (COM Port) is different on your board
#define USB_SERIAL Serial
#define INTEGRATION_TIME_MS 5
#define SHARPENER_PERCENT 15
#define TARGET_ORDER SF_VL53L5CX_TARGET_ORDER::CLOSEST // can change to strongest if needed

L5CX sensor;
VL53L5CX_ResultsData data_holder;

uint16_t current_data_point;

void setup() {
    // communications setup
    USB_SERIAL.begin(115200);
    
    // sensor setup
    Wire.begin();            // set up i2c
    Wire.setClock(400000);  // max i2c speed

    // keep initializing the sensor until it does (doesn't always work first time)
    bool init_status;
    do {
        init_status = sensor.begin();
        delay(10);
    } while (!init_status);

    init_status = sensor.setResolution(64);   // resolution should be on 8x8
    if (sensor.getResolution() != 64 || !init_status) {
        // something's wrong
        USB_SERIAL.println("failed to set resolution");
        for (;;) delay(10); // halt
    }

    init_status = sensor.setRangingFrequency(15); // run at 15Hz, fastest on the L5CX
    if (sensor.getRangingFrequency() != 15 || !init_status) {
        USB_SERIAL.println("failed to set frequency");
        for (;;) delay(10); // halt
    }

    init_status = sensor.setIntegrationTime(INTEGRATION_TIME_MS);
    if (sensor.getIntegrationTime() != INTEGRATION_TIME_MS || !init_status) {
        USB_SERIAL.println("failed to set integration time");
        for (;;) delay(10); // halt
    }

    /* // doesn't seem to work?
    init_status = sensor.setSharpenerPercent(SHARPENER_PERCENT);
    if (sensor.getSharpenerPercent() != SHARPENER_PERCENT || !init_status) {
        USB_SERIAL.println("failed to set sharpener %");
        for (;;) delay(10); // halt
    }

    init_status = sensor.setTargetOrder(TARGET_ORDER);
    if (sensor.getTargetOrder() != TARGET_ORDER || !init_status) {
        USB_SERIAL.println("failed to set target order");
        for (;;) delay(10); // halt
    }
    */

    // sensor has to start and stop ranging, so we start it
    init_status = sensor.startRanging();
    if (!init_status) {
        USB_SERIAL.println("failed to start ranging");
        for (;;) delay(10); // halt
    }
}

void loop() {
    // write a packet containing all mm distance values every time polling is available
    // header: '[', footer: ']'
    if (sensor.isDataReady()) {
        if (sensor.getRangingData(&data_holder)) {
            // send header
            USB_SERIAL.write('[');

            // write each data point (uint16_t)
            for (uint8_t i = 0; i < 64; i++) {
                uint8_t y = i / 8;        // y increases by 1 every 8
                uint8_t x = 7 - (i % 8);  // x cycles from 7 through 0
                current_data_point = data_holder.distance_mm[y * 8 + x];

                // write the data big-endian (would little-endian be better?)
                char* ptr = (char*) &current_data_point;
                USB_SERIAL.write(*ptr);
                USB_SERIAL.write(*(ptr + 1));
            }

            // send footer
            USB_SERIAL.write(']');
        }
    }
}
