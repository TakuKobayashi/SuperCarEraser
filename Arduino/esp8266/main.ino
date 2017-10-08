#include <Wire.h>
#include <KX022.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

const char *ssid = "***";
const char *password = "***";

const char* host = "tk2-254-36888.vs.sakura.ne.jp";
KX022 kx022(KX022_DEVICE_ADDRESS_1E);

void setup() {
    byte rc;

    Serial.begin(9600);
    while (!Serial);

    Wire.begin(2,14);
    rc = kx022.init();

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
}

void loop() {
    byte rc;
    float acc[3];

    rc = kx022.get_val(acc);
    if (rc == 0) {
        float speed = abs(acc[0]) + abs(acc[1]) + abs(acc[2]) - 0.98;
        if(speed > 0.2) {
            Serial.println(speed); 
            WiFiClient client;
            const int httpPort = 4000;
            if (!client.connect(host, httpPort)) {
                Serial.println("connection failed");
                return;
            }

            String data = "{\"device\":\"blue\",\"speed\":\""+(String)speed+"\"}";
            client.println("POST /blue HTTP/1.1");
            client.print("Host: ");
            client.println(host);
            client.println("Accept: */*");
            client.println("Content-Type: application/json");
            client.print("Content-Length: ");
            client.println(data.length());
            client.println();
            client.print(data);

            delay(1000); // Can be changed

            if (client.connected()) { 
                client.stop();  // DISCONNECT FROM THE SERVER
            }
            Serial.println("done.");
        }
    }
    delay(500);
}