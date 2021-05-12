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
  const char* host = "192.168.145.190";
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
  t_mode.start(callback(&mode_queue, &EventQueue::dispatch_forever));
  mqtt_thread.start(callback(&mqtt_queue, &EventQueue::dispatch_forever));
  
  btn2.rise(mode_queue.event(&mode));
  //btn3.rise(&close_mqtt);
  mqtt_queue.call_every(500ms,&publish_message, &client);
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