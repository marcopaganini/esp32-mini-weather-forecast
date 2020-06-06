// A weather client for the National Weather Service (weather.gov)
// (C) 2020 by Marco Paganini <paganini@paganini.net>

#include <Adafruit_SSD1306.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <WiFi.h>

#include <string.h>
#include <stdlib.h>
#include <cstring>

// Private constants.
#include "private.h"

// Number of intervals to wait between refreshes. This number * kIntervalTime
// will dictate the refresh interval (in this case, 20m).
const int kIntervals = 400;

// How long is each interval loop in ms (how long to display the image.)
const int kIntervalTime = 3000;

// Number of forecast periods to show. The first three are usually
// "Today", "Later Today" (or equivalent), and "tomorrow" and are
// usually sufficient for a short term forecast.
const int kForecastPeriods = 3;

// CA Root cert for the domain.
const char * kRootCACert = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDrzCCApegAwIBAgIQCDvgVpBCRrGhdWrJWZHHSjANBgkqhkiG9w0BAQUFADBh\n" \
"MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n" \
"d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD\n" \
"QTAeFw0wNjExMTAwMDAwMDBaFw0zMTExMTAwMDAwMDBaMGExCzAJBgNVBAYTAlVT\n" \
"MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j\n" \
"b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IENBMIIBIjANBgkqhkiG\n" \
"9w0BAQEFAAOCAQ8AMIIBCgKCAQEA4jvhEXLeqKTTo1eqUKKPC3eQyaKl7hLOllsB\n" \
"CSDMAZOnTjC3U/dDxGkAV53ijSLdhwZAAIEJzs4bg7/fzTtxRuLWZscFs3YnFo97\n" \
"nh6Vfe63SKMI2tavegw5BmV/Sl0fvBf4q77uKNd0f3p4mVmFaG5cIzJLv07A6Fpt\n" \
"43C/dxC//AH2hdmoRBBYMql1GNXRor5H4idq9Joz+EkIYIvUX7Q6hL+hqkpMfT7P\n" \
"T19sdl6gSzeRntwi5m3OFBqOasv+zbMUZBfHWymeMr/y7vrTC0LUq7dBMtoM1O/4\n" \
"gdW7jVg/tRvoSSiicNoxBN33shbyTApOB6jtSj1etX+jkMOvJwIDAQABo2MwYTAO\n" \
"BgNVHQ8BAf8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUA95QNVbR\n" \
"TLtm8KPiGxvDl7I90VUwHwYDVR0jBBgwFoAUA95QNVbRTLtm8KPiGxvDl7I90VUw\n" \
"DQYJKoZIhvcNAQEFBQADggEBAMucN6pIExIK+t1EnE9SsPTfrgT1eXkIoyQY/Esr\n" \
"hMAtudXH/vTBH1jLuG2cenTnmCmrEbXjcKChzUyImZOMkXDiqw8cvpOp/2PV5Adg\n" \
"06O/nVsJ8dWO41P0jmP6P6fbtGbfYmbW0W5BjfIttep3Sp+dWOIrWcBAI+0tKIJF\n" \
"PnlUkiaY4IBIqDfv8NZ5YBberOgOzW6sRBc4L0na4UU+Krk2U886UAb3LujEV0ls\n" \
"YSEY1QSteDwsOoBrp+uvFRTp2InBuThs4pFsiv9kuXclVzDAGySj4dzp30d8tbQk\n" \
"CAUw7C29C79Fv1C5qfPrmAESrciIxpg0X40KPMbp1ZWVbd4=\n" \
"-----END CERTIFICATE-----\n";

// Prototypes
std::vector<std::string> wordwrap(const char *str, int maxlen);
void printwrap(const char *str, int maxlen);



// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Current Forecast Period.
int fperiod = 0;

void setup()
{
    // Setup serial and wait for things to settle.
    Serial.begin(115200);
    delay(10);

    wificonnect(kSSID, kPassword);
    setclock();
    setdisplay();
}

