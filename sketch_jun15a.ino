/*
  Rui Santos & Sara Santos - Random Nerd Tutorials
  Complete project details at https://RandomNerdTutorials.com/esp-now-two-way-communication-esp32/
*/
#include <esp_now.h>
#include <WiFi.h>
#include <ESP32Servo.h>
#include <Wire.h>
#include <Arduino.h>

// REPLACE WITH THE MAC Address of your receiver 
uint8_t broadcastAddress[] = {0xf4, 0x12, 0xfa, 0xdf, 0x43, 0x48};

// Define variables to store incoming readings
int incomingPot1; // Kucuk kanatcik
int incomingPot2; // Buyuk kanatcik
int incomingPot3; // Gaz pedali (ESC)

// --- ANLIK DEGER TAKIBI ICIN EKLENEN DEGISKENLER ---
int anlikAci1 = 180; 
int anlikAci2 = 180; 
int motorStd = 819;  

String success;

// --- EKLENEN SERVO VE ESC PINLERI ---
Servo servo1;
Servo servo2;

const int servo1Pin = D12;
const int servo2Pin = D13;
const int escPin = D14;

// Donanımsal PWM (ledc) ayarları
const int resolution = 14;  
const int PWMfreq = 50;     

// --- PID ALGORITMASI DEGISKENLERI ---
float Kp = 1.0;
float Ki = 0.0;
float Kd = 0.0;
float error = 0, previous_error = 0;
float pid_p = 0, pid_i = 0, pid_d = 0, PID_total = 0;

typedef struct struct_message {
    int pot1; 
    int pot2; 
    int pot3; 
} struct_message;

struct_message incomingReadings;
esp_now_peer_info_t peerInfo;

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));
  incomingPot1 = incomingReadings.pot1;
  incomingPot2 = incomingReadings.pot2;
  incomingPot3 = incomingReadings.pot3; 
}
 
void setup() {
  Serial.begin(115200);

  // --- ESC KURULUMU VE GÜVENLİK ---
  ledcAttach(escPin, PWMfreq, resolution);
  ledcWrite(escPin, 819); 
  delay(2000); 

  servo1.attach(servo1Pin);
  servo2.attach(servo2Pin);

  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
  
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));
}

// PID Hesaplama Fonksiyonu
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
  // --- 1. VE 2. KANATÇIK SERVOSU (ONARILMIŞ MAP VE CONSTRAIN) ---
  anlikAci1 = map(incomingPot1, 10, 800, 0, 180);
  anlikAci1 = constrain(anlikAci1, 0, 180);

  anlikAci2 = map(incomingPot2, 0, 540, 0, 180);
  anlikAci2 = constrain(anlikAci2, 0, 180);
  
  servo1.write(anlikAci1);
  servo2.write(anlikAci2);

  // --- ESC (MOTOR) VE PID KONTROLÜ ---
  // 1. Adım: Kumandadan gelen hedef gücü hesapla (Setpoint)
  float hedefGaz = map(incomingPot3, 0, 4095, 819, 1638);
  hedefGaz = constrain(hedefGaz, 819, 1638);
  
  // 2. Adım: Gerçek sensör okuması (Feedback)
  // NOT: Sensör olmadığı için şimdilik hedefGaza eşitliyoruz (Hata = 0 çıkar)
  float gercekMotorDevri = hedefGaz; 
  
  // 3. Adım: PID Çıkışını Hesapla
  float pidDuzenlemesi = hesaplaPID(hedefGaz, gercekMotorDevri);
  
  // 4. Adım: Ana gaza PID düzeltmesini ekleyip sınırla
  motorStd = hedefGaz + pidDuzenlemesi;
  motorStd = constrain(motorStd, 819, 1638); // ESC'nin aralık dışına çıkıp kilitlenmesini kesinlikle önler

  // Motoru Sür
  ledcWrite(escPin, motorStd);

  updateDisplay();
  delay(15);
}

void updateDisplay(){
  Serial.print(anlikAci1);
  Serial.print(" ");
  Serial.print(anlikAci2);
  Serial.print(" ");
  Serial.println(motorStd);
}