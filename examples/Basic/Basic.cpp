	// #include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PushbulletAPI.hpp>

const char* ssid = "<SSID>";
const char* password = "<WIFI-Password>";
const char* pushbullet_key = "<Pushbullet API key>";
const char* pushbullet_fingerprint = "<Pushbullet API SSL Fingerprint>";

// You can pass fingerprint verification by using:
// PushbulletAPI pb(pushbullet_key);
// * when using this, is will still connect using ssl but without any certificate verification 
// Additionaly, you can set a rule to use only verified connection:
// PushbulletAPI pb(pushbullet_key, pushbullet_fingerprint, true);
// * when using this, if fingerprint is not match to pushbullet certificate it won't connect.
// Or use the basic, with fingerprint but dynamically. So if fingerprint is not match it will set a warning in seiral port but still push messages throw pushbullet. 
PushbulletAPI pb(pushbullet_key, pushbullet_fingerprint);

void setup()
{
    // Set baud for serial data
    Serial.begin(115200);

    // Set & Connect to WiFi network
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) 
    {
    delay(500);
    Serial.print(".");
    }

    // Set Pushbullet Push Message

    pb.pushNotifcation("Hello from ESP8266!", "Hello World!");
    pb.pushNotifcationUrl("See that great site", "and don't forget to donate to it.", "https://www.wikipedia.org/");
}

void loop()
{
}