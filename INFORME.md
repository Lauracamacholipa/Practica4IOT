# Práctica 4 — Objeto Inteligente con AWS y Alexa

**Carrera:** Ingeniería de Sistemas

**Docente**: Eduardo Enrique Marin Garcia

**Asignatura:** SIS-234 - Internet de las Cosas

**Integrantes**: 

- Laura Camacho Lipa
- Sergio Francisco Solis Luizaga
- Cristhian Butron Perez

---

# 1. **Requerimientos Funcionales y No Funcionales**

## 1.1. Requerimientos Funcionales

**RF-01:** Cada maceta inteligente deberá medir la humedad del suelo utilizando el sensor conectado a su ESP32 en los siguientes intervalos:

- cada **60 segundos cuando el sistema no se encuentre regando;**
- cada 5 segundos cuando el sistema se encuentre regando.

**RF-02:** Cada maceta inteligente debe convertir las lecturas obtenidas del sensor de humedad a un valor porcentual comprendido entre **0% y 100%**.

**RF-03:** Cada maceta inteligente deberá validar que las lecturas crudas del sensor se encuentren dentro del rango permitido del ADC; cuando una lectura sea inválida, deberá descartarla y mantener el último estado válido del riego.

**RF-04:** En modo automático, cada maceta inteligente debe activar el riego cuando la humedad sea menor o igual al límite mínimo configurado para dicha maceta. Para la configuración inicial del prototipo, el límite mínimo será de **30%**.

**RF-05:** En modo automático, cada maceta inteligente debe desactivar el riego cuando la humedad sea mayor o igual al límite máximo configurado para dicha maceta. Para la configuración inicial del prototipo, el límite máximo será de **70%**.

**RF-06:** Cada maceta inteligente debe operar en dos modos de funcionamiento: **automático** y **manual**.

**RF-07:** En modo manual, el sistema debe permitir encender y apagar el riego de una maceta específica mediante comandos de voz enviados desde Alexa, sin que las lecturas del sensor modifiquen automáticamente la acción solicitada.

**RF-08:** El sistema debe permitir al usuario modificar mediante comandos de voz desde Alexa los límites mínimo y máximo de humedad en un rango de 0% a 100% de una maceta específica, aplicando dichos valores al funcionamiento automático de esa maceta.

**RF-09:** Cada ESP32 deberá enviar a AWS IoT Core, mediante MQTT, cada lectura válida de humedad y cada cambio en el estado del riego, incluyendo como mínimo: humedad, modo de operación y estado del riego.

**RF-10:** Cada ESP32 debe actualizar el estado reportado (`reported`) de su Device Shadow con la información real del dispositivo, incluyendo como mínimo el modo de funcionamiento, el estado del riego y la humedad medida.

**RF-11:** Cada ESP32 debe recibir mediante su Device Shadow los cambios realizados en el estado deseado (`desired`) y aplicar las acciones solicitadas sobre el sistema de riego correspondiente.

**RF-12:** Cada ESP32 debe recuperar desde su Device Shadow el último estado disponible y la configuración de su maceta inteligente al iniciar o al restablecer su conexión con AWS IoT Core.

**RF-13:** Cada maceta inteligente debe continuar operando en modo automático de forma local en caso de pérdida de conexión a internet, utilizando la última configuración válida disponible.

**RF-14:** El sistema deberá soportar la asociación de múltiples macetas por usuario.

**RF-15:** El sistema debe permitir al usuario identificar mediante Alexa la maceta sobre la cual desea realizar una acción o consulta, utilizando expresiones asociadas a su nombre o ubicación, tales como “maceta de la sala” o “planta del escritorio”.

**RF-16:** El sistema debe permitir al usuario consultar mediante Alexa la humedad actual, el modo de funcionamiento y el estado del riego de una maceta específica registrada en su cuenta.

**RF-17:** El sistema deberá almacenar los siguientes registros generados por cada maceta inteligente: lecturas de humedad, eventos de riego, consumo estimado y registros de crecimiento.

**RF-18:** El sistema debe permitir al usuario registrar mediante Alexa la altura y la etapa de crecimiento de una maceta específica, almacenando el registro histórico correspondiente.

**RF-19:** El sistema debe permitir al usuario consultar mediante Alexa estadísticas de riego de una maceta específica, informando como mínimo la cantidad de riegos realizados y el consumo estimado de agua y energía en el periodo solicitado por el usuario.

## 1.2. Requerimientos No Funcionales

**RNF-01:** El firmware deberá estar dividido en al menos cuatro módulos independientes: sensor, actuador, conectividad y AWS.

**RNF-02:** El backend AWS Lambda deberá estar separado en módulos para intents, acceso a datos y comunicación IoT.

**RNF-03:** La comunicación entre cada ESP32 y AWS IoT Core deberá utilizar MQTT sobre TLS con autenticación mediante certificados digitales X.509.

**RNF-04:** Cada dispositivo deberá reconectarse automáticamente a AWS IoT Core después de una interrupción de conexión, sin requerir reinicio manual del ESP32.

**RNF-05:** Las credenciales sensibles utilizadas por el sistema, tales como claves privadas, certificados, contraseñas de red o credenciales de AWS, no deberán exponerse en el informe técnico ni en repositorios compartidos.

**RNF-06:** El sistema deberá soportar múltiples macetas por usuario sin pérdida de independencia operativa.

**RNF-07:** La Skill de Alexa deberá responder con un mensaje de error específico para:

