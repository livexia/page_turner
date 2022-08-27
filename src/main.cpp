#include <Arduino.h>
#include <BleKeyboard.h>
#include <WiFiManager.h> //https://github.com/tzapu/WiFiManager WiFi Configuration Magic
#include <driver/rtc_io.h>
#include <driver/timer.h>

/*
  Blink with two button
  Button 1 is left
  Button 2 is right
*/

BleKeyboard bleKeyboard;
WiFiManager wifiManager;

uint32_t chipId = 0;
int ONBOARD_LED = 2;
int button1Pin = 32;
int button2Pin = 27;
hw_timer_t *timer = NULL;
uint64_t last_time_pressed;

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

    // deep sleep

    // If you were to use ext1, you would use it like
    // To use internal pullup or pulldown resistors,
    // request the RTC peripherals power domain to be kept on during sleep,
    // and configure pullup/pulldown resistors using rtc_gpio_ functions before entering sleep:
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);
    rtc_gpio_pullup_en(GPIO_NUM_27);
    rtc_gpio_pulldown_dis(GPIO_NUM_27);
    rtc_gpio_pullup_en(GPIO_NUM_32);
    rtc_gpio_pulldown_dis(GPIO_NUM_32);

    esp_sleep_enable_ext1_wakeup(BUTTON_PIN_BITMASK, ESP_EXT1_WAKEUP_ALL_LOW);

    // light sleep
    gpio_wakeup_enable(GPIO_NUM_27, GPIO_INTR_LOW_LEVEL);
    gpio_wakeup_enable(GPIO_NUM_32, GPIO_INTR_LOW_LEVEL);
    esp_sleep_enable_gpio_wakeup();

    Serial.println("Starting BLE work!");
    rtc_gpio_deinit(GPIO_NUM_27);
    rtc_gpio_deinit(GPIO_NUM_32);
    bleKeyboard.begin();

    // disable for now
    // wifiManager.autoConnect();

    // put your setup code here, to run once:
    delay(300);
    pinMode(ONBOARD_LED, OUTPUT);

    pinMode(button1Pin, INPUT_PULLUP);
    pinMode(button2Pin, INPUT_PULLUP);

    for (int i = 0; i < 17; i = i + 8)
    {
        chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
    }
    Serial.printf("ESP32 Chip model = %s Rev %d\n", ESP.getChipModel(), ESP.getChipRevision());
    Serial.printf("This chip has %d cores\n", ESP.getChipCores());
    Serial.printf("Chip ID: %d\n", chipId);

    // setup timer
    timer = timerBegin(0, 80, true);
    last_time_pressed = timerRead(timer);
}

void press_button(int buttonPin, uint8_t KEY)
{
    int buttonValue = digitalRead(buttonPin);
    int pressed_time = 0;
    while (buttonValue == LOW)
    {
        buttonValue = digitalRead(buttonPin);
        digitalWrite(ONBOARD_LED, HIGH);
        last_time_pressed = timerRead(timer);
        pressed_time++;
    }
    if (pressed_time)
    {
        digitalWrite(ONBOARD_LED, LOW);
        bleKeyboard.write(KEY);
        Serial.printf("Pin %d on HIGH, key id: %d, pressed time: %d\n", buttonPin, KEY, pressed_time);
    }
}

void loop()
{
    if (bleKeyboard.isConnected())
    {
        if (timerRead(timer) - last_time_pressed > 60 * 1000000)
        {
            // after 10s no press light sleep
            Serial.println("Beacuse there is no press after 10s, going to light sleep now");
            delay(300);
            esp_light_sleep_start();
        }
        else if (timerRead(timer) - last_time_pressed > 300 * 1000000)
        {
            // after 300s no press deep sleep, maybe need a timer to weakup, and put it into deep sleep
            Serial.println("Beacuse there is no press after 300s, going to deep sleep now");
            delay(300);
            esp_deep_sleep_start();
        }
        press_button(button1Pin, KEY_LEFT_ARROW);
        press_button(button2Pin, KEY_RIGHT_ARROW);
    }
    else
    {
        if (timerRead(timer) - last_time_pressed > 300 * 1000000)
        {
            Serial.println("Beacuse there is no bluetooth connect after 300s, going to deep sleep now");
            delay(300);
            esp_deep_sleep_start();
        }
        Serial.println("disconnected");
        delay(1000);
    }
}