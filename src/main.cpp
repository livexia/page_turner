#include <Arduino.h>
#include <BleKeyboard.h>

/*
  Blink with two button
  Button 1 is left
  BUtton 2 is right
*/

BleKeyboard bleKeyboard;

uint32_t chipId = 0;
int ONBOARD_LED = 2;
int button1Pin = 32;
int button2Pin = 27;

void setup()
{
    Serial.begin(115200);

    Serial.println("Starting BLE work!");
    bleKeyboard.begin();

    // put your setup code here, to run once:
    delay(3000);
    pinMode(ONBOARD_LED, OUTPUT);

    pinMode(button1Pin, INPUT_PULLUP);
    pinMode(button2Pin, INPUT_PULLUP);

    for (int i = 0; i < 17; i = i + 8)
    {
        chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
    }
    Serial.printf("ESP32 Chip model = %s Rev %d\n", ESP.getChipModel(), ESP.getChipRevision());
    Serial.printf("This chip has %d cores\n", ESP.getChipCores());
    Serial.print("Chip ID: ");
    Serial.println(chipId);
    delay(300);

    Serial.printf("Init Blink");
    digitalWrite(ONBOARD_LED, HIGH);
    delay(500);
    digitalWrite(ONBOARD_LED, LOW);
}

void loop()
{
    if (bleKeyboard.isConnected())
    {
        int buttonValue = digitalRead(button1Pin);
        if (buttonValue == LOW)
        {
            digitalWrite(ONBOARD_LED, HIGH);
            Serial.println("Button 1 Pushed");
            bleKeyboard.write(KEY_LEFT_ARROW);

            delay(300);
        }
        else
        {
            digitalWrite(ONBOARD_LED, LOW);
        }

        buttonValue = digitalRead(button2Pin);
        if (buttonValue == LOW)
        {
            digitalWrite(ONBOARD_LED, HIGH);
            Serial.println("BUtton 2 Pushed");
            bleKeyboard.write(KEY_RIGHT_ARROW);
            delay(300);
        }
        else
        {
            digitalWrite(ONBOARD_LED, LOW);
        }
    }
}