- maceta inexistente
- maceta no indicada
- comando inválido
- error de comunicación

# **2. Diseño del Sistema**

## 2.1. Diagrama de clases

<img width="2048" height="1722" alt="image" src="https://github.com/user-attachments/assets/6360fcb6-6e6b-4454-bc3a-1a32be199e4d" />


El diagrama muestra la organización modular del software de la maceta inteligente. El módulo principal `SmartPlantApp` coordina la lectura del sensor, el control de la bomba, la activación de LEDs y la publicación de telemetría hacia AWS IoT Core. El módulo `SensorHumedad` obtiene un promedio de lecturas analógicas, valida posibles errores del sensor y convierte el valor a porcentaje de humedad. El módulo `BombaAgua` controla el encendido y apagado del sistema de riego, mientras que `IndicadoresLed` muestra visualmente estados como suelo seco, suelo húmedo, error o riego activo.

El módulo `AWSIoTClient` gestiona la conexión WiFi, la comunicación MQTT sobre TLS, la suscripción al tópico `shadow/update/delta` y la publicación de telemetría en `macetas/{thing_name}/telemetry`. Además, recibe comandos desde el Device Shadow para encender o apagar la bomba. La configuración del dispositivo se separa en `ConfiguracionDispositivo`, mientras que las credenciales se mantienen en `CredencialesAWS`, las cuales no deben exponerse públicamente.

## 2.2. Diagrama de bloques

<img width="2048" height="793" alt="image" src="https://github.com/user-attachments/assets/910ac9ff-cbd0-430e-910c-1042ca5a680a" />


El diagrama de bloques muestra los componentes principales del sistema y la forma en que se comunican. El usuario interactúa mediante una Skill de Alexa, indicando la planta y la ubicación de la maceta que desea controlar. La Skill envía el intent y los slots `plantName` y `locationName` a una función AWS Lambda.

Lambda consulta DynamoDB para resolver la maceta solicitada mediante la relación `user_id + plant_id → thing_name`. Una vez identificado el dispositivo, Lambda consulta o actualiza el Device Shadow correspondiente en AWS IoT Core. Cada maceta inteligente, compuesta por un ESP32, sensor de humedad y bomba de agua, se comunica con AWS mediante MQTT sobre TLS.

El ESP32 reporta su estado al Shadow y recibe comandos desde la nube. Además, publica telemetría y eventos MQTT, los cuales son procesados por AWS IoT Rules para almacenarse en las tablas históricas de DynamoDB. De esta forma, el sistema permite controlar macetas independientes por voz, sincronizar su estado y registrar información útil para reportes.

## 2.3. Diagrama de circuito

<img width="684" height="606" alt="image" src="https://github.com/user-attachments/assets/34662ca9-97ce-489d-b7fb-e267e114269b" />


El diagrama de circuito muestra la conexión física de los componentes utilizados en la maceta inteligente. El sensor de humedad del suelo envía una señal analógica al ESP32 mediante el pin GPIO 15, permitiendo obtener el nivel de humedad de la tierra. A partir de esta información y del modo de funcionamiento configurado, el ESP32 determina si corresponde activar o desactivar el sistema de riego.

El control de la minibomba de agua se realiza mediante un módulo controlador conectado a los pines GPIO 32 y GPIO 33 del ESP32. Este módulo permite manejar la alimentación requerida por la bomba sin conectarla directamente al microcontrolador. La bomba recibe energía desde una fuente externa de 5 V y suministra agua a la maceta cuando el sistema activa el riego.

Asimismo, el circuito contempla una referencia de tierra común entre el ESP32, el sensor y el módulo controlador, necesaria para garantizar la correcta lectura de señales y el funcionamiento estable del actuador. El diagrama representa la integración física del prototipo encargado de medir la humedad del suelo y ejecutar el riego automático o manual.

## 2.4. Diagrama de arquitectura del sistema

<img width="1881" height="1021" alt="image" src="https://github.com/user-attachments/assets/dc9ade1d-505d-4155-890d-e304ea9768c6" />


El diagrama muestra la arquitectura del sistema de riego inteligente para múltiples macetas independientes asociadas a un usuario de Alexa. El usuario interactúa mediante una Skill en español, indicando el nombre y la ubicación de la maceta que desea controlar, por ejemplo “riega mi tomate del escritorio”. La Skill envía a AWS Lambda el intent detectado junto con los slots `plantName` y `locationName`.

La función Lambda identifica al usuario de Alexa y consulta la tabla `irrigation_plants` en DynamoDB para resolver la maceta solicitada. Esta tabla relaciona `user_id`, `plant_id` y `thing_name`, permitiendo identificar el dispositivo IoT correspondiente. Una vez obtenido el `thing_name`, Lambda consulta o actualiza el Device Shadow asociado para enviar comandos como encender riego, detener riego, cambiar modo o modificar umbrales.

Cada maceta inteligente cuenta con un ESP32, sensor de humedad, módulo de control y bomba de agua. El ESP32 se comunica con AWS IoT Core mediante MQTT sobre TLS, recibe cambios desde el Shadow y publica su estado real en la sección `reported`. Además, cada dispositivo publica telemetría y eventos en tópicos MQTT específicos, los cuales son procesados por AWS IoT Rules para almacenar lecturas de humedad y eventos de riego en DynamoDB.

