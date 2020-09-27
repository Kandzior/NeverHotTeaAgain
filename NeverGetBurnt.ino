// Test all components for the project, including:
// - Arduino Uno --> translated to Arudino Nano V3.0
// - DS18B20
// - output transistor for ultra bright LED's (3)
// - proximity sensor KTIR0711S
// - WS2812B 24 diodes (circle)


#include <Adafruit_NeoPixel.h> //Dołączenie biblioteki
#include <OneWire.h>
#include <DallasTemperature.h>

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
  //Fading the LED

  for (int k = 0; k< 24; k++) linijka.setPixelColor(j++, 0, 0, 0);  // r, g, b
  
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

  for (int k = 0; k< 24; k++) linijka.setPixelColor(j++, 255, 0, 0);  // r, g, b
  linijka.show();
  analogWrite(led_pin, 255);
  delay(5000);
  j=0;
  //linijka.clear();

  for (int k = 0; k< 24; k++) linijka.setPixelColor(j++, 0, 0, 255);  // r, g, b
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
