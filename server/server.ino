#include <WiFi.h>
#include <WiFiMulti.h>
#include <WebSocketsServer.h>
#if defined(ESP32)
#include <ESPmDNS.h>
#elif defined(ARDUINO_ARCH_RP2040)
#include <LEAmDNS.h>
#endif
#define SW_PIN 5
#define LED_PIN 4

const char* ssid = "your_ssid";
const char* pass = "your_pass";
const char* host = "wsserver";

WiFiMulti WiFiMulti;
WebSocketsServer ws = WebSocketsServer(8000);
uint8_t c_num = -1;
int sw_stat = HIGH;

void wsEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
    switch(type) {
        case WStype_CONNECTED:
            {
                c_num = num;
                IPAddress ip = ws.remoteIP(num);
                Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
            }
            break;
        case WStype_DISCONNECTED:
            c_num = -1;
            Serial.printf("[%u] Disconnected!\n", num);
            break;
        case WStype_TEXT:
            String str = (char*) payload;
            Serial.printf("[%u] get Text: %s\n", num, payload);
            if (str == "led on") {
              Serial.println("led on");
              digitalWrite(LED_PIN, HIGH);
            }
            else if (str == "led off") {
              Serial.println("led off");
              digitalWrite(LED_PIN, LOW);
            }
            break;
    }
}

void setup() {
    Serial.begin(115200);

    pinMode(SW_PIN, INPUT_PULLUP);
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
    WiFiMulti.addAP(ssid, pass);
    while(WiFiMulti.run() != WL_CONNECTED) {
        delay(100);
        Serial.print(".");
    }
    Serial.println();
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    if (MDNS.begin(host)) {
        Serial.println("MDNS responder started");
    }
    ws.begin();
    ws.onEvent(wsEvent);
}

void loop() {
    int stat;

#if defined(ARDUINO_ARCH_RP2040)
    MDNS.update();
#endif
    ws.loop();
    stat = digitalRead(SW_PIN);
    if (stat != sw_stat) {
        sw_stat = stat;
        if (c_num != -1) {
            Serial.println(stat == LOW ? "send on" : "send off");
            ws.sendTXT(c_num, stat == LOW ? "on" : "off");
        }
    }
}