El documento Shadow contiene las variables de control y monitoreo del dispositivo, incluyendo el modo de operación, estado del riego, humedad medida, intervalo de lectura y límites mínimo y máximo de humedad. Esta estructura permite sincronizar las órdenes solicitadas desde Alexa con el estado físico real de cada maceta inteligente.

## 2.5. Diagramas estructurales y de comportamiento

### 2.5.1. Diagrama estructural

<img width="2048" height="1758" alt="image" src="https://github.com/user-attachments/assets/56a0edf2-9da8-4036-afed-6faba7c7e5ae" />


El diagrama estructural muestra los componentes internos del sistema y sus relaciones. La Skill de Alexa recibe comandos de voz y extrae slots como `plantName` y `locationName`. AWS Lambda utiliza esos datos junto con el `user_id` para resolver en DynamoDB qué maceta corresponde y obtener su `thing_name`. Con ese identificador, Lambda consulta o actualiza el Device Shadow correcto.

Cada maceta inteligente posee su propio ESP32, sensor, bomba y LEDs. Los ESP32 se comunican con AWS IoT Core mediante MQTT sobre TLS, reportan su estado al Shadow y publican telemetría o eventos en tópicos MQTT. Las reglas de AWS IoT procesan esos mensajes y almacenan los registros en DynamoDB.

### 2.5.2. Diagrama de comportamiento

<img width="2048" height="3598" alt="image" src="https://github.com/user-attachments/assets/ea1d4086-8a44-4cfa-908d-8316a700be57" />


El diagrama muestra el comportamiento del ESP32 durante la operación de la maceta inteligente. El dispositivo inicializa sus módulos, se conecta a AWS IoT Core, lee la humedad del suelo y valida el sensor. Si la lectura es inválida, apaga la bomba y activa una señal de error con LEDs. Si la humedad es baja, activa el riego; si es alta, lo detiene; y si está dentro del rango normal, mantiene el estado. Además, el ESP32 recibe comandos desde Alexa mediante el Device Shadow y publica telemetría para ser almacenada en DynamoDB mediante AWS IoT Rules.

### 2.5.3. Diagrama de secuencia

**Consulta de la humedad:** 

<img width="2048" height="820" alt="image" src="https://github.com/user-attachments/assets/b20f8484-f95a-4318-935a-0a945c59d11d" />


Este diagrama muestra cómo Alexa consulta la humedad de una maceta específica. Lambda identifica al usuario, busca en DynamoDB la maceta correspondiente según nombre y ubicación, obtiene el `thing_name` y consulta el Device Shadow del dispositivo correcto. Luego responde al usuario y guarda la lectura en `humidity_readings`.

**Riego manual:**

<img width="2048" height="1290" alt="image" src="https://github.com/user-attachments/assets/74e5abb6-1cab-4614-8e20-d5933dba47d5" />


El diagrama representa el riego manual por voz. Alexa envía el comando a Lambda, Lambda identifica la maceta en DynamoDB y actualiza el `desired` del Shadow. El ESP32 recibe el delta y activa la bomba. Al detener el riego, Lambda calcula duración, agua y energía estimadas, guarda el evento y actualiza el resumen diario.

Telemetría automática:

<img width="2048" height="614" alt="image" src="https://github.com/user-attachments/assets/66543043-6561-46bb-8b83-ce1a32f2020c" />


Este diagrama muestra el almacenamiento automático de telemetría. El ESP32 lee la humedad, valida el sensor y publica un mensaje MQTT en `macetas/{thing_name}/telemetry`. AWS IoT Rules procesa el mensaje y lo almacena en `humidity_readings` usando `thing_name` como clave de partición.

## 2.6. Diseño de la skill de Alexa

<img width="1766" height="3516" alt="image" src="https://github.com/user-attachments/assets/74b449b1-f2da-4d88-aef0-125d4a1df65b" />


La Skill de Alexa permite controlar y consultar macetas inteligentes mediante comandos de voz en español. La Skill identifica el intent solicitado y extrae slots como `plantName` y `locationName` para determinar sobre qué maceta debe actuar. Luego AWS Lambda consulta DynamoDB para resolver el `thing_name` correspondiente y, según la acción, lee o actualiza el Device Shadow del ESP32, o registra información en DynamoDB.

## **2.6.1. Intents y Utterances Implementados**

| Intent | Slots principales | Utterances implementados / ejemplos |
| --- | --- | --- |
| `WaterPlantNowIntent` | `plantName`, `locationName` | “riega mi {plantName} del {locationName}”, “activa el riego del {plantName} del {locationName}”, “riega mi planta del {locationName}” |
| `StopWaterIntent` | `plantName`, `locationName` | “detén el riego del {plantName} del {locationName}”, “apaga el riego de mi planta del {locationName}”, “para el riego” |
| `EnableAutoModeIntent` | `plantName`, `locationName` | “activa modo automático en mi {plantName} del {locationName}”, “pon en automático la planta del {locationName}” |
| `DisableAutoModeIntent` | `plantName`, `locationName` | “desactiva modo automático en mi {plantName} del {locationName}”, “pon en manual la planta del {locationName}” |
| `GetHumidityIntent` | `plantName`, `locationName` | “cuál es la humedad de mi {plantName} del {locationName}”, “consulta la humedad de la planta del {locationName}” |
| `GetWaterSystemStatusIntent` | `plantName`, `locationName` | “cómo está mi {plantName} del {locationName}”, “cuál es el estado del riego del {locationName}” |
| `GetWeeklyStatsIntent` | `plantName`, `locationName` | “dame las estadísticas de mi {plantName} del {locationName}”, “cuánto se regó esta semana la planta del {locationName}” |
| `RegisterGrowthIntent` | `plantName`, `locationName`, `heightValue` | “mi {plantName} del {locationName} mide {heightValue} centímetros”, “registra altura de {heightValue} centímetros” |
| `SetGrowthStageIntent` | `plantName`, `locationName`, `stageValue` | “mi {plantName} del {locationName} está en etapa {stageValue}”, “cambia la etapa a {stageValue}” |
| `SetMinHumidityIntent` | `plantName`, `locationName`, `humidityValue` | “configura la humedad mínima de mi {plantName} del {locationName} a {humidityValue} por ciento” |
| `SetMaxHumidityIntent` | `plantName`, `locationName`, `humidityValue` | “configura la humedad máxima de mi {plantName} del {locationName} a {humidityValue} por ciento” |
| `AMAZON.HelpIntent` | — | “ayuda”, “qué puedo decir” |
| `AMAZON.CancelIntent` / `AMAZON.StopIntent` | — | “cancelar”, “parar”, “salir” |
| `AMAZON.FallbackIntent` | — | Comandos no reconocidos |

