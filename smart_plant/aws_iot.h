#ifndef AWS_IOT_H
#define AWS_IOT_H

void initAWS();
void loopAWS();
void publishTelemetry(
    int humidity,
    bool irrigation
);

#endif