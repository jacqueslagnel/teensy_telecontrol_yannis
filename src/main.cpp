#include <Arduino.h>
#include <RF24.h>
#include <SPI.h>
#include <nRF24L01.h>

// A3=Throtle, Pitch=A2 et Roll=A1
#define THROTLE_PIN A3
#define PITCH_PIN A2
#define ROLL_PIN A1
/*
connection Teensy 4 a NRF24L01
GND  -> GND
VCC  -> 3.3V (mettre une capa de 10uF)
CE   -> 31 select RX/TX mode (non lie a SPI donc peut etre changee)
CSN  -> 10 chip select SPI
SCK  -> 13
MOSI -> 11
MISO -> 12
IRQ  -> 32  (non lie a SPI donc peut etre changee)
*/

RF24 radio(31, 10); // CE, CSN
const byte address[6] = "00001";
volatile bool messageAvailable = false;

void NRF24L01_IRQ();

struct Data {
    int throtle;
    int roll;
    int pitch;
    int yaw;
};

Data dataToSend = { 0, 0, 0, 0 };
Data dataReceived;

void setup()
{
    Serial.begin(115200);
    pinMode(32, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(32), NRF24L01_IRQ, FALLING);
    Serial.println("checking SPI pins....");
    dataToSend.throtle = analogRead(THROTLE_PIN);
    dataToSend.pitch = analogRead(PITCH_PIN);
    dataToSend.roll = analogRead(ROLL_PIN);

    radio.begin();
    radio.setChannel(52);
    radio.setPALevel(RF24_PA_MIN);
    radio.setDataRate(RF24_250KBPS);
    radio.maskIRQ(1, 1, 0); // enable only IRQ on RX event

    radio.openWritingPipe(address);
    radio.openReadingPipe(1, address);

    radio.startListening();
    Serial.println(radio.available());
    Serial.println(radio.getPALevel());
    Serial.println(radio.isChipConnected());
    radio.printDetails();

    delay(1);
}

void loop()
{
    if (messageAvailable) {
        messageAvailable = false;
        if (radio.available()) {
            radio.read(&dataReceived, sizeof(Data));
            Serial.print("RECIEVED TEENSY\tthrotle: ");
            Serial.print(dataReceived.throtle);
            Serial.print("\tpitch: ");
            Serial.print(dataReceived.pitch);
            Serial.print("\troll: ");
            Serial.print(dataReceived.roll);
            Serial.print("\tyaw: ");
            Serial.println(dataReceived.yaw);
        }
    }

    // *********** to send *******************
    radio.stopListening();
    delay(5);

    dataToSend.throtle = analogRead(THROTLE_PIN);
    dataToSend.pitch = analogRead(PITCH_PIN);
    dataToSend.roll = analogRead(ROLL_PIN);

    dataToSend.throtle = map(dataToSend.throtle, 0, 1023, 1000, 2000);
    dataToSend.pitch = map(dataToSend.pitch, 0, 1023, -200, 200); // on va dire par exmple de -20 degres a + 20 degres
    dataToSend.roll = map(dataToSend.roll, 0, 1023, -200, 200); // on va dire par exmple de -20 degres a + 20 degres

    radio.write(&dataToSend, sizeof(Data));

    Serial.print("Throtle: ");
    Serial.print(dataToSend.throtle);
    Serial.print("\tPitch: ");
    Serial.print(dataToSend.pitch);
    Serial.print("\tRoll: ");
    Serial.println(dataToSend.roll);

    delay(1);
    radio.startListening();
    delay(50);
}
// delay(1);
//  Additional code for other tasks

void NRF24L01_IRQ()
{
    messageAvailable = true;
}
