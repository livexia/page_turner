#include <Arduino.h>
#include <BleKeyboard.h>
#include <driver/rtc_io.h>

/*
  Blink with two button
  Button 1 is left
  Button 2 is right
*/

BleKeyboard bleKeyboard;

uint32_t chipId = 0;
int ONBOARD_LED = 2;
int button1Pin = 32;
int button2Pin = 27;

#define BUTTON_PIN_BITMASK 0x108000000 // GPIOs 2 and 15

RTC_DATA_ATTR int bootCount = 0;

void print_wakeup_reason()
{
    esp_sleep_wakeup_cause_t wakeup_reason;

    wakeup_reason = esp_sleep_get_wakeup_cause();

    switch (wakeup_reason)
    {
    case ESP_SLEEP_WAKEUP_EXT0:
        Serial.println("Wakeup caused by external signal using RTC_IO");
        break;
    case ESP_SLEEP_WAKEUP_EXT1:
        Serial.println("Wakeup caused by external signal using RTC_CNTL");
        break;
    case ESP_SLEEP_WAKEUP_TIMER:
        Serial.println("Wakeup caused by timer");
        break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD:
        Serial.println("Wakeup caused by touchpad");
        break;
    case ESP_SLEEP_WAKEUP_ULP:
        Serial.println("Wakeup caused by ULP program");
        break;
    default:
        Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason);
        break;
    }
}

void print_GPIO_wake_up()
{
    uint64_t GPIO_reason = esp_sleep_get_ext1_wakeup_status();
    Serial.print("GPIO that triggered the wake up: GPIO ");
    Serial.println((log(GPIO_reason)) / log(2), 0);
}

void setup()
{
    Serial.begin(115200);

    delay(1000); // Take some time to open up the Serial Monitor

    // Increment boot number and print it every reboot
    ++bootCount;
    Serial.println("Boot number: " + String(bootCount));

    // Print the wakeup reason for ESP32
    print_wakeup_reason();

    // Print the GPIO used to wake up
    print_GPIO_wake_up();

    /*
    First we configure the wake up source
    We set our ESP32 to wake up for an external trigger.
    There are two types for ESP32, ext0 and ext1 .
    ext0 uses RTC_IO to wakeup thus requires RTC peripherals
    to be on while ext1 uses RTC Controller so doesnt need
    peripherals to be powered on.
    Note that using internal pullups/pulldowns also requires
    RTC peripherals to be turned on.
    */
    // esp_deep_sleep_enable_ext0_wakeup(GPIO_NUM_15,1); //1 = High, 0 = Low

    // If you were to use ext1, you would use it like
    // To use internal pullup or pulldown resistors,
    // request the RTC peripherals power domain to be kept on during sleep,
    // and configure pullup/pulldown resistors using rtc_gpio_ functions before entering sleep:
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);
    rtc_gpio_pullup_en(GPIO_NUM_27);
    rtc_gpio_pulldown_dis(GPIO_NUM_27);
    rtc_gpio_pullup_en(GPIO_NUM_32);
    rtc_gpio_pulldown_dis(GPIO_NUM_32);
    // esp_sleep_enable_ext0_wakeup(GPIO_NUM_32, 0); // 1 = High, 0 = Low
    Serial.println(digitalRead(GPIO_NUM_27));
    Serial.println(digitalRead(GPIO_NUM_32));
    esp_sleep_enable_ext1_wakeup(BUTTON_PIN_BITMASK, ESP_EXT1_WAKEUP_ALL_LOW);

    // Go to sleep now
    Serial.println("Going to sleep now");
    delay(1000);
    esp_deep_sleep_start();
    Serial.println("This will never be printed");

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