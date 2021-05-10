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




#include <LiquidCrystal_PCF8574.h>
#include <Wire.h>
/*
 * Prints readings like these to the Serial console:
 *
 * AQI 56  PM 1.0 = 16, PM 2.5 = 18, PM 4.0 = 21, PM 10.0 = 21
 * AQI 55  PM 1.0 = 15, PM 2.5 = 17, PM 4.0 = 20, PM 10.0 = 20
 * AQI 54  PM 1.0 = 14, PM 2.5 = 16, PM 4.0 = 18, PM 10.0 = 18
 * etc.
 */

#define MIN 5       // granica przy której oczyszczacz się wyłącza
#define HIST 3      // histereza
#define DELAY 100     // ilość minut od ostatniego wwyłączenia oczysczacza

#define PinA 2 
#define PinB 3 
 
unsigned long time = 0; 
long ilosc_impulsow = 0; 
long licznik = 0;

#include <SoftwareSerial.h>

SoftwareSerial hpmSerial(4,5);

LiquidCrystal_PCF8574 lcd(0x27); // set the LCD address to 0x27 for a 16 chars and 2 line display


char out = 0;
char out_licznik = 5;
char out_poprz = 0;

// Arduino Due code for Honeywell HPMA115S0-XXX particle sensor
// https://electronza.com/arduino-measuring-pm25-pm10-honeywell-hpma115s0/

bool my_status;

// IMPORTANT!!! We are working on an Arduino DUE, 
// so int is 32 bit (-2,147,483,648 to 2,147,483,647)
// For Arduino Uno int size is 8 bit, that is -32,768 to 32,767
// Use long or float if working with an Uno or simmilar 8-bit board
int  PM25;
int  PM10;

void(* resetFunc) (void) = 0;   // funckja resetująca Arduino za pomocą software

void setup() {

    int error;
  Serial.begin(9600);
  //while (!Serial);
  hpmSerial.begin(9600);
  pinMode(10, OUTPUT);    // sets the digital pin 13 as output

    // See http://playground.arduino.cc/Main/I2cScanner how to test for a I2C device.
  Wire.begin();
  Wire.beginTransmission(0x27);
  error = Wire.endTransmission();
  lcd.begin(20, 2); // initialize the lcd
  lcd.setBacklight(255);

  if (error == 0) {
    Serial.println(": LCD found.");
    lcd.setCursor(0, 0); lcd.print("LCD init     ");
  }
  
  delay(500); Serial.print("-----  ");

lcd.setCursor(0, 0); lcd.print("Stop meas.      ");
   my_status = stop_measurement();
   delay(2000);

  // Stop autosend
  lcd.setCursor(0, 0); lcd.print("Stop autosend.      ");
  my_status = stop_autosend(); 
  // Serial print is used just for debugging
  // But one can design a more complex code if desired
  Serial.print("Stop autosend status is ");
  Serial.println(my_status, BIN);
  Serial.println(" ");
  delay(2000);

  lcd.setCursor(0, 0); lcd.print("Start meas.      ");
   my_status = start_measurement();
  // Serial print is used just for debugging
  // But one can design a more complex code if desired
  Serial.print("Start measurement status is ");
  Serial.println(my_status, BIN);
  Serial.println(" ");
  delay(3000);

//      if (my_status != 1) {
////               /* Debug ...  */
////               //lcd.setCursor(0, 0);  lcd.print("RESET Arduino za 2s  ");
//               Serial.print("RESET Arduino za 2 s (HMPA nie odpowiada)");
//               delay(500);
//               resetFunc();
//      }
 pinMode(PinA,INPUT); 
 pinMode(PinB,INPUT); 

 //pinMode(7,INPUT);
 
// attachInterrupt(0, blinkA, LOW); 
// attachInterrupt(1, blinkB, LOW); 
 
 time = millis(); 

}

void loop() {

if (out) {digitalWrite(10, HIGH); digitalWrite(13, HIGH);}
else {digitalWrite(10, LOW); digitalWrite(13, LOW);}

  
  lcd.setCursor(0, 0); lcd.print("PM25 PM10 STAT v:  *");
  lcd.setCursor(0, 1); lcd.print("               h:   ");
  // Read the particle data every minute
  my_status = read_measurement(); 
  // Serial print is used just for debugging
  // But one can design a more complex code if desired
  Serial.print("Read measurement status is ");
  Serial.println(my_status, BIN);
  Serial.print("PM2.5 value is ");
  Serial.println(PM25, DEC);
  Serial.print("PM10 value is ");
  Serial.println(PM10, DEC);
  Serial.println(" ");

//Serial.println(my_status, BIN);
lcd.setCursor(10, 1); lcd.print("  ");
if (my_status)
{lcd.setCursor(10, 1); lcd.print(" OK ");}
else
{lcd.setCursor(10, 1); lcd.print(" err");}

lcd.setCursor(6, 1); lcd.print("     ");
lcd.setCursor(1, 1); lcd.print("     ");

if (my_status){
lcd.setCursor(6, 1); lcd.print(PM10);
lcd.setCursor(1, 1); lcd.print(PM25);
}
else
{
lcd.setCursor(6, 1); lcd.print("---");
lcd.setCursor(1, 1); lcd.print("---");
}
//if (ilosc_impulsow < 0)
//ilosc_impulsow = 0;
//if (ilosc_impulsow >99)
//ilosc_impulsow = 99;
//
//lcd.setCursor(17, 1); lcd.print(ilosc_impulsow);

if (out_licznik > 0)
out_licznik--;
if (out_licznik == 0)
{
// sprawdzanie czy wlaczyc czy wylaczyc
if (out == 0) 
  if (PM25 > MIN + HIST) {out = 1; out_licznik = DELAY;} 
if (out == 1)
  if (PM25 < MIN) {out = 0; out_licznik = DELAY;}
}
if (out) {lcd.setCursor(19, 0); lcd.print("*");}
else {lcd.setCursor(19, 0); lcd.print(" ");}

if (out_licznik > 0)
{lcd.setCursor(10, 0); lcd.print("    ");
lcd.setCursor(11, 0); lcd.print(out_licznik,DEC);}
else {lcd.setCursor(10, 0); lcd.print("STAT");}


  delay(3000);

}

