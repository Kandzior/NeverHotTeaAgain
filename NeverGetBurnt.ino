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
void RGB_blue_circling(int PWM, int start_pos);
void RGB_red(int PWM);
void RGB_green(int PWM);
void RGB_off(void);
void RGB_effect_1 (int PWM, int speed_RGB);
int CheckCup(void);

// there an be few users / few different cups
// each user should have his own presets:
// - cup height (from table to the bottom)
// - treshold temperature
// - info about transparency of the cup - if the cup isn't transparent there is noo need to flash 5mm leds from bottom
// - PWM of 5mm leds
// - PWM of leds (RGB)

// Conception changed - to less complicated. State machine will be soon. Now it's three four state: 
// cup is out/ cup is on --> temp too high, temp ok, temp too low

//struct userDataStruct {
//  int cup_height;
//  float treshold_temperature;
//  char transparency;
//  int PWM_RGB;
//  int PWM_5mm;
//} user1data;
//userDataStruct user2data;
//userDataStruct user3data;


int NumberOfCups = 2; // number of cups defined is project
int Cup[10][2];

float TempCup[10][2]; // Temp from red to green, temp from green to blue

// preprogrammed values - for nowe for only one type of cup
float temp_treshold = 35.5;   // temperature, when color changes from red to green (during falling)
float temp_cold = 28.0;       // below that tamperature RGB goes blue
float temp;

// programming the device for now should be done manually. If you want to do that, connect Arduino to USB,
// turn on the serial monitor and watch the displayed values - temperature and distance. Prepare your drink, put it on the device,
// write the distance and watch the tepmerture rise and wait till it start falling. When the drink is cold enuogh, write down 
// the temperature and go back to code. Remember - if you want to do that correctly, it will be better to to dhat in two steps:
// first - make a hot drink, check when it's good (measure time)
// second - put the drink on the device and measure temperature, when you'll take the drink away after measured time,
// if you'll do that few times, the temperature will not be correct
// if you want to enter programming mode - write PROGRAMMING: 1, else - write 0
#define PROGRAMMING 0

OneWire oneWire(2);                   //Podłączenie do A5
DallasTemperature sensors(&oneWire);  //Przekazania informacji do biblioteki

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
  Serial.println("Start of device..."); 

  sensors.begin(); //Inicjalizacja czujnikow

  // definition of cup's (distance sensor)
Cup[0][0]= 48; Cup[0][1] = 58;  // definition of Cup No 0 - between number 1 and 2
Cup[1][0]= 73; Cup[1][1] = 100; // definition of cup No 2
// definition of temperatures
TempCup[0][0] = 35; TempCup[0][1] = 30;   // first - threshold to green from red, second - from green to blue
TempCup[1][0] = 32; TempCup[1][1] = 29;
}

int j=0;
int l=0;
float temp_previous=0, temp_actuall;
int previous_cup_number = 100, cup_number=100;
int internat_effect_counter = 0;

void loop() {

  // programming mode or normal mode
  while(PROGRAMMING == 1)
  {
    // display distance 0-1024
    int odl = analogRead(czujnik);      //odczytanie wartości z czujnika
    Serial.println(odl);                //wyświetlenie jej na monitorze
    
    // display temperature
    sensors.requestTemperatures(); //Pobranie temperatury czujnika
    Serial.print("Pierwszy: ");
    Serial.println(sensors.getTempCByIndex(0));  //Wyswietlenie informacji

    // auxiliary sensor - for monitoring battery temoperature - optional, not needed, for future use
    Serial.print("Drugi: ");
    Serial.println(sensors.getTempCByIndex(1));  //Wyswietlenie informacji

    // setting the PWM of white diodes to 50%
    analogWrite(led_pin, 127);

    // setting the color of 24LED strip to GREEN
    for (int k = 0; k< 24; k++) linijka.setPixelColor(j++, 0, 255, 0);  // r, g, b
    linijka.show();
    sensors.requestTemperatures();
    delay(5000);
  }
  
  // normal mode (PROGRAMMING == 0)
  while(1)
  {
    // checking cup presence
    cup_number = CheckCup();
    if (cup_number <10 && previous_cup_number == 100)   // befor was nothing putted on the top - start the LED's WHITE
    {
      sensors.requestTemperatures();
      for (int i = 0; i<255; i++) {analogWrite(led_pin, i); delay(10);}
      previous_cup_number = cup_number;
    }
    if (cup_number == 100 && previous_cup_number != 100)
    {
      for (int i = 0; i<255; i++) {analogWrite(led_pin, 254-i); delay(7);}
      previous_cup_number = cup_number;
      RGB_blue(1);
    }
    Serial.print(cup_number); Serial.print(", ");
    Serial.println(analogRead(czujnik));
    if (cup_number != 100) 
    {
      temp_actuall = sensors.getTempCByIndex(0); Serial.println(temp_actuall); sensors.requestTemperatures();
      if (temp_actuall < TempCup[cup_number][1]) {RGB_blue_circling(100, internat_effect_counter++); if(internat_effect_counter>23) internat_effect_counter=0;}
      else
      {
        if(temp_actuall > TempCup[cup_number][0]) RGB_red(64);
        else
        {
          RGB_green(64);
        }
      }
      
    }
    delay(10);
    

  }




}

void RGB_blue(int PWM)
{
  j=0;
  for (int k = 0; k< 24; k++) linijka.setPixelColor(j++, 0, 0, PWM);  // r, g, b
  linijka.show();
  ;
}
void RGB_blue_circling(int PWM, int start_pos)
{
  int TAB[24];
    
    
  for (int k = 0; k<24;k++) 
  {
    
    if ((start_pos - k) < 0) TAB[24+start_pos-k] = PWM;
    else TAB[start_pos-k] = PWM;
    PWM = PWM/2;
  }
  j=0;
  for (int k = 0; k< 24; k++) linijka.setPixelColor(j++, 0, 0, TAB[j]);  // r, g, b
  linijka.show();
  ;
}
void RGB_red(int PWM)
{
    j=0;
  for (int k = 0; k< 24; k++) linijka.setPixelColor(j++, PWM, 0, 0);  // r, g, b
  linijka.show();
}
void RGB_green(int PWM)
{
    j=0;
  for (int k = 0; k< 24; k++) linijka.setPixelColor(j++, 0, PWM, 0);  // r, g, b
  linijka.show();
}

void RGB_off(void)
{
    j=0;
  for (int k = 0; k< 24; k++) linijka.setPixelColor(j++, 0, 0, 0);  // r, g, b
  linijka.show();
}

void RGB_effect_1 (int PWM, int speed_RGB)    // todo
{
  ;
}

// returns number of cup (first, second...) beginning from 0, no cup is outputed by 100
int CheckCup(void)
{
  for (int i = 0; i< NumberOfCups; i++)
  {
    if ( analogRead(czujnik) > Cup[i][0] && analogRead(czujnik) < Cup[i][1]) 
      { return (i);}  // returns number of cup
  }
  return (100);   // returns 100, if no cup was detected


}
