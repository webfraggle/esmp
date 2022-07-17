/*
  board settings:
  board:            ESP32 Dev Module
  upload speed:     921600
  cpu frequency:    240MHz (WiFi/BT)
  flash frequency:  40MHz
  flash mode:       DIO
  flash size:       16MB (128Mb)
  psram:            enabled

  model: https://github.com/Xinyuan-LilyGO/LilyGo-EPD47
*/

#include <Arduino.h>
#include "epd_driver.h" // from https://github.com/Xinyuan-LilyGO/LilyGo-EPD47 library

#include "env.h"
#define BATT_PIN            36
int vref = 1100;

#include "default_picture.h"
#include "low_battery.h"

#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

int current_hour = 0, current_min = 0, current_sec = 0;
long start_time = 0;

uint8_t *framebuffer;

void setup() {
    start_time = millis();
    Serial.begin(230400);

    if (startWiFi() == WL_CONNECTED && setupTime() == true) {
        if ((current_hour >= WAKEUP_TIME && current_hour <= SLEEP_TIME)) {

            Serial.println("Initialising Display");
            epd_init();
            epd_poweron();
            epd_clear();

            framebuffer = (uint8_t *)heap_caps_malloc(DISPLAY_WIDTH * DISPLAY_HEIGHT / 2, MALLOC_CAP_SPIRAM);
            memset(framebuffer, 0xFF, (DISPLAY_WIDTH * DISPLAY_HEIGHT)/2);

            String bat_string = checkBattery();

            WiFiClient client;

            int attempt = 0;
            int image_received = false;
            while (attempt < 3  && !image_received) {
                attempt++;
                image_received = getImage(client, bat_string);
            }
            if(image_received) {
                displayImage();
            } else {
                displayDefaultImage();
            }
        }

        updateLocalTime();
        stopWiFi();
    }
    sleep();
}

// never runs
void loop() {
}

void sleep() {
    epd_poweroff_all();

    long sleep_timer = (SLEEP_DURATION * 60 - ((current_min % SLEEP_DURATION) * 60 + current_sec));
    // Add 20 seconds extra delay because of slow ESP32 RTC timers
    esp_sleep_enable_timer_wakeup((sleep_timer+20) * 1000000LL);

#ifdef BUILTIN_LED
    // If it's On, turn it off and some boards use GPIO-5 for SPI-SS, which remains low after screen use
    pinMode(BUILTIN_LED, INPUT);
    digitalWrite(BUILTIN_LED, HIGH);
#endif

    Serial.println("Awake for : " + String((millis() - start_time) / 1000.0, 3) + " seconds");
    Serial.println("Entering " + String(sleep_timer) + " seconds of sleep time");
    Serial.println("Starting deep-sleep.");
    esp_deep_sleep_start();
}


bool getImage(WiFiClient& client, String bat_string) {
    Serial.println("Attempting to get image");

    // close connection before sending a new request
    client.stop();

    HTTPClient http;
    http.getStream().setNoDelay(true);
    http.getStream().setTimeout(3000000);
    http.setConnectTimeout(3000000);
    http.setTimeout(3000000);
    http.begin(TODAY_URL+"?val="+String(bat_string));

    Serial.println(TODAY_URL+"?val="+String(bat_string));
    
    int httpCode = http.GET();

    char snum[5];
    itoa(httpCode, snum, 10);
    Serial.printf("code: %s", snum);


    if(httpCode == HTTP_CODE_OK) {

        Serial.print("HTTP OK");

        WiFiClient stream = http.getStream();
        

        // counter for framebuffer index
        int fb_counter = 0;

        int byte_counter = 0;
        byte stream_byte;
        unsigned long stream_size = 40000;
        unsigned long clength = 0;

        while (fb_counter < stream_size) {
            //Serial.printf("%d %d\n",fb_counter, stream_size);
            if (stream.available() == 0) continue;
            // read next byte
            //Serial.printf("\navail: %d \n",stream.available());
            
            stream_byte = (byte)stream.read();
            byte_counter++;

            if (byte_counter < 5)
            {
              clength <<= 8;
              clength |= stream_byte;
              Serial.printf("\nfirst 4Bytes: %d %d \n",stream_byte,clength);
              if (byte_counter == 4) stream_size = clength;
              continue;
            }
            
            //Serial.printf("%d \n",stream_byte);
            framebuffer[fb_counter] = (byte)stream_byte;
            fb_counter++;
              
        }
        Serial.printf("\nFB Counter: %d \n",fb_counter);

    } else {
        Serial.println();
        Serial.printf("connection failed, error: %s", http.errorToString(httpCode).c_str());
        Serial.println();
        return false;
    }

    client.stop();
    http.end();

    return true;
}

