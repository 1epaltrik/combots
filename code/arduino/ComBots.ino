/******************************************************************
 * Κάδος κομποστοποίησης
 * 1ο ΕΠΑΛ Τρικάλων - Ομάδα ComBots
 * 4ος Πανελλήνιος Διαγωνισμός Ανοιχτών Τεχνολογιών στην Εκπαίδευση
 ******************************************************************/

// βιβλιοθήκες
#include "DFRobot_RGBLCD1602.h"
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

// θύρες εισόδου-εξόδου
const int switchPin = 4;  // θύρα σύνδεσης του διακόπτη εύρεσης της αρχικής θέσης του κάδου
const int motorPin = 10;  // θύρα ελέγχου αντλίας
const int fanPin = 11;    // θύρα ελέγχου ανεμιστήρα
const int dirPin = 2;     // θύρα ελέγχου της φοράς περιστροφής του stepper motor
const int stepPin = 3;    // θύρα ελέγχου των βημάτων του stepper motor

const int stepsPerRevolution = 400;   // το μοτέρ μας κάνει 1 πλήρη περιστροφή σε 400 βήματα

int strofi = 110;             // ο αριθμός βημάτων για την περιστροφή του κάδου
int stepperSpeed = 15000;     // μεγάλη τιμή -> μικρή ταχύτητα -> μεγάλη ροπή
bool DexiaAristera = HIGH;    // LOW: προς τα δεξιά - HIGH: προς τα αριστερά

// οθόνη LCD 16x2
DFRobot_RGBLCD1602 lcd(16,2);

// αναλογικές θύρες σύνδεσης αισθητήρων
const int hygrometer = A0;    // 1ος αισθητήρας υγρασίας εδάφους
const int hygrometer2 = A2;   // 2ος αισθητήρας υγρασίας εδάφους
const int MQ4pin = A1;        // αισθητήρας μεθανίου

// οι τιμές των αισθητήρων
int value = 0;        // 1ος αισθητήρας υγρασίας εδάφους
int value2 = 0;       // 2ος αισθητήρας υγρασίας εδάφους
int methValue = 0;    // μεθάνιο
float temp = 0.0;     // θερμοκρασία
float hum = 0.0;      // υγρασία αέρα
float pressure = 0.0; // ατμοσφαιρική πίεση

// ο αισθητήρας BME280
Adafruit_BME280  bme; // I2C
//Adafruit_BME280 bme(BME_CS); // hardware SPI
//Adafruit_BME280 bme(BME_CS, BME_MOSI, BME_MISO, BME_SCK); // software SPI

void setup()
{
  // αρχικοποίηση serial monitor
  Serial.begin(9600);

  // αρχικοποίηση οθόνης LCD
  lcd.init();

  lcd.print("1o EPAL TRIKALON");
  Serial.println("1o EPAL TRIKALON");
  Serial.println("----------------");
  delay(500);
  
  // ορισμός θυρών εξόδου
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  pinMode(motorPin, OUTPUT);
  pinMode(fanPin, OUTPUT);

  digitalWrite(fanPin, HIGH);   // ανεμιστήρας κλειστός
  digitalWrite(motorPin, HIGH); // αντλία κλειστή

  // ορισμός θυρών εισόδου
  pinMode(switchPin, INPUT_PULLUP); // ενεργοποίηση της εσωτερικής pullup αντίστασης

  // περιστροφή του κάδου στην αρχική θέση (προς τα δεξιά)
  lcd.setCursor(0, 0);
  lcd.print("PERISTROFI KADOU");
  lcd.setCursor(0, 1);
  lcd.print("STHN ARXIKH 8ESI");
  digitalWrite(dirPin, LOW);
  while( digitalRead(switchPin) == LOW )  // μέχρι να πατηθεί ο διακόπτης...
  {
    digitalWrite(stepPin, HIGH);      // ...περιστροφή κατά ένα βήμα
    delayMicroseconds(stepperSpeed);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(800);
  }
  delay(300);
  // ανάποδη περιστροφή για απελευθέρωση του διακόπτη
  digitalWrite(dirPin, HIGH);
  for(int i=0; i<20; i++)
  {
    digitalWrite(stepPin, HIGH);      // περιστροφή κατά ένα βήμα
    delayMicroseconds(stepperSpeed);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(800);
  }

  // αρχικοποίηση αισθητήρα BME
  Serial.println("BME280 test");
  bool status;
  status = bme.begin();
  if (!status)
  {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    lcd.setCursor(0, 0);
    lcd.print("BME error.......");
    while (1);  // ατέρμων βρόχος - τέλος προγράμματος
  }
}

void loop()
{
  readSensors();      // διάβασμα αισθητήρων

  printValues();      // εμφάνιση τιμών αισθητήρων στο serial monitor

  screen1();          // εμφάνιση τιμών αισθητήρων στην οθόνη LCD
  delay(2000);
  screen2();
  delay(2000);
  screen3();
  delay(2000);

  checkConditions();  // έλεγχος συνθηκών
}

