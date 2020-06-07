// Class weather fetches information about the current weather.

#ifndef Weather_h
#define Weather_h

#include <ArduinoJson.h>
#include <Arduino.h>
#include <HTTPClient.h>
#include <string>

// OpenWeatherMAP URL
const char *kOpenWeatherMapURL = "https://api.openweathermap.org/data/2.5";

// CA root cert for api.openweathermap.org
const char *kOpenWeatherMapCA = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIF3jCCA8agAwIBAgIQAf1tMPyjylGoG7xkDjUDLTANBgkqhkiG9w0BAQwFADCB\n" \
"iDELMAkGA1UEBhMCVVMxEzARBgNVBAgTCk5ldyBKZXJzZXkxFDASBgNVBAcTC0pl\n" \
"cnNleSBDaXR5MR4wHAYDVQQKExVUaGUgVVNFUlRSVVNUIE5ldHdvcmsxLjAsBgNV\n" \
"BAMTJVVTRVJUcnVzdCBSU0EgQ2VydGlmaWNhdGlvbiBBdXRob3JpdHkwHhcNMTAw\n" \
"MjAxMDAwMDAwWhcNMzgwMTE4MjM1OTU5WjCBiDELMAkGA1UEBhMCVVMxEzARBgNV\n" \
"BAgTCk5ldyBKZXJzZXkxFDASBgNVBAcTC0plcnNleSBDaXR5MR4wHAYDVQQKExVU\n" \
"aGUgVVNFUlRSVVNUIE5ldHdvcmsxLjAsBgNVBAMTJVVTRVJUcnVzdCBSU0EgQ2Vy\n" \
"dGlmaWNhdGlvbiBBdXRob3JpdHkwggIiMA0GCSqGSIb3DQEBAQUAA4ICDwAwggIK\n" \
"AoICAQCAEmUXNg7D2wiz0KxXDXbtzSfTTK1Qg2HiqiBNCS1kCdzOiZ/MPans9s/B\n" \
"3PHTsdZ7NygRK0faOca8Ohm0X6a9fZ2jY0K2dvKpOyuR+OJv0OwWIJAJPuLodMkY\n" \
"tJHUYmTbf6MG8YgYapAiPLz+E/CHFHv25B+O1ORRxhFnRghRy4YUVD+8M/5+bJz/\n" \
"Fp0YvVGONaanZshyZ9shZrHUm3gDwFA66Mzw3LyeTP6vBZY1H1dat//O+T23LLb2\n" \
"VN3I5xI6Ta5MirdcmrS3ID3KfyI0rn47aGYBROcBTkZTmzNg95S+UzeQc0PzMsNT\n" \
"79uq/nROacdrjGCT3sTHDN/hMq7MkztReJVni+49Vv4M0GkPGw/zJSZrM233bkf6\n" \
"c0Plfg6lZrEpfDKEY1WJxA3Bk1QwGROs0303p+tdOmw1XNtB1xLaqUkL39iAigmT\n" \
"Yo61Zs8liM2EuLE/pDkP2QKe6xJMlXzzawWpXhaDzLhn4ugTncxbgtNMs+1b/97l\n" \
"c6wjOy0AvzVVdAlJ2ElYGn+SNuZRkg7zJn0cTRe8yexDJtC/QV9AqURE9JnnV4ee\n" \
"UB9XVKg+/XRjL7FQZQnmWEIuQxpMtPAlR1n6BB6T1CZGSlCBst6+eLf8ZxXhyVeE\n" \
"Hg9j1uliutZfVS7qXMYoCAQlObgOK6nyTJccBz8NUvXt7y+CDwIDAQABo0IwQDAd\n" \
"BgNVHQ4EFgQUU3m/WqorSs9UgOHYm8Cd8rIDZsswDgYDVR0PAQH/BAQDAgEGMA8G\n" \
"A1UdEwEB/wQFMAMBAf8wDQYJKoZIhvcNAQEMBQADggIBAFzUfA3P9wF9QZllDHPF\n" \
"Up/L+M+ZBn8b2kMVn54CVVeWFPFSPCeHlCjtHzoBN6J2/FNQwISbxmtOuowhT6KO\n" \
"VWKR82kV2LyI48SqC/3vqOlLVSoGIG1VeCkZ7l8wXEskEVX/JJpuXior7gtNn3/3\n" \
"ATiUFJVDBwn7YKnuHKsSjKCaXqeYalltiz8I+8jRRa8YFWSQEg9zKC7F4iRO/Fjs\n" \
"8PRF/iKz6y+O0tlFYQXBl2+odnKPi4w2r78NBc5xjeambx9spnFixdjQg3IM8WcR\n" \
"iQycE0xyNN+81XHfqnHd4blsjDwSXWXavVcStkNr/+XeTWYRUc+ZruwXtuhxkYze\n" \
"Sf7dNXGiFSeUHM9h4ya7b6NnJSFd5t0dCy5oGzuCr+yDZ4XUmFF0sbmZgIn/f3gZ\n" \
"XHlKYC6SQK5MNyosycdiyA5d9zZbyuAlJQG03RoHnHcAP9Dc1ew91Pq7P8yF1m9/\n" \
"qS3fuQL39ZeatTXaw2ewh0qpKJ4jjv9cJ2vhsE/zB+4ALtRZh8tSQZXq9EfX7mRB\n" \
"VXyNWQKV3WKdwrnuWih0hKWbt5DHDAff9Yk2dDLWKMGwsAvgnEzDHNb842m1R0aB\n" \
"L6KCq9NjRHDEjf8tM7qtj3u1cIiuPhnPQCjY/MiQu12ZIvVS5ljFH4gxQ+6IHdfG\n" \
"jjxDah2nGN59PRbxYvnKkKj9\n" \
"-----END CERTIFICATE-----\n";

