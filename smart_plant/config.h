#ifndef CONFIG_H
#define CONFIG_H

extern const char* THING_NAME;
extern const char* CLIENT_ID;

// pines
#define SENSOR_HUMEDAD 35
#define IN1_MOTOR 32
#define IN2_MOTOR 33
#define LED_VERDE 23
#define LED_ROJO 22

// calibración
const int SENSOR_SECO = 2600;
const int SENSOR_MOJADO = 800;

const int ADC_ERROR_MIN = 50;
const int ADC_ERROR_MAX = 4050;

#endif