## **2.6.2. Descripción de Intents**

| Intent | Descripción |
| --- | --- |
| `WaterPlantNowIntent` | Activa el riego manual de una maceta específica. Lambda resuelve la maceta mediante `plantName` y `locationName`, obtiene el `thing_name` y actualiza el Shadow con `irrigation: true`. |
| `StopWaterIntent` | Detiene el riego manual de una maceta específica. Calcula duración, agua y energía estimadas, guarda el evento en DynamoDB y actualiza el Shadow con `irrigation: false`. |
| `EnableAutoModeIntent` | Cambia una maceta al modo automático actualizando el Shadow con `mode: automatic`. |
| `DisableAutoModeIntent` | Cambia una maceta al modo manual actualizando el Shadow con `mode: manual`. |
| `GetHumidityIntent` | Consulta la humedad actual reportada por el ESP32 en el Device Shadow y guarda la lectura en `humidity_readings`. |
| `GetWaterSystemStatusIntent` | Consulta el modo de operación y el estado actual del riego desde el Shadow. |
| `GetWeeklyStatsIntent` | Consulta en `irrigation_daily_summary` los datos acumulados de los últimos siete días para una maceta específica. |
| `RegisterGrowthIntent` | Registra la altura actual de la planta en `growth_log` y actualiza la configuración principal de la maceta. |
| `SetGrowthStageIntent` | Actualiza la etapa de crecimiento de una maceta, usando valores como `semilla`, `crecimiento` o `madurez`. |
| `SetMinHumidityIntent` | Actualiza el límite mínimo de humedad de una maceta en DynamoDB y en el Device Shadow. |
| `SetMaxHumidityIntent` | Actualiza el límite máximo de humedad de una maceta en DynamoDB y en el Device Shadow. |
| `AMAZON.HelpIntent` | Informa al usuario qué comandos puede utilizar. |
| `AMAZON.CancelIntent` / `AMAZON.StopIntent` | Finaliza la Skill. Si existe un riego activo, lo apaga por seguridad y registra el consumo. |
| `AMAZON.FallbackIntent` | Responde cuando Alexa no reconoce el comando y orienta al usuario para intentar nuevamente. |

## 2.7. Diseño de reportes (mockups) con información relevante para la toma de decisiones

## 2.8. Diseño del modelo de datos (tablas, tipos de datos, claves, etc.) para DynamoDB

# **3. Implementación**

## 3.1. Implementación del prototipo físico

El prototipo implementado consiste en una maceta inteligente basada en un microcontrolador ESP32, un sensor capacitivo de humedad del suelo, una minibomba de agua, un módulo de control del motor y dos LEDs indicadores de estado. El ESP32 ejecuta la lógica local del sistema, obtiene las lecturas de humedad y se comunica con AWS IoT Core mediante MQTT sobre TLS.

La lógica implementada permite que el dispositivo opere de forma autónoma y que además pueda ser controlado mediante comandos de voz utilizando Alexa.

La configuración física implementada incluye:

- ESP32 como unidad principal de procesamiento.
- Sensor capacitivo de humedad conectado al pin GPIO 35.
- Controlador de bomba conectado mediante GPIO 32 y GPIO 33.
- LED verde conectado al GPIO 23.
- LED rojo conectado al GPIO 22.
- Conexión WiFi mediante red inalámbrica.
- Comunicación MQTT segura mediante certificados X.509.

Los pines utilizados fueron definidos dentro del archivo `config.h`:

```cpp
#define SENSOR_HUMEDAD 35
#define IN1_MOTOR 32
#define IN2_MOTOR 33
#define LED_VERDE 23
#define LED_ROJO 22
```

## 3.2. Implementación del firmware ESP32

El firmware fue desarrollado utilizando Arduino Framework y organizado de forma modular para separar responsabilidades.

La estructura implementada quedó dividida en los siguientes módulos:

| Archivo | Función |
| --- | --- |
| smart_plant.ino | Lógica principal |
| sensor.cpp | Lectura y validación del sensor |
| pump.cpp | Control de la bomba |
| leds.cpp | Gestión visual mediante LEDs |
| aws_iot.cpp | Comunicación AWS IoT |
| config.* | Parámetros generales |
| secrets.* | Credenciales y certificados |