void loop()
{
    // Estimated (and doubled) from https://arduinojson.org/v6/assistant
    const size_t capacity = 20480;

    WiFiClientSecure *client = new WiFiClientSecure;
    if(!client) {
        Serial.println("Unable to create client");
        delay(2000);
        return;
    }

    client -> setCACert(kRootCACert);

    DynamicJsonDocument doc(capacity);

    {
      // Add a scoping block for HTTPClient https to make sure
      // it is destroyed before WiFiClientSecure *client is.
      HTTPClient https;

      Serial.print("[HTTPS] begin...\n");
      if (https.begin(kURL, kRootCACert)) {
        // api.weather.gov requires an User-Agent header set to the email
        // of the API user, so they can contact in case of problems.
        https.setUserAgent(kEmail);

        Serial.print("[HTTPS] GET...\n");
        int httpCode = https.GET();

        // httpCode will be negative on error
        if (httpCode > 0) {
          Serial.printf("[HTTPS] GET code: %d\n", httpCode);

          // Valid HTTP code.
          if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
            String payload = https.getString();
            parseJSON(&doc, payload);
          }
        } else {
          Serial.printf("[HTTPS] GET failed, error: %s\n", https.errorToString(httpCode).c_str());
        }

        https.end();
      } else {
        Serial.printf("[HTTPS] Unable to connect to %s\n", kURL);
      }
      // End extra scoping block
    }

    delete client;

    JsonObject properties = doc["properties"];
    JsonArray properties_periods = properties["periods"];

    // Print values kIntervals times, with kIntervalTime delay between each iteration.
    for (int interval = 0; interval < kIntervals; interval++) {
        JsonObject prop = properties_periods[fperiod];

        const char *name = prop["name"];
        const int temperature = prop["temperature"];
        const char *temperatureUnit = prop["temperatureUnit"];
        //const char *detailedForecast = prop["detailedForecast"];
        const char *shortForecast = prop["shortForecast"];
        //TODO: Why is this blank even though the JSON is populated?
        //const char *temperatureTrend = prop["temperatureTrend"];
        const char *windSpeed = prop["windSpeed"];
        const char *windDirection = prop["windDirection"];

        char header[256];
        char line1[256];

        sprintf(header, "%s %d%s", name, temperature, temperatureUnit);
        sprintf(line1, "W: %s %s", windSpeed, windDirection);
        show(header, line1, shortForecast);

        fperiod = (fperiod + 1) % kForecastPeriods;
        delay(kIntervalTime);
    }

    Serial.println("Starting new cycle.");
}

// Connect to WiFi using the given ssid and password.
void wificonnect(const char *ssid, const char *password) {
    Serial.printf("\n\nConnecting to SSID %s\n", ssid);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.printf("\nWiFi connected. IP address: %s", WiFi.localIP().toString().c_str());
}


// Set the internal clock from pool.ntp.org.
void setclock() {
    configTime(0, 0, "pool.ntp.org", "time.nist.gov");

    Serial.print("Waiting for NTP time sync: ");
    time_t nowSecs = time(nullptr);
    while (nowSecs < 8 * 3600 * 2) {
      delay(500);
      Serial.print(".");
      yield();
      nowSecs = time(nullptr);
    }

    Serial.println();
    struct tm timeinfo;

    gmtime_r(&nowSecs, &timeinfo);

    Serial.print("Current time: ");
    Serial.print(asctime(&timeinfo));
}

// Prepares the OLED display.
void setdisplay() {
    // Start I2C Communication SDA = 5 and SCL = 4 on Wemos Lolin32 ESP32 with built-in SSD1306 OLED
    Wire.begin(5, 4);

    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
        Serial.println("SSD1306 allocation failed");
        for(;;); // Don't proceed, loop forever
    }
    delay(2000);
    display.clearDisplay();
}

// Parse JSON formatted weather data.
int parseJSON(DynamicJsonDocument *doc, String json) {
    // Deserialize the JSON document
    DeserializationError error = deserializeJson(*doc, json);

    // Test if parsing succeeds.
    if (error) {
      Serial.printf("JSON deserializeJson() failed: %s", error.c_str());
      return -1;
    }

    return 0;
}

// display main screen
void show(const char *header, const char *line1, const char *line2) {
    display.clearDisplay();

    // White Text (is all we got).
    display.setTextColor(SSD1306_WHITE);
    //display.setTextColor(SSD1306_BLACK, SSD1306_WHITE); // Draw 'inverse' text

    // Note: a 128x64 display can show 20 columns at size 1, or 10 at size 2.
    display.setTextSize(2);
    display.setCursor(0,0);

    // Word wrap the header at 10 columns.
    printwrap(header, 10);

    // Print remaining lines.
    display.setTextSize(1);
    printwrap(line1, 20);
    printwrap(line2, 20);

    display.display();
}

// printwrap prins a word wrapped line.
void printwrap(const char *str, int maxlen) {
    std::vector<std::string> lines = wordwrap(str, maxlen);
    if (lines.size() == 0) {
        Serial.printf("Error splitting line: %s\n", str);
        return;
    }
    for (int i = 0; i < lines.size(); i++) {
        display.println(lines[i].c_str());
    }
}

// wordwrap wraps a line at word boundaries. Returns a vector of
// std::string elements, ready to print.
std::vector<std::string> wordwrap(const char *str, int maxlen) {
    std::vector<std::string> ret;
    char *buf = strdup(str);
    if (buf == NULL) {
        return ret;
    }

    std::string line;

    for (char *s = buf; ; s = NULL) {
        char *token = strtok(s, " ");
        if (token == NULL) {
            break;
        }
        // Longer than max line (including space?) Start a new one.
        if (strlen(token) + line.length() + 1 > maxlen) {
            ret.push_back(line);
            line = token;
            continue;
        }
        // If not, just keep appending. Make sure not to add a
        // space prefix to the first line.
        if (line.length() == 0) {
            line = token;
        } else {
            line = line + " " + token;
        }
    }
    // last line
    ret.push_back(line);
    free(buf);
    return ret;
}
