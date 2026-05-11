import threading
import time
from contextlib import asynccontextmanager

import paho.mqtt.client as mqtt
from fastapi import FastAPI
from fastmcp import FastMCP
import uvicorn


# ============================================================
# AWS IOT MQTT CONFIGURATION
# ============================================================

# Replace with your AWS IoT endpoint
AWS_IOT_ENDPOINT = "PUT_YOUR_AWS_ENDPOINT_HERE"

# Example:
# AWS_IOT_ENDPOINT = "abcdefgh123456-ats.iot.us-east-1.amazonaws.com"

MQTT_PORT = 8883

# Replace with your Thing client ID
CLIENT_ID = "PUT_YOUR_CLIENT_ID_HERE"

# MQTT Topics
TOPIC_COUNTER = "ucb/test/topic/counter"
TOPIC_LED = "ucb/test/topic/led"

# Certificates paths
ROOT_CA_PATH = "PUT_PATH_TO_AmazonRootCA1.pem"
CERTIFICATE_PATH = "PUT_PATH_TO_DEVICE_CERTIFICATE.pem.crt"
PRIVATE_KEY_PATH = "PUT_PATH_TO_PRIVATE.pem.key"


# ============================================================
# GLOBAL VARIABLE
# ============================================================

last_counter_value = "No message received yet"


# ============================================================
# MQTT CLIENT
# ============================================================

mqtt_client = mqtt.Client(
    client_id=CLIENT_ID,
    protocol=mqtt.MQTTv311
)

# Configure TLS Certificates
mqtt_client.tls_set(
    ca_certs=ROOT_CA_PATH,
    certfile=CERTIFICATE_PATH,
    keyfile=PRIVATE_KEY_PATH
)


def on_connect(client, userdata, flags, rc):
    """
    Called when connected to AWS IoT.
    """

    if rc == 0:
        print("Connected to AWS IoT Core")

        client.subscribe(TOPIC_COUNTER)
        print(f"Subscribed to topic: {TOPIC_COUNTER}")

    else:
        print(f"Connection failed with code: {rc}")


def on_message(client, userdata, msg):
    """
    Called when a message is received.
    """

    global last_counter_value

    payload = msg.payload.decode()

    print(f"[MQTT] {msg.topic}: {payload}")

    if msg.topic == TOPIC_COUNTER:
        last_counter_value = payload


mqtt_client.on_connect = on_connect
mqtt_client.on_message = on_message


def start_mqtt():
    """
    Keeps MQTT running forever.
    Reconnects automatically if disconnected.
    """

    while True:
        try:
            print("Connecting to AWS IoT Core...")

            mqtt_client.connect(
                AWS_IOT_ENDPOINT,
                MQTT_PORT,
                keepalive=60
            )

            mqtt_client.loop_forever()

        except Exception as e:
            print(f"MQTT Error: {e}")
            print("Retrying in 5 seconds...")
            time.sleep(5)


# ============================================================
# MCP SERVER
# ============================================================

mcp = FastMCP("AWS_IOT_LED_CONTROLLER")


@mcp.tool()
def turn_light_on():
    """
    Publish ON to LED topic.
    """

    mqtt_client.publish(
        TOPIC_LED,
        payload="ON",
        qos=1
    )

    return "Light turned ON"


@mcp.tool()
def turn_light_off():
    """
    Publish OFF to LED topic.
    """

    mqtt_client.publish(
        TOPIC_LED,
        payload="OFF",
        qos=1
    )

    return "Light turned OFF"


@mcp.tool()
def get_counter_value():
    """
    Returns the latest counter value
    received from MQTT.
    """

    return last_counter_value


# ============================================================
# FASTAPI APP
# ============================================================

@asynccontextmanager
async def lifespan(app: FastAPI):

    mqtt_thread = threading.Thread(
        target=start_mqtt,
        daemon=True
    )

    mqtt_thread.start()

    print("MQTT background thread started")

    yield

    print("Server shutting down...")


app = FastAPI(
    title="AWS IoT MCP Server",
    lifespan=lifespan
)

# Mount MCP endpoint
app.mount("/mcp", mcp.http_app())


# ============================================================
# MAIN
# ============================================================

if __name__ == "__main__":

    uvicorn.run(
        app,
        host="0.0.0.0",
        port=8000
    )