void displayImage() {
    Serial.println("Displaying image");
    Rect_t area = {
        .x = 0,
        .y = 0,
        .width = DISPLAY_WIDTH,
        .height =  DISPLAY_HEIGHT
    };
    epd_draw_grayscale_image(area, framebuffer);
}

void displayDefaultImage() {
    Serial.println("Displaying default image");
    Rect_t area = {
        .x = 0,
        .y = 0,
        .width = default_picture_width,
        .height =  default_picture_height
    };
    epd_draw_grayscale_image(area, (uint8_t *) default_picture_data);
}


uint8_t startWiFi() {
    Serial.print("\r\nConnecting to: " + String(SSID));

    // Google DNS
    // IPAddress dns(8, 8, 8, 8);
    WiFi.disconnect();
    WiFi.mode(WIFI_MODE_STA);
    WiFi.setAutoConnect(true);
    WiFi.setAutoReconnect(true);
    WiFi.begin(SSID, PASSWORD);

    unsigned long start = millis();
    uint8_t connection_status;
    bool connecting = true;
    while (connecting) {
        connection_status = WiFi.status();

        // Wait 15-secs maximum
        if (millis() > start + 15000) {
            connecting = false;
        }
        if (connection_status == WL_CONNECTED || connection_status == WL_CONNECT_FAILED) {
            connecting = false;
        }
        delay(50);
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("WiFi connected at: " + WiFi.localIP().toString());
    } else {
        Serial.println("WiFi connection failed");
    }

    return WiFi.status();
}

void stopWiFi() {
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
}

boolean setupTime() {
    configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER, "time.nist.gov");

    setenv("TZ", TIMEZONE, 1);
    tzset();
    delay(100);
    bool time_status = updateLocalTime();
    return time_status;
}

boolean updateLocalTime() {
    struct tm time;

    // Wait for 5-sec for time to synchronise
    while (!getLocalTime(&time, 5000)) {
        Serial.println("Failed to obtain time");
        return false;
    }

    current_hour = time.tm_hour;
    current_min  = time.tm_min;
    current_sec  = time.tm_sec;
    return true;
}

// check battery level and if 15% or less, display icon
String checkBattery() {
    Serial.println("Checking battery level");

    delay(10); // Make adc measurement more accurate
    uint16_t v = analogRead(BATT_PIN);
    float voltage = ((float)v / 4095.0) * 2.0 * 3.3 * (vref / 1000.0);
    //String bat_voltage = "➸ Voltage: " + String(battery_voltage) + "V";
    //Serial.println(bat_voltage);
    //return;
    
    uint8_t percentage = 100;
    //float voltage = analogRead(35) / 4096.0 * 7.46;
    if (voltage > 1 ) {
        Serial.println("Voltage = " + String(voltage));
        percentage = 2836.9625 * pow(voltage, 4) - 43987.4889 * pow(voltage, 3) + 255233.8134 * pow(voltage, 2) - 656689.7123 * voltage + 632041.7303;
        Serial.println(percentage);
        if (voltage >= 4.20) percentage = 100;
        if (voltage <= 3.50) percentage = 0;
        if (percentage <= 15) {
            Rect_t area = {
                .x = 0,
                .y = 0,
                .width = low_battery_width,
                .height =  low_battery_height
            };
            epd_draw_grayscale_image(area, (uint8_t *) low_battery_data);
        }
    }
    return String(voltage)+"-"+String(percentage);
}
