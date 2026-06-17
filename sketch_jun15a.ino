/*
  Rui Santos & Sara Santos - Random Nerd Tutorials
  Complete project details at https://RandomNerdTutorials.com/esp-now-two-way-communication-esp32/
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*/
#include <esp_now.h>
#include <WiFi.h>
#include <ESP32Servo.h> // Kanatçık servoları için eklendi

//const int pot1Pin = A0;
//const int pot2Pin = A1;

// REPLACE WITH THE MAC Address of your receiver 
uint8_t broadcastAddress[] = {0xf4, 0x12, 0xfa, 0xdf, 0x43, 0x48};

//int pot1;
//int pot2;

// Define variables to store incoming readings
int incomingPot1; //kucuk kanatcık
int incomingPot2; //buyuk kanatcık

// --- ANLIK ACI TAKIBI ICIN EKLENEN DEGISKENLER ---
int anlikAci1 = 180; // Servo 1'in o anki anlık açı değeri
int anlikAci2 = 180; // Servo 2'in o anki anlık açı değeri

// Variable to store if sending data was successful
String success;

// --- EKLENEN SERVO PINLERI ---
Servo servo1;
Servo servo2;

const int servo1Pin = D12; // 1. Kanatçık servo sinyal pini
const int servo2Pin = D13; // 2. Kanatçık servo sinyal pini

// --- PID ALGORITMASI DEGISKENLERI ---
float Kp = 1.0;
float Ki = 0.0;
float Kd = 0.0;
float error = 0, previous_error = 0;
float pid_p = 0, pid_i = 0, pid_d = 0, PID_total = 0;

//Structure example to send data
//Must match the receiver structure
typedef struct struct_message {
    int pot1; // 1. Kanatçık verisi
    int pot2; // 2. Kanatçık verisi
} struct_message;

struct_message potReadings;

// Create a struct_message to hold incoming sensor readings
struct_message incomingReadings;

esp_now_peer_info_t peerInfo;

// Callback when data is sent
/* void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  if (status ==0){
    success = "Delivery Success :)";
  }
  else{
    success = "Delivery Fail :(";
  }
} */

// Callback when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));
  // Serial.print("Bytes received: ");
  // Serial.println(len);
  incomingPot1 = incomingReadings.pot1;
  incomingPot2 = incomingReadings.pot2;
}
 
void setup() {
  // Init Serial Monitor
  Serial.begin(115200);

  // Servolari ilgili dijital pinlere bağla
  servo1.attach(servo1Pin);
  servo2.attach(servo2Pin);

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  //esp_now_register_send_cb(esp_now_send_cb_t(OnDataSent));
  
  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
  // Register for a callback function that will be called when data is received
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));
}

// PID Hesaplama Fonksiyonu (Stabilizasyon için temel omurga)
float hesaplaPID(float setpoint, float current_value) {
  error = setpoint - current_value;
  pid_p = Kp * error;
  pid_i = pid_i + (Ki * error);
  pid_d = Kd * (error - previous_error);
  PID_total = pid_p + pid_i + pid_d;
  previous_error = error;
  
  return PID_total;
}
 
void loop() {
  //getReadings();
 
  // Set values to send
  //potReadings.pot1 = pot1;
  //potReadings.pot2 = pot2;

  // Send message via ESP-NOW
  //esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &potReadings, sizeof(potReadings));
/* if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  } */
  
  // Kumandadan gelen ham verileri anlık açı değerlerine dönüştür ve kaydet
  anlikAci1 = map(incomingPot1, 10, 9, 0, 180);
  anlikAci2 = map(incomingPot2, 0, 540, 0, 180);
  // kucuk kanat 0-540 buyuk kanat 10-800

  // Servoları o an hesaplanan gerçek açıya konumlandır
  servo1.write(anlikAci1);
  servo2.write(anlikAci2);

  updateDisplay();
  delay(15);
}

/* void getReadings(){
  pot1 = analogRead(pot1Pin);
  pot2 = analogRead(pot2Pin);
} */

void updateDisplay(){
  // Seri Port Çizici (Serial Plotter) formatına uygun olarak sadece saf sayısal verileri basıyoruz
  Serial.print(anlikAci1);
  Serial.print(" ");
  Serial.println(anlikAci2);
}