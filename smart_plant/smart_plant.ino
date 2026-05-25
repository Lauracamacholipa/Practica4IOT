#include "config.h"
#include "sensor.h"
#include "pump.h"
#include "leds.h"
#include "aws_iot.h"

int humidity;

void setup() {
    Serial.begin(115200);

    initSensor();
    initPump();
    initLeds();
    initAWS();
}

void loop() {

    loopAWS();

    humidity = readHumidity();

    if(!isSensorOk()){
        ledError();
        turnPumpOff();
        return;
    }

    if(humidity < 30){
        ledDry();
        turnPumpOn();
    }
    else if(humidity > 70){
        ledWet();
        turnPumpOff();
    }
    else{
        setLeds(false,true);
    }

    publishTelemetry(
        humidity,
        isPumpOn()
    );

    delay(5000);
}