bool start_measurement(void)
{
  // First, we send the command
  byte start_measurement[] = {0x68, 0x01, 0x01, 0x96 };
  hpmSerial.write(start_measurement, sizeof(start_measurement));
  //Then we wait for the response
  while(hpmSerial.available() < 2);
  char read1 = hpmSerial.read();
  char read2 = hpmSerial.read();
  // Test the response
  if ((read1 == 0xA5) && (read2 == 0xA5)){
    // ACK
    return 1;
  }
  else if ((read1 == 0x96) && (read2 == 0x96))
  {
    // NACK
    return 0;
  }
  else return 0;
}

bool stop_measurement(void)
{
  // First, we send the command
  byte stop_measurement[] = {0x68, 0x01, 0x02, 0x95 };
  hpmSerial.write(stop_measurement, sizeof(stop_measurement));
  //Then we wait for the response
  while(hpmSerial.available() < 2);
  char read1 = hpmSerial.read();
  char read2 = hpmSerial.read();
  // Test the response
  if ((read1 == 0xA5) && (read2 == 0xA5)){
    // ACK
    return 1;
  }
  else if ((read1 == 0x96) && (read2 == 0x96))
  {
    // NACK
    return 0;
  }
  else return 0;
}

bool read_measurement (void)
{
  // Send the command 0x68 0x01 0x04 0x93
  byte read_particle[] = {0x68, 0x01, 0x04, 0x93 };
  hpmSerial.write(read_particle, sizeof(read_particle));
  // A measurement can return 0X9696 for NACK
  // Or can return eight bytes if successful
  // We wait for the first two bytes
  while(hpmSerial.available() < 1);
  byte HEAD = hpmSerial.read();
  while(hpmSerial.available() < 1);
  byte LEN = hpmSerial.read();
  // Test the response
  if ((HEAD == 0x96) && (LEN == 0x96)){
    // NACK
    Serial.println("NACK");
    return 0;
  }
  else if ((HEAD == 0x40) && (LEN == 0x05))
  {
    // The measuremet is valid, read the rest of the data 
    // wait for the next byte
    while(hpmSerial.available() < 1);
    byte COMD = hpmSerial.read();
    while(hpmSerial.available() < 1);
    byte DF1 = hpmSerial.read(); 
    while(hpmSerial.available() < 1);
    byte DF2 = hpmSerial.read();     
    while(hpmSerial.available() < 1);
    byte DF3 = hpmSerial.read();   
    while(hpmSerial.available() < 1);
    byte DF4 = hpmSerial.read();     
    while(hpmSerial.available() < 1);
    byte CS = hpmSerial.read();      
    // Now we shall verify the checksum
    if (((0x10000 - HEAD - LEN - COMD - DF1 - DF2 - DF3 - DF4) % 0XFF) != CS){
      Serial.println("Checksum fail");
      return 0;
    }
    else
    {
      // Checksum OK, we compute PM2.5 and PM10 values
      PM25 = DF1 * 256 + DF2;
      PM10 = DF3 * 256 + DF4;
      return 1;
    }
  }
}

bool stop_autosend(void)
{
 // Stop auto send
  byte stop_autosend[] = {0x68, 0x01, 0x20, 0x77 };
  hpmSerial.write(stop_autosend, sizeof(stop_autosend));
  //Then we wait for the response
  while(hpmSerial.available() < 2);
  char read1 = hpmSerial.read();
  char read2 = hpmSerial.read();
  // Test the response
  if ((read1 == 0xA5) && (read2 == 0xA5)){
    // ACK
    return 1;
  }
  else if ((read1 == 0x96) && (read2 == 0x96))
  {
    // NACK
    return 0;
  }
  else return 0;
}

bool start_autosend(void)
{
 // Start auto send
  byte start_autosend[] = {0x68, 0x01, 0x40, 0x57 };
  hpmSerial.write(start_autosend, sizeof(start_autosend));
  //Then we wait for the response
  while(hpmSerial.available() < 2);
  char read1 = hpmSerial.read();
  char read2 = hpmSerial.read();
  // Test the response
  if ((read1 == 0xA5) && (read2 == 0xA5)){
    // ACK
    return 1;
  }
  else if ((read1 == 0x96) && (read2 == 0x96))
  {
    // NACK
    return 0;
  }
  else return 0;
}


void blinkA()
{
 if ((millis() - time) > 3)
 ilosc_impulsow++; 
 time = millis();
}
 
void blinkB()
{
 if ((millis() - time) > 3) 
 ilosc_impulsow-- ;
 time = millis();
}