// υποπρόγραμμα για διάβασμα αισθητήρων
void readSensors()
{
  // διάβασμα αισθητήρα BME θερμοκρασίας, υγρασίας αέρα και πίεσης
  temp = bme.readTemperature();
  hum = bme.readHumidity();
  pressure = bme.readPressure() / 100.0F;

  // διάβασμα υγρασίας εδάφους από τους 2 αισθητήρες
  value = analogRead(hygrometer);
  value = constrain(value,400,1023);
  // μετατροπή υγρασίας σε ποσοστό
  value = map(value,400,1023,100,0);

  value2 = analogRead(hygrometer2);
  value2 = constrain(value2,400,1023);
  // μετατροπή υγρασίας σε ποσοστό
  value2 = map(value2,400,1023,100,0);

  // διάβασμα αισθητήρα μεθανίου
  methValue = analogRead(MQ4pin);
}

// 1η οθόνη: εμφάνιση υγρασίας εδάφους
void screen1()
{
  lcd.clear();
  lcd.setCursor(0, 0); 
  lcd.print("H1: ");
  lcd.print(value);
  lcd.print("%");
  lcd.print(" - H2: ");
  lcd.print(value2);
  lcd.print("%");
  lcd.setCursor(0, 1); 
  lcd.print("AvgHum: ");
  lcd.print((value+value2)/2);
}

// 2η οθόνη: εμφάνιση θερμοκρασίας - ατμοσφαιρικής πίεσης
void screen2()
{
  lcd.clear();
  lcd.setCursor(0, 0); 
  lcd.print("Temp: ");
  lcd.print(temp);
  lcd.setCursor(0, 1); 
  lcd.print("Pressure: ");
  lcd.print(pressure);
}

// 3η οθόνη: εμφάνιση μεθανίου - υγρασίας αέρα
void screen3()
{
  lcd.clear();
  lcd.setCursor(0, 0); 
  lcd.print("Methane: ");
  lcd.print(methValue);
  lcd.setCursor(0, 1); 
  lcd.print("AirHum: ");
  lcd.print(hum);
}

// υποπρόγραμμα ελέγχου των τιμών των αισθητήρων και λήψης απόφασης για τις αντίστοιχες ενέργειες
void checkConditions()
{
  // έλεγχος αν η υγρασία αέρα είναι > 60% 
  lcd.clear();
  if( hum > 60 )
  {
    lcd.setCursor(0, 0);
    lcd.print("POLLH YGRASIA   ");
    lcd.setCursor(0, 1);
    lcd.print("ANEMISTHRAS ON  ");
    digitalWrite(fanPin, LOW);    // ενεργοποίηση ανεμιστήρα
  }
  else
  {
    lcd.setCursor(0, 0);
    lcd.print("LIGH YGRASIA");
    lcd.setCursor(0, 1);
    lcd.print("ANEMISTHRAS OFF ");    
    digitalWrite(fanPin, HIGH);   // απενεργοποίηση ανεμιστήρα
  }
  delay(2000);

  // έλεγχος αν η υγρασία εδάφους είναι < 40%
  lcd.clear();
  if( (value + value2)/2 < 40 )
  {
    lcd.setCursor(0, 0);
    lcd.print("THELEI NERO     ");
    lcd.setCursor(0, 1);
    lcd.print("ANTLIA ANOIKTH  ");
    digitalWrite(motorPin, LOW);    // ενεργοποίηση αντλίας
  }
  else
  {
    lcd.setCursor(0, 0);
    lcd.print("EXEI ARKETO NERO");
    lcd.setCursor(0, 1);
    lcd.print("ANTLIA KLEISTH  ");
    digitalWrite(motorPin, HIGH);   // απενεργοποίηση αντλίας
  }
  delay(2000);

  // ελεγχος για το μεθάνιο
  lcd.clear();
  if( methValue > 650 )
  {
    lcd.setCursor(0, 0);
    lcd.print("METHANIO ARKETO ");
    delay(1000);
    peristrofiKadou();    // αναποδογύρισμα κάδου
  }
  else
  {
    delay(1000);
  }
}

// υποπρόγραμμα για αναποδογύρισμα κάδου
void peristrofiKadou()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("PERISTROFI KADOU");
  lcd.setCursor(0, 1);
  if( DexiaAristera )
    lcd.print("ARISTERA        ");
  else
    lcd.print("DEXIA           ");

  // ορισμός κατεύθυνσης περιστροφής
  digitalWrite(dirPin, DexiaAristera);
  // περιστροφή κατά "strofi" βήματα
  for(int x = 0; x < strofi; x++)
  {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(stepperSpeed);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(800);
  }
  DexiaAristera = !DexiaAristera; // αλλαγή κατεύθυνσης περιστροφής
}

// υποπρόγραμμα εμφάνισης τιμών στη σειριακή οθόνη
void printValues()
{
  Serial.println("SENSOR VALUES");
  Serial.println("--------------------------");

  Serial.print("Temperature = ");
  Serial.print(temp);
  Serial.println(" *C");
  
  Serial.print("Pressure = ");
  Serial.print(pressure);
  Serial.println(" hPa");
  
  Serial.print("Air Humidity = ");
  Serial.print(hum);
  Serial.println(" %");

  Serial.print("Soil Humidity 1 = ");
  Serial.print(value);
  Serial.println(" %");

  Serial.print("Soil Humidity 2 = ");
  Serial.print(value2);
  Serial.println(" %");

  Serial.print("Methane = ");
  Serial.println(methValue);
  Serial.println();
}
