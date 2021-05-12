import paho.mqtt.client as paho
import time
import serial

serdev = '/dev/ttyACM0'                # use the device name you get from `ls /dev/ttyACM*`
s = serial.Serial(serdev, 9600)
# https://os.mbed.com/teams/mqtt/wiki/Using-MQTT#python-client
# MQTT broker hosted on local machine
mqttc = paho.Client()
i = 0
t = np.arange(0,10,1)
# Settings for connection
# TODO: revise host to your IP
host = "192.168.195.230"
topic = "Mbed"

# Callbacks
def on_connect(self, mosq, obj, rc):
    print("Connected rc: " + str(rc))

def on_message(mosq, obj, msg):
    print("[Received] Topic: " + msg.topic + ", Message: " + str(msg.payload) + "\n")
    if str(msg.payload)[2] ==  "1" and str(msg.payload)[2] ==  "0":
        s.write(bytes("/printdata/run\r", 'UTF-8'))
        
        for x in range(0, 10):
            line=s.readline() # Read an echo string from B_L4S5I_IOT01A terminated with '\n'
            # print line
            y[x] = float(line)

        Y = y # fft computing and normalization
        Y = Y[range(10)] # remove the conjugate frequency parts
        fig, ax = plt.subplots(2, 1)
        ax[0].plot(t,y)
        ax[0].set_xlabel('gesture_ID')
        ax[0].set_ylabel('times')
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