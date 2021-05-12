#include "mbed.h"
#include "mbed_events.h"
#include "MQTTNetwork.h"
#include "MQTTmbed.h"
#include "MQTTClient.h"
#include "stm32l475e_iot01_accelero.h"
#include "accelerometer_handler.h"
#include "fig.h"
#include "magic_wand_model_data.h"

#include "tensorflow/lite/c/common.h"
#include "tensorflow/lite/micro/kernels/micro_ops.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"
#include "mbed_rpc.h"
#include "uLCD_4DGL.h"
#include <math.h>  
#define PI 3.14159265
void gesture();
void gesture_UI();
int PredictGesture(float* output) ;
void acc(Arguments *in, Reply *out);
Thread t_g;
EventQueue queue_g(32 * EVENTS_EVENT_SIZE);
EventQueue mqtt_queue;
Thread mqtt_thread(osPriorityHigh);
InterruptIn btnRecord(USER_BUTTON);
EventQueue queue(32 * EVENTS_EVENT_SIZE);
BufferedSerial pc(USBTX, USBRX);
int data[10] ;
void printdata(Arguments *in, Reply *out)
{
    for(int i = 0 ; i < 10 ; i++)
        printf("%d",data[i]);
}
//RPCFunction rpcm(&stop, "stop");
Thread t;
bool p = false;
int16_t pData[3] = {0};
int idR[32] = {0};
int indexR = 0;
int times = 0;
RPCFunction rpc_acc(&acc, "acc");
int gesture_ID = 0;

volatile bool closed = false;
constexpr int kTensorArenaSize = 60 * 1024;
const char* topic = "Mbed";
volatile int message_num = 0;
volatile int arrivedcount = 0;
WiFiInterface *wifi;
bool mode_acc = true;

void messageArrived(MQTT::MessageData& md) {
    MQTT::Message &message = md.message;
    char msg[300];
    sprintf(msg, "Message arrived: QoS%d, retained %d, dup %d, packetID %d\r\n", message.qos, message.retained, message.dup, message.id);
    printf(msg);
    ThisThread::sleep_for(1000ms);
    char payload[300];
    sprintf(payload, "Payload %.*s\r\n", message.payloadlen, (char*)message.payload);
    printf(payload);
    ++arrivedcount;
}

void publish_message(MQTT::Client<MQTTNetwork, Countdown>* client) {
    if(p == true)
    {
      MQTT::Message message;
      char buff[100];
      sprintf(buff, "%d", gesture_ID);
      message.qos = MQTT::QOS0;
      message.retained = false;
      message.dup = false;
      message.payload = (void*) buff;
      message.payloadlen = strlen(buff) + 1;
      int rc = client->publish(topic, message);
  
      printf("rc:  %d\r\n", rc);
      printf("Puslish message: %s\r\n", buff);
      p = false;
    }
}

void close_mqtt() {
    closed = true;
}

void record(void) {
   BSP_ACCELERO_AccGetXYZ(pData);
   printf("%d, %d, %d\n", pData[0], pData[1], pData[2]);
   double deg = (double)pData[2]/sqrt(pData[0]*pData[0]+pData[1]*pData[1]+pData[2]*pData[2]);
    if(deg < cos(30* PI/ 180.0))
        times++;
    else
        times = 0;
    if(times >= 10)
    {
        data[gesture_ID]++;
        times = 0;
    }
}

void startRecord(void) {
   printf("---start---\n");
   times = 0;
   data[gesture_ID] = 0;
   idR[indexR++] = queue.call_every(1ms, record);
   indexR = indexR % 32;
}

void stopRecord(void) {
    printf("case of change direction %d\n", data[gesture_ID]);
    printf("---stop---\n");
    p = true;
    ThisThread::sleep_for(5ms);
    gesture_ID++;
    for (auto &i : idR)
      queue.cancel(i);
}
void acc(Arguments *in, Reply *out)
{
   
   BSP_ACCELERO_Init();
   printf("Start accelerometer init\n");
   t.start(callback(&queue, &EventQueue::dispatch_forever));
   btnRecord.fall(queue.event(startRecord));
   btnRecord.rise(queue.event(stopRecord));
}









int main() {
  char buf[256], outbuf[256];
  FILE *devin = fdopen(&pc, "r");
  FILE *devout = fdopen(&pc, "w");
  
  wifi = WiFiInterface::get_default_instance();
  if (!wifi) {
          printf("ERROR: No WiFiInterface found.\r\n");
  }
  printf("\nConnecting to %s...\r\n", MBED_CONF_APP_WIFI_SSID);
  int ret = wifi->connect(MBED_CONF_APP_WIFI_SSID, MBED_CONF_APP_WIFI_PASSWORD, NSAPI_SECURITY_WPA_WPA2);
  if (ret != 0) {
          printf("\nConnection error: %d\r\n", ret);
  }
  NetworkInterface* net = wifi;
  MQTTNetwork mqttNetwork(net);
  MQTT::Client<MQTTNetwork, Countdown> client(mqttNetwork);
  //TODO: revise host to your IP
  const char* host = "192.168.195.230";
  printf("Connecting to TCP network...\r\n");

  SocketAddress sockAddr;
  sockAddr.set_ip_address(host);
  sockAddr.set_port(1883);
  printf("address is %s/%d\r\n", (sockAddr.get_ip_address() ? sockAddr.get_ip_address() : "None"),  (sockAddr.get_port() ? sockAddr.get_port() : 0) ); //check settin
  int rc = mqttNetwork.connect(sockAddr);//(host, 1883);
  if (rc != 0) {
          printf("Connection error.");
  }
  printf("Successfully connected!\r\n");
  MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
  data.MQTTVersion = 3;
  data.clientID.cstring = "Mbed";
  if ((rc = client.connect(data)) != 0){
          printf("Fail to connect MQTT\r\n");
  }
  if (client.subscribe(topic, MQTT::QOS0, messageArrived) != 0){
          printf("Fail to subscribe\r\n");
  }  
  
    mqtt_thread.start(callback(&mqtt_queue, &EventQueue::dispatch_forever));
  
    mqtt_queue.call_every(5ms,&publish_message, &client);
  
  
  while(1) {
    memset(buf, 0, 256);      // clear buffer
    for(int i=0; ; i++) {
      char recv = fgetc(devin);
      if (recv == '\n') {
        printf("\r\n");
        break;
      }
      buf[i] = fputc(recv, devout);
    }
    //Call the static call method on the RPC class
    RPC::call(buf, outbuf);
    printf("%s\r\n", outbuf);
  }
}