#include <Adafruit_NeoPixel.h> //Dołączenie biblioteki
#include <OneWire.h>
#include <DallasTemperature.h>

// Test all components for the project, including:
// - Arduino Uno --> translated to Arudino Nano V3.0
// - DS18B20
// - output transistor for ultra bright LED's (3)
// - proximity sensor KTIR0711S
// - WS2812B 24 diodes (circle)
// Purpose: testing reactions on different cups, how works DS18B20, how much current draws led strip and how accurate is proximity detector.
// In preparation: software for one user and one cup, proper detecting of peak teperature, best effects displaed by led strip.

/* Device will work like state machine. There will be few states:
 *  1. Cup is not present.Everything is turned off. Only sesnor measures distance from infinite to 0.
 *  2. Cup is present. White leds are on. Device starts to shine - waits until themperature stops to rise. LED RGB is circling.
 *  3. Cup is present. Device detects peak of temperture rise. LED RGB starts shining RED.
 *  4. Cup is present. Temperture falled to USER_TEMPERATURE. LED RGB switches to GREEN.
 *  5. Cup is present. Device waits till user takes off the cup. Measures temperture, whet it reaches room temperture, LED is BLUE.
 *  6. Cup is taken away. LEDs turning off. 
 *  7. Cup is back. White leds is ON. RGB dimm to minimum and are GREEN until temperture reaches room temp, than go BLUE. * 
 */
int device_state = 0;
void RGB_blue(int PWM);
void RGB_red(int PWM);
void RGB_green(int PWM);
void RGB_effect_1 (int PWM, int speed_RGB);

// there an be few users / few different cups
// each user should have his own presets:
// - cup height (from table to the bottom)
// - treshold temperature
// - info about transparency of the cup - if the cup isn't transparent there is noo need to flash 5mm leds from bottom
// - PWM of 5mm leds
// - PWM of leds (RGB)

struct userDataStruct {
  int cup_height;
  int treshold_temperature;
  char transparency;
  int PWM_RGB;
  int PWM_5mm;
} user1data;
userDataStruct user2data;
userDataStruct user3data;


OneWire oneWire(2); //Podłączenie do A5
DallasTemperature sensors(&oneWire); //Przekazania informacji do biblioteki

int czujnik = A4;   //pin analogowy A1 połączony z sygnałem z czujnika
//Konfiguracja linijki
Adafruit_NeoPixel linijka = Adafruit_NeoPixel(24, A5, NEO_GRB + NEO_KHZ800);

//Initializing LED Pin
int led_pin = 6;
void setup() {
  //Declaring LED pin as output
  pinMode(led_pin, OUTPUT);
  linijka.begin(); //Inicjalizacja

  Serial.begin(9600);        //inicjalizacja monitora szeregowego
  Serial.println("Test czujnika odbiciowego"); 

    sensors.begin(); //Inicjalizacja czujnikow
}

int j=0;
int l=0;

void loop() {
  //Fading the LED - testing the components

  //for (int k = 0; k< 24; k++) linijka.setPixelColor(j++, 0, 0, 0);  // r, g, b
  
  for(int i=0; i<255; i++){
    analogWrite(led_pin, i);
    delay(20);
  }
  for(int i=255; i>0; i--){
    analogWrite(led_pin, i);
    delay(20);
  }
  int odl = analogRead(czujnik);      //odczytanie wartości z czujnika
  //Serial.println(odl);                //wyświetlenie jej na monitorze

  for (int k = 0; k< 24; k++) linijka.setPixelColor(j++, 0, 255, 0);  // r, g, b
  linijka.show();
  analogWrite(led_pin, 255);
  delay(5000);
  j=0;
  //linijka.clear();

  for (int k = 0; k< 24; k++) linijka.setPixelColor(j++, 0, 255, 0);  // r, g, b
  linijka.show();
  analogWrite(led_pin, 255);
  delay(5000);
  j=0;
  //linijka.clear();

  for (int k = 0; k< 24; k++) linijka.setPixelColor(j++, 0, 255, 0);  // r, g, b
  linijka.show();
  analogWrite(led_pin, 255);
  delay(5000);
  j=0;
  //linijka.clear();

    sensors.requestTemperatures(); //Pobranie temperatury czujnika
  Serial.print("Pierwszy: ");
  Serial.println(sensors.getTempCByIndex(0));  //Wyswietlenie informacji
  Serial.print("Drugi: ");
  Serial.println(sensors.getTempCByIndex(1));  //Wyswietlenie informacji
  Serial.print("Trzeci: ");
  Serial.println(sensors.getTempCByIndex(2));  //Wyswietlenie informacji
  delay(5000);


}

void RGB_blue(int PWM)
{
  ;
}
void RGB_red(int PWM)
{
  ;
}
void RGB_green(int PWM)
{
  ;
}
void RGB_effect_1 (int PWM, int speed_RGB)
{
  ;
}
