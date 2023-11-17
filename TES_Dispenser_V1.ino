#include <ESP8266WiFi.h>
#include <HX711.h>

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
        scale.set_scale(-101);
        scale.tare();
    }

    float getWeight() {
        return scale.get_units(10);
    }

private:
    int dtPin;
    int sckPin;
    HX711 scale;
};

// Initialize instances of each class
Ultrasonic ultrasonic(D5, D7, 10);
Pump pump(D6);
LoadCell loadCell(4, 14);

void setup() {
    Serial.begin(9600);

    ultrasonic.setup();
    pump.setup();
    loadCell.setup();
}

void loop() {
    float distance = ultrasonic.getDistance();
    float weight = loadCell.getWeight();

    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.print(" cm, Weight: ");
    Serial.print(weight, 1);
    Serial.println(" grams");

    if (ultrasonic.minRange(distance)) {
        pump.enable();
    } else {
        pump.disable();
    }

    delay(2000);
}