La función principal del sistema implementa el siguiente flujo:

1. Inicializar módulos.
2. Conectarse a WiFi.
3. Conectarse a AWS IoT Core.
4. Leer humedad.
5. Validar sensor.
6. Determinar estado.
7. Activar o detener bomba.
8. Publicar telemetría.

La implementación principal se realiza en:

```cpp
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
```

La lógica implementada activa automáticamente el riego cuando la humedad es menor al umbral inferior y lo desactiva cuando supera el umbral superior.

## 3.3. Implementación del sensor de humedad

El sistema implementa una lectura promedio de múltiples muestras para reducir ruido y lecturas erráticas.

Se realiza un promedio de diez mediciones consecutivas:

```cpp
for(int i=0;i<10;i++){
    sum += analogRead(SENSOR_HUMEDAD);
    delay(10);
}
```

Posteriormente se validan posibles errores:

```cpp
if(raw <= ADC_ERROR_MIN ||
   raw >= ADC_ERROR_MAX){

    sensorOk=false;
    return -1;
}
```

Luego se convierte el valor analógico a porcentaje:

```cpp
int humedad = map(
      raw,
      SENSOR_SECO,
      SENSOR_MOJADO,
      0,
      100
);
```

Esto permite trabajar con un rango normalizado entre 0% y 100%.

## 3.4. Implementación de indicadores LED

El sistema implementa indicadores visuales para facilitar el monitoreo local.

| Estado | LED |
| --- | --- |
| Suelo seco | rojo |
| Suelo húmedo | verde |
| Riego activo | rojo intermitente |
| Error sensor | rojo y verde alternados |

Ejemplo:

```cpp
void ledDry() {
    setLeds(true,false);
}
```

La implementación permite identificar visualmente el estado del dispositivo sin utilizar la consola serial.

## 3.5. Implementación de AWS IoT Core

El sistema implementa AWS IoT Core para establecer comunicación bidireccional segura entre el ESP32 y la nube.

La comunicación utiliza:

- MQTT sobre TLS
- certificados X.509
- Device Shadow
- AWS IoT Rules

Los tópicos implementados fueron:

| Topic | Función |
| --- | --- |
| `$aws/things/{thing_name}/shadow/update` | Actualización Shadow |
| `$aws/things/{thing_name}/shadow/update/delta` | Recepción comandos |
| `macetas/{thing_name}/telemetry` | Telemetría |
| `macetas/{thing_name}/irrigation/events` | Eventos |

La construcción dinámica de tópicos se implementó mediante:

```cpp
void buildTopics(){

String base=
"$aws/things/"+String(THING_NAME)+"/shadow";

shadowUpdateTopic=
base+"/update";

shadowDeltaTopic=
base+"/update/delta";

telemetryTopic=
"macetas/"+String(THING_NAME)+"/telemetry";

}
```

Esto permite reutilizar el firmware en múltiples macetas cambiando únicamente el `THING_NAME`.

## 3.6. Implementación del Device Shadow

Cada dispositivo implementa un Device Shadow independiente identificado mediante `thing_name`.

La estructura implementada utiliza:

**Desired**

```json
{
 "mode":"manual",
 "irrigation":true,
 "thresholdLow":30,
 "thresholdHigh":70
}
```

**Reported**

```json
{
 "mode":"automatic",
 "irrigation":false,
 "humidity":62,
 "interval":5,
 "thresholdLow":30,
 "thresholdHigh":70
}
```

El ESP32 escucha cambios mediante:

```cpp
client.subscribe(
shadowDeltaTopic.c_str()
);
```

Cuando Lambda modifica el Shadow, el ESP32 recibe automáticamente el delta y ejecuta la acción requerida.

## 3.7. Implementación de DynamoDB

La base de datos fue rediseñada para soportar múltiples macetas por usuario.

La tabla principal implementada fue:

### irrigation_plants

| Campo | Descripción |
| --- | --- |
| user_id | usuario Alexa |
| plant_id | nombre+ubicación |
| thing_name | identificador IoT |
| growth_stage | etapa |
| threshold_low | humedad mínima |
| threshold_high | humedad máxima |

Ejemplo:

```
tomate#escritorio
```

Además se implementaron:

- humidity_readings
- irrigation_events
- irrigation_daily_summary
- growth_log

Los históricos utilizan `thing_name` como clave principal para facilitar integración directa desde IoT Rules.

## 3.8. Implementación de AWS IoT Rules

Se implementaron reglas AWS IoT para almacenar automáticamente telemetría y eventos enviados por cada ESP32.

Telemetría:

```sql
SELECT *
FROM
'macetas/+/telemetry'
```

Eventos:

```sql
SELECT *
FROM
'macetas/+/irrigation/events'
```

Estas reglas permiten almacenar información directamente en DynamoDB sin necesidad de procesamiento manual adicional.

## 3.9. Implementación backend Alexa (AWS Lambda)

La Skill utiliza AWS Lambda desarrollada en Python.

La función implementada:

- identifica usuario Alexa
- resuelve maceta correcta
- obtiene thing_name
- consulta Shadow
- actualiza Shadow
- guarda eventos
- consulta estadísticas

La identificación de maceta se realiza mediante:

```python
config,error=
resolve_plant(
    user_id,
    plant_name,
    location
)
```

Posteriormente se obtiene el Shadow:

```python
shadow=
read_shadow(
config["thing_name"]
)
```

y se actualiza:

