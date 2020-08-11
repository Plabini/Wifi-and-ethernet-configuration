/** @file code.c
 *  @brief MQTT Client that Subscribes to 2 different topics "config/wifi" & "config/eth"
 * and configures Wifi and Ethernet when commands are received on these MQTT topics.
 * 
 *  @author Plabini Jibanjyoti Nayak 
 *  @bug No bugs found 
 */

/**-------------------------------------------------------------------------------
                        ## Function Description ## 
*--------------------------------------------------------------------------------
* get()- This function saves attributes for publishing.
* onMqttMessage(): To receive a message, print out the topic and contents.
* connect_to_Ethernet()- This function attempt to connect Ethernet.
* connect_to_WiFi () - This function attempts to connect wifi. 
* main() - Main function.
*-----------------------------------------------------------------------------------
*/

/*
 *#####################################################################
 *  Initialization block
 *#####################################################################
 */

/* --- Standard Includes --- */
#include "MQTTClient.h"
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>

#define Server "test.mosquitto.org"

char SSID[100]; //Network SSID
char Password[100]; //Network Password
char Address[100]; //Network Address
char NetMask[100]; //Network Netmask
char Gateway[100]; //Network Gateway

MQTTClient mqtt_Client;

/*
 *#####################################################################
 *  Process block
 *#####################################################################
 */

void get(MQTTClient, char*, char*, int , int);//function definition of get()
int connect_to_Ethernet(char *);//funtion definition of connect_to_Ethernet()
void connect_to_WiFi(char *);//function definition of connect_to_Wifi()
/**
 *
 *  @brief: get()- This function saves attributes for publishing.
 * 
 */
void get(MQTTClient mqtt_Client, char *payload, char *Topic, int QOS, int retain){
    MQTTClient_message publish = MQTTClient_message_initializer;
    publish.payload = payload;
    publish.payloadlen = strlen(publish.payload);
    publish.qos = QOS;
    publish.retained = retain;
    MQTTClient_deliveryToken token;
    MQTTClient_publishMessage(mqtt_Client, Topic, &publish, &token);
    MQTTClient_waitForCompletion(mqtt_Client, token, 100L);
}
/** 
 *  @brief onMqttMessage(): To receive a message, print out the topic and contents.
 *  
 */

int onMqttMessage(void *context, char *inTopic, int length_topic, MQTTClient_message *message) {

  char *payload= message->payload;
  printf("Received command%s\n", payload); //displays the command

  int i = 0, count1=0;;  // i: loop interator. count1: to check if the payload command is for ethernet or wifi.

  char temp[50];
  if(!strcmp(payload,"quit")) //checks if the command is quit. If true, exits program.
    {
      MQTTClient_disconnect(&mqtt_Client, 100);
      MQTTClient_destroy(&mqtt_Client);
      exit(-1) ;
    }

  else{
    count1 = connect_to_Ethernet(payload); //connect_to_Ethernet() called. count1 checks is the payload is for ethernet.
    if(count1==0){
      connect_to_WiFi(payload); //connect_to_WiFi() is called if count1 values does not change, i.e, payload command was not for ethernet.
    }
  }

  MQTTClient_freeMessage(&message);
  MQTTClient_free(inTopic);
  return 1;
}

/** 
 *  @brief Attempt to connect Ethernet.
 * 
 */ 
 int connect_to_Ethernet(char *payload){
   int flag=0; 
    if(!strcmp(payload,"ethernet on")){
      
      system("sudo dhcpd eth0") ; //Ethernet switched on
      flag=1;
    }

  else if(!strcmp(payload,"ethernet off")){
        system("sudo ifdown eth0") ; //Ethernet switched off
        flag=1;
    }

  else if(!strcmp(payload, "ethernet connect dhcp"))
    {
      system("# The primary network interface -- use DHCP to find our address | sudo tee /etc/network/interfaces") ;
      system("echo auto eth0 | sudo tee -a /etc/network/interfaces") ;
      system("iface eth0 inet dhcp | sudo tee -a /etc/network/interfaces") ;
      system("sudo /etc/init.d/networking restart") ;
      flag=1;
    }
    return flag;//returns 1 if payload code is for Ethernet.
  }
  
