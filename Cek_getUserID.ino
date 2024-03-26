#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <HX711.h>

const char *ssid = "HUAWEI-bA32"; 
const char *password = "7GUMCyrB"; 
const char *server = "http://projectdispenser01.000webhostapp.com/disp_get.php"; 
const int port = 80;

const char *serverUrl = "http://projectdispenser01.000webhostapp.com/get_id_user.php";

float calibration_factor = -101.5;
float units;
float ounces;

int berat_awal;
int berat_akhir;
int volume = 0;

class Ultrasonic {
public:
    Ultrasonic(int trigPin, int echoPin, int minDistance) : trigPin(trigPin), echoPin(echoPin), minDistance(minDistance) {}

    void setup() {
        pinMode(trigPin, OUTPUT);
        pinMode(echoPin, INPUT);
    }

    float getDistance() {
        digitalWrite(trigPin, LOW);
        delayMicroseconds(2);
        digitalWrite(trigPin, HIGH);
        delayMicroseconds(10);
        digitalWrite(trigPin, LOW);

        long duration = pulseIn(echoPin, HIGH);
        return duration * 0.034 / 2;
    }

    bool minRange(float distance) {
        return distance <= minDistance;
    }

private:
    int trigPin;
    int echoPin;
    int minDistance;
};

class Pump {
public:
    Pump(int relayPin) : relayPin(relayPin) {}

    void setup() {
        pinMode(relayPin, OUTPUT);
        digitalWrite(relayPin, LOW);
    }

    void enable() {
        digitalWrite(relayPin, HIGH);
    }

    void disable() {
        digitalWrite(relayPin, LOW);
    }

private:
    int relayPin;
};

class LoadCell {
public:
    LoadCell(int dtPin, int sckPin) : dtPin(dtPin), sckPin(sckPin) {}

    void setup() {
        scale.begin(dtPin, sckPin);
        scale.set_scale();
        scale.tare();
    }

    int getWeight_1() {
      scale.set_scale(calibration_factor);

      units = scale.get_units(), 10;
      if (units < 0) {
        units = 0.00;
      }
      ounces = units * 0.035274;

      return units;
    }

    int getWeight_2() {
      scale.set_scale(calibration_factor); 

      units = scale.get_units(), 10;
      if (units < 0) {
        units = 0.00;
      }
      ounces = units * 0.035274;

      return units;
    }

private:
    int dtPin;
    int sckPin;
    HX711 scale;
};

Ultrasonic ultrasonic(D5, D7, 6);
Pump pump(D6);
LoadCell loadCell(4, 14);

void connectWiFi() {
    Serial.print("Connecting to WiFi");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    Serial.println("Connected to WiFi");
}

String get_userid(String disp_id) {
    HTTPClient http;
    WiFiClient wifiClient;
    
    String url = String(serverUrl) + "?disp_id=" + disp_id;

    http.begin(wifiClient, url);

    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        String user_id = payload;
        Serial.println("Status get_user_id"); 
        Serial.println("Payload: " + user_id);
        return user_id;
    } else {
        String payload = http.getString();
        String user_id = payload;
        
        Serial.println("Failed to get user_id");
        Serial.println("HTTP Code: " + String(httpCode));
        Serial.println("Payload: " + payload);
        return "-1";
    }

    http.end();
}

void sendGetRequest(String disp_id, String user_id, int volume) { 
  HTTPClient http;
  WiFiClient wifiClient;

  String url = String(server) + "?disp_id=" + disp_id + "&user_id=" + user_id + "&volume=" + volume;
  http.begin(wifiClient, url);

  int httpCode = http.GET();
  if (httpCode == HTTP_CODE_OK) {
     Serial.println("Success send data");
     }
     else{
      Serial.println("Failed send data");
     }

    http.end();
}

void setup() {
    Serial.begin(115200);
    connectWiFi();
    ultrasonic.setup();
    pump.setup();
    loadCell.setup();
    
}

void loop() {
    float distance = ultrasonic.getDistance();
    String disp_id = "disp1";
    String user_id;
    berat_awal = loadCell.getWeight_1();

    if (ultrasonic.minRange(distance)) {  
      user_id = get_userid(disp_id);
      if (user_id != "No user ID found for disp_id: " + disp_id) {
        
        while (ultrasonic.getDistance() < 6) {
            delay(100);
            Serial.println(distance);
            pump.enable();
            delay(100);
        }
        pump.disable();

        Serial.println("Pump Stopped");
        berat_akhir = loadCell.getWeight_2();
        volume = berat_awal - berat_akhir;
        Serial.println(volume);
        Serial.println(user_id);
        sendGetRequest(disp_id, user_id, volume);

      } else {
        while (ultrasonic.getDistance() < 6) {
            delay(100);
            Serial.println(distance);
            pump.enable();
            delay(100);
        }
        pump.disable();
        Serial.println("Data user id tidak ada");
      }
    }

    delay(500);
}



  