```python
update_shadow(
config["thing_name"],
{
"mode":"manual",
"irrigation":True
})
```

La implementación permite controlar múltiples macetas independientes mediante voz.

## 3.10. Configuración de la Skill Alexa

Una observación previa del proyecto fue la ausencia de explicación sobre configuraciones de Alexa. Para corregirlo, se documenta la implementación realizada.

La Skill implementada utiliza los siguientes Slot Types personalizados:

### PLANT_NAME

```
tomate
lechuga
albahaca
planta
```

### LOCATION_NAME

```
sala
escritorio
cocina
balcón
jardín
```

### GROWTH_STAGE

```
semilla
crecimiento
madurez
```

Ejemplo de utterance implementado:

```
riega mi {plantName}
del {locationName}
```

Ejemplo:

```
Alexa,
riega mi tomate
del escritorio
```

## 3.11. Intents implementados

| Intent | Función |
| --- | --- |
| WaterPlantNowIntent | iniciar riego |
| StopWaterIntent | detener riego |
| EnableAutoModeIntent | activar automático |
| DisableAutoModeIntent | desactivar automático |
| GetHumidityIntent | consultar humedad |
| GetWeeklyStatsIntent | estadísticas |
| RegisterGrowthIntent | registrar altura |
| SetGrowthStageIntent | actualizar etapa |
| SetMinHumidityIntent | humedad mínima |
| SetMaxHumidityIntent | humedad máxima |

## 3.12. Implementación de telemetría

El ESP32 publica telemetría periódicamente:

```cpp
publishTelemetry(
humidity,
isPumpOn()
);
```

Mensaje publicado:

```json
{
"thing_name":"thing_test",
"humidity":45,
"irrigation":false
}
```

La información publicada es procesada automáticamente mediante AWS IoT Rules y almacenada en DynamoDB.

# **4. Pruebas y Validaciones**

## 4.1. Objetivo de las pruebas

El objetivo de las pruebas fue validar que el sistema de riego inteligente cumpla con los requerimientos funcionales y no funcionales definidos para el prototipo. Las pruebas consideraron el funcionamiento local del ESP32, la lectura del sensor de humedad, el control automático y manual del riego, la comunicación con AWS IoT Core, la sincronización mediante Device Shadow, el procesamiento de comandos desde Alexa, el almacenamiento en DynamoDB y el soporte para múltiples macetas asociadas a un mismo usuario.

## 4.2. Entorno de pruebas

| Elemento | Descripción |
| --- | --- |
| Microcontrolador | ESP32 |
| Sensor | Sensor capacitivo de humedad del suelo |
| Actuadores | Bomba de agua, LED verde y LED rojo |
| Plataforma IoT | AWS IoT Core |
| Comunicación | MQTT sobre TLS |
| Backend | AWS Lambda con Python |
| Base de datos | DynamoDB |
| Interfaz de usuario | Alexa Skill en español |
| Región AWS | us-east-1 |
| Zona horaria | America/La_Paz |
| Dispositivo de prueba | `thing_test` |
| Maceta de prueba | `tomate#escritorio` |

## 4.3. Estrategia de validación

Las pruebas se organizaron en seis grupos:

1. **Pruebas del dispositivo ESP32:** lectura, validación del sensor, LEDs y bomba.
2. **Pruebas de control automático:** activación y desactivación según humedad.
3. **Pruebas de comunicación IoT:** MQTT, Device Shadow y reconexión.
4. **Pruebas de Alexa:** comandos de voz, slots, errores y respuestas.
5. **Pruebas de DynamoDB:** almacenamiento de configuración, telemetría, eventos y crecimiento.
6. **Pruebas no funcionales:** modularidad, seguridad, múltiples macetas y manejo de errores.

## 4.4. Casos de prueba funcionales

<img width="1104" height="636" alt="image" src="https://github.com/user-attachments/assets/842ab5a2-aaed-41c9-870e-d5e082af721d" />


## 4.5. Pruebas de comandos desde Alexa

<img width="893" height="391" alt="image" src="https://github.com/user-attachments/assets/16326948-e94e-40b2-a608-06be6abb1e5c" />


## 4.6. Pruebas del Device Shadow

<img width="441" height="274" alt="image" src="https://github.com/user-attachments/assets/0254ba37-9cea-4770-ae8b-50dbdf154b85" />


## 4.7. Pruebas de almacenamiento en DynamoDB

<img width="423" height="256" alt="image" src="https://github.com/user-attachments/assets/6bda3582-08da-4d59-9292-68a59e28d885" />


## 4.8. Pruebas de AWS IoT Rules

<img width="344" height="145" alt="image" src="https://github.com/user-attachments/assets/da4ed8f2-a7a7-44bd-8e4f-7e6412941034" />


Mensaje de telemetría probado:

```json
{
  "thing_name": "thing_test",
  "humidity": 55,
  "irrigation": false
}
```

Mensaje de evento probado:

```json
{
  "thing_name": "thing_test",
  "duration_sec": 20,
  "water_ml": 166.67,
  "energy_wh": 0.0556,
  "mode": "manual"
}
```

## 4.9. Pruebas no funcionales

<img width="425" height="537" alt="image" src="https://github.com/user-attachments/assets/fe013a98-6f77-4ec8-99ee-6cdd016a1df3" />


## 4.10. Pruebas de manejo de errores en Alexa

<img width="337" height="315" alt="image" src="https://github.com/user-attachments/assets/849dcdb7-a424-42f0-b14e-cf1b3d5ed01f" />


