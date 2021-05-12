import paho.mqtt.client as paho
import time
import serial

serdev = '/dev/ttyACM0'                # use the device name you get from `ls /dev/ttyACM*`
s = serial.Serial(serdev, 9600)
# https://os.mbed.com/teams/mqtt/wiki/Using-MQTT#python-client
# MQTT broker hosted on local machine
mqttc = paho.Client()
i = 0
# Settings for connection
# TODO: revise host to your IP
host = "192.168.145.190"
topic = "Mbed"

# Callbacks
def on_connect(self, mosq, obj, rc):
    print("Connected rc: " + str(rc))

def on_message(mosq, obj, msg):
    print("[Received] Topic: " + msg.topic + ", Message: " + str(msg.payload) + "\n")
    if str(msg.payload)[2] ==  "0":
        print("tilt larger than threshold")
        s.write(bytes("/stop/run\r", 'UTF-8'))
    else:
        print(str(msg.payload)[2])
        s.write(bytes("/tilt/run\r", 'UTF-8'))
def on_subscribe(mosq, obj, mid, granted_qos):
    print("Subscribed OK")

def on_unsubscribe(mosq, obj, mid, granted_qos):
    print("Unsubscribed OK")

# Set callbacks
mqttc.on_message = on_message
mqttc.on_connect = on_connect
mqttc.on_subscribe = on_subscribe
mqttc.on_unsubscribe = on_unsubscribe

# Connect and subscribe
print("Connecting to " + host + "/" + topic)
mqttc.connect(host, port=1883, keepalive=60)
mqttc.subscribe(topic, 0)



# Loop forever, receiving messages
mqttc.loop_forever()