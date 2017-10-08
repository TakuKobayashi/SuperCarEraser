KX022-1020のサンプルコードをベースに開発
http://www.rohm.co.jp/web/japan/sensor-shield-support-001/accelerometer

ESP8266で本サンプルコードを使う場合は
KX022.cppの
#include <avr\pgmspace.h>
を以下のように修正
#if (defined(__AVR__))
#include <avr\pgmspace.h>
#else
#include <pgmspace.h>
#endif