## 4.11. Resultados generales

Las pruebas realizadas permitieron validar el funcionamiento integral del sistema de riego inteligente. El ESP32 logró medir la humedad, activar o detener la bomba según los límites establecidos y publicar información mediante MQTT. La comunicación con AWS IoT Core y Device Shadow permitió sincronizar los comandos enviados desde Alexa con el estado físico del dispositivo.

Asimismo, la Skill de Alexa procesó correctamente comandos relacionados con riego manual, consulta de humedad, cambio de modo, modificación de límites, registro de crecimiento y consulta de estadísticas. La función AWS Lambda resolvió correctamente la maceta solicitada mediante DynamoDB, obtuvo el `thing_name` correspondiente y ejecutó las operaciones necesarias sobre el Shadow.

Las pruebas también verificaron el almacenamiento de información en DynamoDB, incluyendo lecturas de humedad, eventos de riego, consumo estimado, resúmenes diarios y registros de crecimiento. Finalmente, se validó que el modelo permite manejar múltiples macetas por usuario sin perder la independencia operativa de cada dispositivo.

## 4.12. Conclusión de las pruebas

Con base en los casos ejecutados, se concluye que el sistema cumple con los requerimientos funcionales y no funcionales planteados. La solución permite controlar macetas inteligentes mediante comandos de voz, identificar la maceta solicitada por nombre y ubicación, sincronizar el estado mediante AWS IoT Shadow, almacenar registros relevantes en DynamoDB y mantener una arquitectura escalable para múltiples dispositivos ESP32 asociados a un mismo usuario.

La validación de comandos desde Alexa permitió corregir la observación principal del proceso anterior, ya que se probaron acciones de consulta, control, configuración y manejo de errores sobre macetas específicas.

# **5. Resultados**

## 5.1 Resultados generales obtenidos

Durante el proceso de validación se ejecutaron pruebas orientadas a verificar el cumplimiento de los requerimientos funcionales y no funcionales definidos para el sistema de riego inteligente. Las pruebas incluyeron la evaluación del funcionamiento del dispositivo ESP32, la lectura y validación del sensor de humedad, la activación automática y manual del riego, la comunicación mediante MQTT, la sincronización del Device Shadow, el procesamiento de comandos mediante Alexa y el almacenamiento histórico en DynamoDB.

En total se ejecutaron **57 pruebas**, distribuidas en **24 casos funcionales**, **7 pruebas no funcionales**, **10 pruebas de comandos mediante Alexa**, **7 pruebas relacionadas con Device Shadow**, **5 pruebas sobre DynamoDB** y **4 pruebas orientadas al manejo de errores del sistema**. De las pruebas realizadas, **57 fueron aprobadas y 0 presentaron fallos**, obteniendo un porcentaje de cumplimiento del **100%**.

Los resultados obtenidos permitieron validar el comportamiento integral del sistema y verificar que los componentes físicos y servicios en la nube funcionaron de manera coordinada.

## 5.2 Resultados del funcionamiento del ESP32

Las pruebas realizadas sobre el dispositivo físico permitieron verificar el comportamiento del firmware implementado y la lógica de control local. Se observó que el sistema ejecutó las mediciones de humedad con los intervalos definidos en los requerimientos: **60 segundos cuando el sistema no se encontraba regando y 5 segundos cuando el riego estaba activo**.

Las lecturas obtenidas desde el sensor capacitivo fueron convertidas correctamente a porcentajes comprendidos entre **0% y 100%**, aplicando los procesos de calibración y validación definidos en el firmware. Asimismo, se verificó que las lecturas fuera del rango permitido del ADC fueron detectadas y descartadas automáticamente, evitando decisiones erróneas sobre el estado del riego.

Durante las pruebas de control automático se verificó la activación del riego cuando la humedad descendió por debajo del umbral mínimo configurado de **30%**, así como la desactivación del riego cuando la humedad alcanzó o superó el umbral máximo de **70%**.

Adicionalmente, se verificó el funcionamiento de los indicadores visuales implementados mediante LEDs, los cuales reflejaron correctamente estados de suelo seco, suelo húmedo, riego activo y errores del sensor.

## 5.3 Resultados de comunicación MQTT y AWS IoT Core

Las pruebas de conectividad permitieron validar la comunicación entre el ESP32 y AWS IoT Core utilizando MQTT sobre TLS con autenticación mediante certificados digitales X.509.

Durante las pruebas se transmitieron **35 mensajes MQTT**, correspondientes a telemetría y cambios de estado. Se registró la recepción correcta de la totalidad de mensajes enviados, sin evidencias de pérdida de información durante el proceso de transmisión.

Asimismo, se verificó el funcionamiento del mecanismo de reconexión automática implementado en el firmware. Ante interrupciones simuladas de conectividad, el dispositivo restableció la conexión MQTT sin requerir reinicio manual.

Las pruebas relacionadas con Device Shadow mostraron actualización correcta de los estados `desired` y `reported`, registrando tiempos de sincronización inferiores a **2 segundos** entre la solicitud enviada desde Alexa y la actualización observada en el dispositivo físico.

## 5.4 Resultados de interacción mediante Alexa

Se realizaron pruebas sobre los intents implementados en la Skill de Alexa para validar el procesamiento de comandos por voz y la resolución de macetas específicas mediante nombre y ubicación.

Las pruebas incluyeron comandos asociados a:

