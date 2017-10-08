#include "mbed.h"

#include <stdlib.h>
#include <cmath>
#include <string>

#include <iostream>
#include <string>

#include "SNIC_WifiInterface.h"
#include "HTTPClient.h"
#if defined(TARGET_LPC1768)
#include "PowerControl/EthernetPowerControl.h"

#include "MMA7660.h"//加速度センサ用追記

#endif

#define DEMO_AP_SSID                  "chuo_customer"
#define DEMO_AP_SECURITY_TYPE         e_SEC_WPA2_AES
#define DEMO_AP_SECUTIRY_KEY          "himitudeonegaisimasu"
//
//#define DEMO_AP_SSID                  "AndroidAP"
//#define DEMO_AP_SECURITY_TYPE         e_SEC_WPA2_AES
//#define DEMO_AP_SECUTIRY_KEY          "emonday5"

//#define DEMO_AP_SSID                  "TP_LINK_0819"
//#define DEMO_AP_SECURITY_TYPE         e_SEC_WPA2_AES
//#define DEMO_AP_SECUTIRY_KEY          "04651093"


#if defined(TARGET_LPC1768)
C_SNIC_WifiInterface     wifi( p9, p10, NC, NC, p30 );
#elif defined(TARGET_NUCLEO_F401RE) || defined(TARGET_NUCLEO_L152RE)
C_SNIC_WifiInterface     wifi( D8, D2, NC, NC, D3);
#elif defined(TARGET_K64F)
C_SNIC_WifiInterface     wifi( D1, D0, NC, NC, D2);
#endif


//加速度センサ用追記
MMA7660 MMA(p28, p27);

DigitalOut connectionLed(LED1);
DigitalOut wificonnectionLed(LED4);
//PwmOut Zaxis_p(LED2);
//PwmOut Zaxis_n(LED3);

//DigitalOut myled(LED1);
Serial pc(USBTX, USBRX);

HTTPClient http;
char str[512];

int main()
{
    //加速度センサ用追記
    if (MMA.testConnection()){
        pc.printf("Axis Accelerometer connected\n");
        connectionLed = 1;
    }
    
    //while(1) {
//        pc.printf("X : %f\n", MMA.x());
//        pc.printf("Y : %f\n", MMA.y());
//        pc.printf("Z : %f\n", MMA.z());
//        wait(3);
//    }
        
        //myled = 1;
//        wait(0.2);
//        myled = 0;
//        wait(0.2);
    
#if defined(TARGET_LPC1768)
    PHY_PowerDown();
#endif

    pc.printf("WiFi init...\n");
    wifi.init();

    wait(0.5);
    int s = wifi.disconnect();
    if( s != 0 ) {
        return -1;
    }

    wait(0.3);
    // Connect AP
    wifi.connect( DEMO_AP_SSID
                  , strlen(DEMO_AP_SSID)
                  , DEMO_AP_SECURITY_TYPE
                  , DEMO_AP_SECUTIRY_KEY
                  , strlen(DEMO_AP_SECUTIRY_KEY) );
    wait(0.5);
    wifi.setIPConfig( true );     //Use DHCP
    wait(0.5);
    pc.printf("IP Address is %s\n", wifi.getIPAddress());
    wificonnectionLed = 1;

    int ret = 0;
#if 0
// disabled this test, since the library doesn't support HTTPS connection
    //GET data
    pc.printf("\nTrying to fetch page...\n");
    ret = http.get("http://tk2-254-36888.vs.sakura.ne.jp:4000/", str, 128);
    if (!ret) {
        pc.printf("Page fetched successfully - read %d characters\n", strlen(str));
        pc.printf("Result: %s\n", str);
    } else {
        pc.printf("Error - ret = %d - HTTP return code = %d\n", ret, http.getHTTPResponseCode());
    }
#endif

    //POST data
    HTTPMap map;
    HTTPText inText(str, 512);
    //map.put("Hello", "World");
//    map.put("test", "1234");
//    pc.printf("\nTrying to post data...\n");

    
    
        char _buf[20];
        while(1){
            if(((abs(MMA.y())+abs(MMA.x())+abs(MMA.z()))-0.98) >1){
                pc.printf("\nTrying to post data...\n");
                map.put("device", "red");
                //_buf = std::to_string(MMA.y()+MMA.x());
                sprintf(_buf, "%f", (abs(MMA.y())+abs(MMA.x())+abs(MMA.z()))-0.98);
                pc.printf("%s\n",_buf);
                map.put("speed", _buf);
                
                ret = http.post("http://tk2-254-36888.vs.sakura.ne.jp/red", map, &inText);
                
                if (!ret) {
                pc.printf("Executed POST successfully - read %d characters\n", strlen(str));
                pc.printf("Result: %s\n", str);
                } else {
                pc.printf("Error - ret = %d - HTTP return code = %d\n", ret, http.getHTTPResponseCode());
                }
                wait(0.5);
                map.clear();
            }
        
        }
    
//    ret = http.post("http://tk2-254-36888.vs.sakura.ne.jp/", map, &inText);
//    ret = http.post("http://httpbin.org/post", map, &inText);
//      ret = http.post("http://192.168.43.208", map, &inText);
    //if (!ret) {
//        pc.printf("Executed POST successfully - read %d characters\n", strlen(str));
//        pc.printf("Result: %s\n", str);
//    } else {
//        pc.printf("Error - ret = %d - HTTP return code = %d\n", ret, http.getHTTPResponseCode());
//    }
    
    wifi.disconnect();
    while(1) {
    }
}