/** 
 *  @brief Attempt to connect to Wifi.
 */ 

  void connect_to_WiFi(char *payload){
    int count=0; //to check if wifi is connected or not.
    int i,n;
    char store[500]; 
    if(!strcmp(payload, "wifi on")){
      //Following commands are used to switch on wifi, scan and display all the networks.
      system("iwconfig") ;
      system("sudo ifconfig wlp2s0 up") ; 
      system("sudo iwlist wlp2s0 scan | grep ESSID") ; 
  }

  else if(!strcmp(payload,"wifi off"))
    {
      //Following commands are used switch off the wifi.
      system("sudo ifconfig wlp2s0 down") ; 
      system("sudo iwlist wlp2s0 scan | grep ESSID") ;

    }
      
      if(!strcmp(payload,"wifi connect")) //statement to check if wifi is connected. If yes, count becomes 1.
        count = 1 ; 
      
      if(count==1) 
      {
        //In this block SSID and Password is saved to store, separated by a space. In payload, the SSID can be found after 13 characters, after that followed by a space Password.
        for (i=13;payload[i]!=' ';i++){ 
          SSID[i-13] = payload[i] ;
        }
        SSID[i-13]='\0' ;

        for(i=13+strlen(SSID)+1;i<strlen(payload);i++){ 
          Password[i-13-strlen(SSID)-1] = payload[i] ;
        }
        Password[i-13-strlen(SSID)-1]='\0';

       //Following set of commands are used to connect wifi using the newly created .conf file which has SSID and password.
        system("sudo killall wpa_supplicant") ; //To kill any wpa_supplicant processes that may disrupt execution
        system("iwconfig") ;
        system("sudo ifconfig wlp2s0 up") ; 
        system("sudo iwlist wlp2s0 scan | grep ESSID") ;
        sprintf(store,"wpa_passphrase %s %s | sudo tee /etc/wpa_supplicant.conf",SSID,Password) ; //SSID and Password stored in a string
        system(store) ; //Making store configurable
        system("sudo wpa_supplicant -B -c /etc/wpa_supplicant.conf -i wlp2s0") ; //connect wifi
        sleep(5);//Delay
        system("iwconfig") ;
      }

      else{
        //In this block Address, Netmask and Gateway is saved. In payload, the Address can be found after 24 characters, after that followed by a space Netmask and Gateway.
        for (i = 24 ; payload[i]!=' ' ; i++)
        {
          Address[i-24] = payload[i] ;
        }

        Address[i-24] = '\0' ;
        n = i+1 ;
        for (i = n ; payload[i] != ' ' ; i++)
        {
          NetMask[i-n] = payload[i] ;
        }
        NetMask[i-n] = '\0' ;
        n = i+1 ;
        for ( i = n ; i < strlen(payload) ; i++ )
        {
          Gateway[i-n] = payload[i] ;
        }
        Gateway[i-n] = '\0' ;
        system("echo auto eth0 | sudo tee /etc/network/interfaces") ;
        system("echo iface eth0 inet static | sudo tee -a /etc/network/interfaces") ;
        char Address_Copy[100] ;
        char NetMask_Copy[100] ;
        char Gateway_Copy[100] ;
        sprintf(Address_Copy,"echo address %s | sudo tee -a /etc/network/interfaces",Address) ;
        system(Address_Copy) ;//Making string configurable
        sprintf(NetMask_Copy,"echo netmask %s | sudo tee -a /etc/network/interfaces",NetMask) ;
        system(NetMask_Copy) ;//Making string configurable
        sprintf(Gateway_Copy,"echo gateway %s | sudo tee -a /etc/network/interfaces",Gateway) ;
        system(Gateway_Copy) ;//Making string configurable
        system("sudo /etc/init.d/networking restart") ;
      }

    }
  
/** 
 *  @brief Main function.
 */


void main(){
    MQTTClient_create(&mqtt_Client, Server, "ubuntu_client", MQTTCLIENT_PERSISTENCE_NONE, NULL);
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;

    MQTTClient_setCallbacks(mqtt_Client, NULL, NULL, onMqttMessage, NULL);
    
   
    if ( MQTTClient_connect(mqtt_Client, &conn_opts) != MQTTCLIENT_SUCCESS) {
        printf("Connection Failed");
        exit(-1);
    }
    else
    {
      printf("Connection Succesful\n");
      printf("Enter a command\n") ;
    }

    get(mqtt_Client,"ethenet on", "Message", 0, 0);
    MQTTClient_subscribe(mqtt_Client, "Message", 0);
    
    while(1) {}

}
