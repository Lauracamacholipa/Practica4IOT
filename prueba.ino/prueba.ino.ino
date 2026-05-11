// ======================================================
// PRUEBA INTEGRADA SENSOR + BOMBA
// ESP32 + HW-080 + L298N
// ======================================================

// ---------------- PINES ----------------
#define SENSOR_HUMEDAD 15

#define IN1_MOTOR 32
#define IN2_MOTOR 33


// -------- CALIBRACION SENSOR --------
// Tus valores reales medidos
const int SENSOR_SECO = 2300;
const int SENSOR_MOJADO = 800;


// -------- UMBRALES AUTOMATICOS --------
const int HUMEDAD_MINIMA = 30; // encender bomba
const int HUMEDAD_MAXIMA = 70; // apagar bomba


// -------- VARIABLES --------
bool irrigationOn = false;

unsigned long previousMillis = 0;
const unsigned long interval = 1000; // leer cada 1 segundo


// ======================================================
// FUNCIONES
// ======================================================

void turnPumpOn() {

  digitalWrite(IN1_MOTOR, HIGH);
  digitalWrite(IN2_MOTOR, LOW);

  irrigationOn = true;

  Serial.println("BOMBA ENCENDIDA");
}

void turnPumpOff() {

  digitalWrite(IN1_MOTOR, LOW);
  digitalWrite(IN2_MOTOR, LOW);

  irrigationOn = false;

  Serial.println("BOMBA APAGADA");
}

int readHumidity() {

  int rawValue = analogRead(SENSOR_HUMEDAD);

  // Convertir ADC -> porcentaje
  int humidity = map(
    rawValue,
    SENSOR_SECO,
    SENSOR_MOJADO,
    0,
    100
  );

  // Validar rango RF-03
  humidity = constrain(humidity, 0, 100);

  Serial.println("-------------------------");
  Serial.print("Valor ADC: ");
  Serial.println(rawValue);

  Serial.print("Humedad: ");
  Serial.print(humidity);
  Serial.println("%");

  return humidity;
}


// ======================================================
// SETUP
// ======================================================

void setup() {

  Serial.begin(115200);

  // Pines motor
  pinMode(IN1_MOTOR, OUTPUT);
  pinMode(IN2_MOTOR, OUTPUT);

  // Sensor
  pinMode(SENSOR_HUMEDAD, INPUT);

  // Bomba apagada al iniciar
  turnPumpOff();

  analogReadResolution(12);

  Serial.println("================================");
  Serial.println("SISTEMA DE RIEGO INICIADO");
  Serial.println("================================");
}


// ======================================================
// LOOP
// ======================================================

void loop() {

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {

    previousMillis = currentMillis;

    int humidity = readHumidity();


    // ==================================
    // LOGICA AUTOMATICA
    // ==================================

    // Tierra seca
    if (humidity <= HUMEDAD_MINIMA) {

      if (!irrigationOn) {

        Serial.println(
          "Tierra seca -> Encendiendo riego"
        );

        turnPumpOn();
      }
    }

    // Tierra humeda
    else if (humidity >= HUMEDAD_MAXIMA) {

      if (irrigationOn) {

        Serial.println(
          "Tierra humeda -> Apagando riego"
        );

        turnPumpOff();
      }
    }

    // Zona intermedia
    else {

      Serial.println(
        "Humedad estable -> Mantener estado"
      );
    }

    Serial.print("Estado bomba: ");

    if (irrigationOn)
      Serial.println("ENCENDIDA");
    else
      Serial.println("APAGADA");
  }
}