const int kUrlLen = 512;
const int kJSONCapacity = 20480;

// Class Weather returns information about the current weather for a given location.
class Weather{
    private:
        const char *name_;
        const char *short_description_;
        const char *description_;
        int date_;
        int timezone_;
        float temp_;
        float feels_like_;
        float temp_min_;
        float temp_max_;
        float pressure_;
        float humidity_;
        float wind_speed_;
        float wind_direction_;
        float wind_gust_;

        // failure indicator
        bool failed_ = false;

        // Private methods.
        int parseResponse(String payload);

    public:
        // Constructor.
        Weather(int zipcode, const char *key);

        // Getters.
        bool failed() const { return failed_; }
        const char *name() const { return name_; }
        const char *short_description() const { return short_description_; }
        const char *description() const { return description_; }
        int date() const { return date_; }
        int timezone() const { return timezone_; }
        float temp() const { return temp_; }
        float feels_like() const { return feels_like_; }
        float temp_min() const { return temp_min_; }
        float temp_max() const { return temp_max_; }
        float pressure() const { return pressure_; }
        float humidity() const { return humidity_; }
        float wind_speed() const { return wind_speed_; }
        float wind_direction() const { return wind_direction_; }
        float wind_gust() const { return wind_gust_; }

};

// Constructor.
Weather::Weather(int zipcode, const char *key) {
    HTTPClient  https;

    char url[kUrlLen];
    sprintf(url, "%s/weather?zip=%d,us&units=imperial&appid=%s", kOpenWeatherMapURL, zipcode, key);
    if (!https.begin(url, kOpenWeatherMapCA)) {
        Serial.printf("[HTTPS] https.begin returned error for %s\n", url);
        failed_ = true;
        return;
    }

    int httpCode = https.GET();
    Serial.printf("[HTTPS] GET %s, code: %d\n", url, httpCode);

    // httpCode will be negative on error
    if (httpCode < 0) {
        Serial.printf("[HTTPS] GET failed.\n");
        failed_ = true;
        return;
    }

    // Valid HTTP code, parse JSON and fill in class variables.
    if (httpCode != HTTP_CODE_OK && httpCode != HTTP_CODE_MOVED_PERMANENTLY) {
        Serial.printf("[HTTPS] GET failed, error: %s\n", https.errorToString(httpCode).c_str());
        failed_ = true;
        return;
    }

    String payload = https.getString();
    if (parseResponse(payload) != 0) {
        failed_ = true;
        return;
    }

    https.end();
    return;
}

// Parse openweather payload into class variables. Returns 0 if everything went
// OK, or -1 if JSON parsing failed.
int Weather::parseResponse(String payload) {
    // {"coord":{"lon":-121.83,"lat":37.25},
    //  "weather":[{"id":801,"main":"Clouds","description":"few clouds","icon":"02d"}],
    //  "base":"stations",
    //  "main":{"temp":70.39,"feels_like":59.09,"temp_min":68,"temp_max":73.4,"pressure":1016,"humidity":33},
    //  "visibility":16093,
    //  "wind":{"speed":16.11,"deg":330,"gust":24.16}
    //  "clouds":{"all":20},
    //  "dt":1591485039,
    //  "sys":{"type":1,"id":5661,"country":"US","sunrise":1591447630,"sunset":1591500322},
    //  "timezone":-25200,
    //  "id":0",
    //  "name":"San Jose",
    //  "cod":200}
    //
    DynamicJsonDocument doc(kJSONCapacity);
    DeserializationError error = deserializeJson(doc, payload);
    if (error) {
      Serial.printf("JSON deserializeJson() failed for current weather: %s", error.c_str());
      return -1;
    }

    JsonArray weather = doc["weather"];
    JsonObject main = doc["main"];
    JsonObject wind = doc["wind"];
    JsonObject w0 = weather[0];

    name_ = doc["name"];

    short_description_ = w0["main"];
    description_ = w0["description"];

    date_ = doc["dt"];

    timezone_ = doc["timezone"];

    temp_ = main["temp"].as<float>();
    feels_like_ = main["feels_like"].as<float>();
    temp_min_ = main["temp_min"].as<float>();
    temp_max_ = main["temp_max"].as<float>();
    pressure_ = main["pressure"].as<float>();
    humidity_ = main["humidity"].as<float>();

    wind_speed_ = wind["speed"].as<float>();
    wind_direction_ = wind["deg"].as<float>();
    wind_gust_ = wind["gust"].as<float>();

    return 0;
}

#endif