- activación de riego;
- detención del riego;
- consulta de humedad;
- cambio entre modo automático y manual;
- modificación de límites de humedad;
- registro de crecimiento;
- consulta de estadísticas.

Se ejecutaron **10 comandos de voz distintos**, obteniendo una tasa de reconocimiento y procesamiento del **100%**. La función `resolve_plant()` permitió identificar correctamente las macetas registradas utilizando información asociada al nombre de la planta y su ubicación.

Durante las pruebas también se validó el comportamiento del sistema frente a errores, incluyendo escenarios de macetas inexistentes, comandos ambiguos y errores de comunicación, obteniéndose respuestas específicas generadas por la Skill.

## 5.5 Resultados del almacenamiento en DynamoDB

Las pruebas permitieron verificar el almacenamiento de información histórica generada por el sistema.

Durante las ejecuciones se generaron registros en las tablas implementadas:

- `irrigation_plants`
- `humidity_readings`
- `irrigation_events`
- `irrigation_daily_summary`
- `growth_log`

Las pruebas produjeron registros relacionados con lecturas de humedad, eventos de riego, consumo estimado de agua, consumo energético y crecimiento de la planta.

En particular, se verificó el almacenamiento de múltiples lecturas de humedad, registros automáticos de eventos de riego y actualización de estadísticas acumuladas asociadas a cada maceta.

La utilización de `thing_name` como clave principal permitió asociar correctamente la información generada por cada dispositivo físico.

## 5.6 Resultados respecto al cumplimiento de requerimientos

La validación realizada permitió evaluar los **19 requerimientos funcionales** y los **7 requerimientos no funcionales** definidos para el proyecto.

Los resultados obtenidos mostraron cumplimiento total de los requerimientos funcionales relacionados con lectura de humedad, control automático, interacción mediante Alexa, actualización de Device Shadow, recuperación de configuraciones, manejo de múltiples macetas y almacenamiento histórico.

Asimismo, se verificó el cumplimiento de los requerimientos no funcionales relacionados con modularidad, comunicación segura, reconexión automática, seguridad de credenciales y soporte de múltiples dispositivos por usuario.

Como resultado de las pruebas realizadas, el porcentaje de cumplimiento obtenido fue de:

- Requerimientos funcionales: **19 de 19 (100%)**
- Requerimientos no funcionales: **7 de 7 (100%)**

Los resultados obtenidos muestran cumplimiento completo respecto a los requerimientos establecidos para el prototipo desarrollado.

## 5.7 Resultado de integración general

La integración de los distintos componentes permitió establecer comunicación entre dispositivos físicos, servicios AWS y procesamiento mediante Alexa.

Durante las pruebas se verificó el flujo:

**Usuario → Alexa → AWS Lambda → DynamoDB → Device Shadow → ESP32 → MQTT → AWS IoT Core → DynamoDB**

La integración permitió controlar macetas específicas mediante lenguaje natural, sincronizar estados mediante Device Shadow y generar registros históricos asociados a cada dispositivo. Además, se validó el soporte de múltiples macetas por usuario utilizando resolución dinámica basada en `plant_id` y `thing_name`.

# **6. Conclusiones**

1. Se desarrolló exitosamente un sistema de riego inteligente basado en IoT que integra dispositivos físicos, servicios en la nube y procesamiento por voz mediante Alexa.
2. La utilización de AWS IoT Core y Device Shadow permitió desacoplar la lógica del dispositivo físico del procesamiento realizado por el backend, facilitando la sincronización y administración remota de múltiples dispositivos.
3. La arquitectura implementada permitió asociar múltiples macetas a un mismo usuario mediante el uso de `plant_id`, `thing_name` y resolución dinámica desde AWS Lambda.
4. La integración con Alexa mejoró la interacción del usuario, permitiendo controlar, consultar y configurar las macetas mediante lenguaje natural.
5. La utilización de DynamoDB permitió almacenar información histórica relacionada con humedad, eventos de riego, consumo estimado y crecimiento.
6. Las pruebas realizadas permitieron validar todos los requerimientos funcionales y no funcionales definidos para el prototipo, corrigiendo observaciones previas relacionadas con validaciones incompletas.
7. La arquitectura desarrollada permite escalar la solución incorporando nuevos dispositivos ESP32 sin modificar significativamente el backend existente.

# **7. Recomendaciones**

1. Implementar reglas AWS IoT definitivas para almacenamiento automático directo de telemetría y eventos publicados por cada dispositivo.
2. Incrementar el número de dispositivos físicos utilizados durante las pruebas para medir comportamiento con múltiples ESP32 operando simultáneamente.
3. Incorporar pruebas prolongadas para evaluar estabilidad del sistema durante periodos continuos de operación.
4. Implementar métricas automáticas para registrar tiempos de respuesta de Alexa, tiempos de actualización Shadow y latencia MQTT.
5. Agregar sensores complementarios como temperatura, luminosidad y nivel de agua para ampliar las capacidades del sistema.
6. Implementar mecanismos automáticos para registrar nuevas macetas sin intervención manual sobre DynamoDB.
7. Incorporar una interfaz móvil o dashboard web para visualizar el historial almacenado en DynamoDB.

# **8. Anexos**
## ANEXO A. Evidencia del prototipo físico

https://docs.google.com/spreadsheets/d/1WImEL0LnLkhCWSxBCTux21_BuRoeFxdC_QsLRs135mI/edit?usp=sharing

