#include <Arduino.h>
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <SparkFunLSM6DSO.h>
#include <Wire.h>
#include <cmath>
#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define LED_PIN 33

LSM6DSO myIMU; 

bool calibration_phase = true;
const long calibration_length = 10000; // 10 secs calibration

float currAccelX = 0; float currAccelY = 0; //Current X and Y accelerations
float step_threshold = 0; //Threshold of acceleration neeeded to cross before we count a step

int stepCount = 0; //sum of steps we've taken

float minAccelX = 1000; float maxAccelX = -1000;
float minAccelY = 1000; float maxAccelY = -1000;

float minMagnitude = 0;
float maxMagnitude = 0;
float currMagnitude = 0;

std::vector<float> magnitudeData;

// BLECharacteristic *pCharacteristic; // Pointer to the BLE characteristic
class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
        std::string value = pCharacteristic->getValue();
        if (value.length() > 0) {
            Serial.println("*********");
            Serial.print("New value: ");
            for (int i = 0; i < value.length(); i++)
                Serial.print(value[i]);
            Serial.println();
            Serial.println("*********");
        }

        if (value == "on"){
            digitalWrite(LED_PIN, HIGH);
            Serial.println("LED is now on");
        }
        if (value == "off"){
            digitalWrite(LED_PIN, LOW);
            Serial.println("LED is now off");
        }
        if (value == "steps"){
            pCharacteristic->setValue("step count: " + std::to_string(stepCount));
            pCharacteristic->notify();
        }
    }
};


float getMagnitude(float accelX, float accelY){
    return sqrt(accelX *accelX + accelY*accelY);
}

float getAvg(std::vector<float> & data){
    float sum = 0;
    for (int i = 0; i < data.size(); i++) {
        sum += data[i];
    }
    return sum / data.size();
}

void setup(){
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);
    delay(500); 

    Wire.begin();
    delay(10);
    if( myIMU.begin() )
        Serial.println("Ready.");
    else { 
        Serial.println("Could not connect to IMU.");
        Serial.println("Freezing");
    }

    if( myIMU.initialize(BASIC_SETTINGS) )
        Serial.println("Loaded Settings.");

    // Initialize BLE
    BLEDevice::init("MyESP32");
    BLEServer *pServer = BLEDevice::createServer();
    BLEService *pService = pServer->createService(SERVICE_UUID);
    BLECharacteristic *pCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ |
    BLECharacteristic::PROPERTY_WRITE
    );
    pCharacteristic->setCallbacks(new MyCallbacks());
    pCharacteristic->setValue("Hello World");
    pService->start();
    BLEAdvertising *pAdvertising = pServer->getAdvertising();
    pAdvertising->start();
    Serial.println("BLE Setup Complete");

}

void loop(){
    unsigned long currTime = millis();
    if (calibration_phase){
        if((currTime / 500)%2 == 0){
            digitalWrite(LED_PIN, HIGH);
        }else{
            digitalWrite(LED_PIN, LOW);
        }

        if(currTime > calibration_length){
            calibration_phase = false;
            Serial.println("max X:");
            Serial.println(maxAccelX, 3);
            Serial.println("min X:");
            Serial.println(minAccelX,3);

            Serial.println("max Y:");
            Serial.println(maxAccelY,3);
            Serial.println("min Y:");
            Serial.println(minAccelY,3);

            Serial.println("Calibration Complete");

            minMagnitude = getMagnitude(minAccelX, minAccelY);
            maxMagnitude = getMagnitude(maxAccelX, maxAccelY);

            step_threshold = (maxMagnitude + minMagnitude)/2;

        }

        currAccelX = myIMU.readFloatAccelX(); //reading in X, Y accelerations in terms of how they relate to 9.8g
        currAccelY = myIMU.readFloatAccelY();

        if(currAccelX > maxAccelX){
            maxAccelX = currAccelX;
        }
        if(currAccelX < minAccelX){
            minAccelX = currAccelX;
        }

        if(currAccelY > maxAccelY){
            maxAccelY = currAccelY;
        }
        if(currAccelY < minAccelY){
            minAccelY = currAccelY;
        }


    }
    else{
        currAccelX = myIMU.readFloatAccelX();
        currAccelY = myIMU.readFloatAccelY();

        currMagnitude = getMagnitude(currAccelX, currAccelY);

        magnitudeData.push_back(currMagnitude);

        if (magnitudeData.size() >= 25){
            float avgMag = getAvg(magnitudeData);
            if(avgMag >= step_threshold){
                stepCount++;
                Serial.println("1 step taken, total steps:");
                Serial.println(stepCount);
            }

            magnitudeData.clear();
        }

        

    }
   delay(20);    
}



/*
Notes:

- reset the default lab environment to Default (Lab 3) at the bottom of the screen
- Check platformIO's Devices tab under esp32dev/General
- set the cmd line to be in Lab 3 rather than Lab 0

*/

/*
Part 1:
#include <Arduino.h>
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define LED_PIN 33

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
        std::string value = pCharacteristic->getValue();
        if (value.length() > 0) {
            Serial.println("*********");
            Serial.print("New value: ");
            for (int i = 0; i < value.length(); i++)
                Serial.print(value[i]);
            Serial.println();
            Serial.println("*********");
        }

        if (value == "on"){
            digitalWrite(LED_PIN, HIGH);
            Serial.println("LED is now on");
        }
        if (value == "off"){
            digitalWrite(LED_PIN, LOW);
            Serial.println("LED is now off");
        }
    }
};
void setup() {
    Serial.begin(9600);

    pinMode(LED_PIN, OUTPUT);
    Serial.println("1- Download and install an BLE scanner app in your phone");
    Serial.println("2- Scan for BLE devices in the app");
    Serial.println("3- Connect to MyESP32");
    Serial.println("4- Go to CUSTOM CHARACTERISTIC in CUSTOM SERVICE and write something");
    Serial.println("5- See the magic =)");
    BLEDevice::init("MyESP32");
    BLEServer *pServer = BLEDevice::createServer();
    BLEService *pService = pServer->createService(SERVICE_UUID);
    BLECharacteristic *pCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ |
    BLECharacteristic::PROPERTY_WRITE
    );
    pCharacteristic->setCallbacks(new MyCallbacks());
    pCharacteristic->setValue("Hello World");
    pService->start();
    BLEAdvertising *pAdvertising = pServer->getAdvertising();
    pAdvertising->start();
}
void loop() {
delay(2000);
}
*/