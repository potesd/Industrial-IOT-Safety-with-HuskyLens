#include <HardwareSerial.h>
#include <HuskyLensProtocolCore.h>
#include <HUSKYLENS.h>
#include <HUSKYLENSMindPlus.h>
#include <DFRobot_HuskyLens.h>
//The above libraries have to be edited
//Most of the issues appeared to be usage of the SoftwareSerial library so I just commented those out in the headers.
//DFRobotHuskyLens.h lines 63-69 commented out
//HUSKYLensMindPlus.h lines 63-69 commented out
//
//I removed these library files from the library
//DFMobile.h removed
//DFMobile.cpp removed


#include <WiFi.h>
#include <PubSubClient.h>
//import wifi and pubsub client in order to set up communication


HUSKYLENS huskylens;
#define RXD2 17  //Oops Switched my wires!!
#define TXD2 16  //Oops Switched my wires!!
//define the Huskylens and the tx and rx pins it's attached to. Make sure you have the right pinout


int ID1 = 1;
void printResult(HUSKYLENSResult result);

String stringone;
//create the string we will later fill with the ID#


// Change the credentials below, so your ESP32 connects to your router
const char* ssid = "CRR_2.4GHz";
const char* password = "abbirb6700";
boolean peoples = false;
// Change the variable to your Raspberry Pi IP address, so it connects to your MQTT broker
const char* mqtt_server = "192.168.50.153";
char people[100];
WiFiClient espClient;
PubSubClient client(espClient);

const int lamp = 2;






void setup() {
    Serial.begin(115200);             
    Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);  //Communication with Huskylens
    setup_wifi();
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);
    while (!huskylens.begin(Serial2))
    {
        Serial.println(F("Failbus!"));
        delay(100);
    }
//     huskylens.writeAlgorithm(ALGORITHM_OBJECT_RECOGNITION);
}





// Don't change the function below. This functions connects your ESP8266 to your router
void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("WiFi connected - ESP IP address: ");
  Serial.println(WiFi.localIP());
}


// Not currently using Callback, but good to have for future use
void callback(String topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;

  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  // Feel free to add more if statements to control more GPIOs with MQTT

  // If a message is received on the topic test/onoff, you check if the message is either on or off. Turns the lamp GPIO according to the message
  if (topic == "test/onoff") {
    Serial.print("Message is ");
    Serial.println(messageTemp);
    if (messageTemp != "[]") {
////This is where you put code for people Reaction
////E.G. if messageTempt != "[]" then send stop or pause command to robot.
      digitalWrite(lamp, LOW);
      peoples = true;
      
      Serial.print("There are people");
    }
    if (messageTemp == "[]") {
       digitalWrite(lamp, HIGH);
       peoples = false; 
       Serial.print("There are no people");
    }
  }
  Serial.println();
}


void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    /*
      YOU MIGHT NEED TO CHANGE THIS LINE, IF YOU'RE HAVING PROBLEMS WITH MQTT MULTIPLE CONNECTIONS
      To change the ESP device ID, you will have to give a new name to the ESP8266.
      Here's how it looks:
       if (client.connect("ESP8266Client")) {
      You can do it like this:
       if (client.connect("ESP1_Office")) {
      Then, for the other ESP:
       if (client.connect("ESP2_Garage")) {
      That should solve your MQTT multiple connections problem
    */
    if (client.connect("espClient")) {
      Serial.println("connected");
      // Subscribe or resubscribe to a topic
      // You can subscribe to more topics (to control more LEDs in this example)
      client.subscribe("test/onoff");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}









void loop() {
  //convert the string you declare below into the Char people with a buffer size of 100
     stringone.toCharArray(people, 100);
     if (!client.connected()) {
      reconnect();
    }
    if(!client.loop())
      client.connect("espClient");

    client.publish("test/input", people);
    if (!huskylens.request())    {Serial.println(F("Fail to request data from HUSKYLENS, recheck the connection!"));}
    else if(!huskylens.isLearned()) {Serial.println(F("Nothing learned, press learn button on HUSKYLENS to learn one!"));
    stringone = "Teach me what a Person is";
    }
    else if(!huskylens.available()) {Serial.println(F("No block or arrow appears on the screen!"));
    stringone = "No people";
    }
    
    else
    {
        Serial.println(F("###########"));
        while (huskylens.available())
        {
            HUSKYLENSResult result = huskylens.read();
            printResult(result);
        }    
    }
}

//where you declare what stringone will pass to NodeRed
void printResult(HUSKYLENSResult result){
    if (result.command == COMMAND_RETURN_BLOCK){
        Serial.println(String()+F("Block:xCenter=")+result.xCenter+F(",yCenter=")+result.yCenter+F(",width=")+result.width+F(",height=")+result.height+F(",ID=")+result.ID);
        stringone = String("Warning")+F(",ID=")+result.ID;
    }
    else if (result.command == COMMAND_RETURN_ARROW){
        Serial.println(String()+F("Arrow:xOrigin=")+result.xOrigin+F(",yOrigin=")+result.yOrigin+F(",xTarget=")+result.xTarget+F(",yTarget=")+result.yTarget+F(",ID=")+result.ID);
        
    }
    else{
        Serial.println("Object unknown!");
        stringone = "no people";
    }
}
