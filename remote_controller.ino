#include <esp_now.h>
#include <WiFi.h>

const int pot1Pin = A0;
const int pot2Pin = A1;


uint8_t broadcastAddress[] = {0xf4, 0x12, 0xfa, 0xdf, 0xb3, 0x04};

int pot1;
int pot2;

// Define variables to store incoming readings
int incomingPot1;
int incomingPot2;

// Variable to store if sending data was successful
String success;

//Structure example to send data
//Must match the receiver structure
typedef struct msg_send_struct {
    int pot1;
    int pot2;
} msg_send_struct;

msg_send_struct potReadings;

// Create a struct_message to hold incoming sensor readings
//msg_send_struct incomingReadings;

esp_now_peer_info_t peerInfo;

// Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  if (status ==0){
    success = "Delivery Success :)";
  }
  else{
    success = "Delivery Fail :(";
  }
}

// Callback when data is received
/* void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));
  Serial.print("Bytes received: ");
  Serial.println(len);
  incomingPot1 = incomingReadings.pot1;
  incomingPot2 = incomingReadings.pot2;
} */
 
void setup() {
  // Init Serial Monitor
  Serial.begin(115200);
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(esp_now_send_cb_t(OnDataSent));
  
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
  //esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));
}
 
void loop() {
  getReadings();
 
  // Set values to send
  potReadings.pot1 = pot1;
  potReadings.pot2 = pot2;

  // Send message via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &potReadings, sizeof(potReadings));
   
  if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }
  updateDisplay();
  delay(50);
}
void getReadings(){
  pot1 = analogRead(pot1Pin);
  pot2 = analogRead(pot2Pin);
}

void updateDisplay(){
  Serial.println(pot1);
  Serial.println(pot2);
}
