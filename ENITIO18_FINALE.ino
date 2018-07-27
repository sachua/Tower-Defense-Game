#define RED //CHOOSE CLAN: RED GREEN BLUE YELLOW

#include <Arduino.h>
#include <IRremote.h>
#include <Adafruit_NeoPixel.h>
#include <EEPROMex.h>
#include <math.h>

//TODO: Standardise DPS, DEATHPULSE, ATTACK codes from clans

#define LDR_PIN A0
#define HEALTH_PIN 5
#define BUZZER_PIN 8
#define ACTIVATED_PIN 4
#define IR_PIN 11
#define RESET_PIN 13
#define BRIGHTNESS 50
#define LIFE 50           //no. of pixels showing damage ratio
#define ACTIVATED 50      //no. of pixels showing tower is activated
#define DPS 3000          //Damage per second code
#define DEATHPULSE 3001   //Instant-death code
#define ATTACKED_R1 2601  //Weakest attack
#define ATTACKED_R2 2602  //Weak attack
#define ATTACKED_R3 2603  //Strong attack
#define ATTACKED_R4 2801  //Strongest attack 2604
#define ATTACKED_G1 2604
#define ATTACKED_G2 2704
#define ATTACKED_G3 2904
#define ATTACKED_G4 2804
#define ATTACKED_B1 2901
#define ATTACKED_B2 2902
#define ATTACKED_B3 2903
#define ATTACKED_B4 2802 //2904
#define ATTACKED_Y1 2701
#define ATTACKED_Y2 2702
#define ATTACKED_Y3 2703
#define ATTACKED_Y4 2803 //2704
#define dmg1 1            //Weakest attack damage
#define dmg2 2            //weak attack damage
#define dmg3 5            //Strong attack damage
#define dmg4 10           //Strongest attack damage
#define DEBUG 1           // 1 for DEBUG ; 0 for PLAY

Adafruit_NeoPixel Health_Pixels = Adafruit_NeoPixel(LIFE, HEALTH_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel Activated_Pixels = Adafruit_NeoPixel(ACTIVATED, ACTIVATED_PIN, NEO_GRB + NEO_KHZ800);
IRrecv irrecv(IR_PIN);
IRsend irsend;
decode_results results;

int addr = 0, threshold, avgR = 0, avgG = 0, avgB = 0, avgY = 0;
float R = 0, G = 0, B = 0, Y = 0;
unsigned long previousMillis1 = 0, previousMillis2 = 0, previousMillis3 = 0;
unsigned long red = Health_Pixels.Color(150, 0, 0), green = Health_Pixels.Color(0, 150, 0), blue = Health_Pixels.Color(0, 0, 150), yellow = Health_Pixels.Color(150, 150, 0);

//Get threshold before game starts
int getThreshold(int pin, int samples, float multiplier)
{
  unsigned long reading = 0;
  for (int i = 0; i < samples; i++)
  {
    reading += analogRead(pin);
  }
  reading = reading / samples;
  return reading * multiplier;
}

//Determine if tower is attacked or not
boolean isShot(int pin, int threshold)
{
  threshold += 100;
  #if DEBUG
    Serial.println("LDR DETECTION");
    Serial.print("Threshold: ");
    Serial.println(threshold);
    Serial.print("Current: ");
    Serial.println(analogRead(pin));
    Serial.println("-------------------------------------------------------------");
  #endif
  if (analogRead(pin) > threshold)
  {
  #if DEBUG
    Serial.println("Tower Activated");
    Serial.println("-------------------------------------------------------------");
  #endif
    return true;
  }
  else
    return false;
}

//ColourWipe animation
void colorWipe(uint32_t c, uint8_t wait)
{
  for (uint16_t i = 0; i < Health_Pixels.numPixels(); i++)
  {
    Health_Pixels.setPixelColor(i, c);
    Health_Pixels.show();
    delay(wait);
  }
}

//Tower activated indication
void activated()
{
  noInterrupts();
  for (int i = 0; i < ACTIVATED; i++)
  {
    Activated_Pixels.setPixelColor(i, Activated_Pixels.Color(150, 150, 150));
  }
  Activated_Pixels.show();
  interrupts();
}

//Tower deactivated indication
void deactivated()
{
  noInterrupts();
  for (int i = 0; i < ACTIVATED; i++)
  {
    Activated_Pixels.setPixelColor(i, Activated_Pixels.Color(0, 0, 0));
  }
  Activated_Pixels.show();
  interrupts();
}

//Read Scores
void readScores()
{
  #if DEBUG
  Serial.println("Reading Past Scores");
  Serial.println("-------------------------------------------------------------");
  #endif

  R = EEPROM.readInt(addr);
  addr += sizeof(int);
  G = EEPROM.readInt(addr);
  addr += sizeof(int);
  B = EEPROM.readInt(addr);
  addr += sizeof(int);
  Y = EEPROM.readInt(addr);
  addr = 0;
}

//Update Scores
void updateScores(int A, int B, int C, int D)
{

  #if DEBUG
  Serial.println("updating Scores");
  Serial.println("-------------------------------------------------------------");
  #endif

  EEPROM.updateInt(addr, A);
  addr += sizeof(int);
  EEPROM.updateInt(addr, B);
  addr += sizeof(int);
  EEPROM.updateInt(addr, C);
  addr += sizeof(int);
  EEPROM.updateInt(addr, D);
  addr = 0;
}

void setup()
{
  pinMode(LDR_PIN, INPUT);
  pinMode(IR_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RESET_PIN, INPUT_PULLUP);
  pinMode(HEALTH_PIN, OUTPUT);
  pinMode(ACTIVATED_PIN, OUTPUT);

  Health_Pixels.begin();
  Health_Pixels.setBrightness(BRIGHTNESS);
  Activated_Pixels.begin();
  Activated_Pixels.setBrightness(BRIGHTNESS);

  #if DEBUG
    Serial.begin(9600);
  #endif

  //Enable IR
  #if DEBUG
    Serial.println("-------------------------------------------------------------");
    Serial.println("Enabling IR");
  #endif
  irrecv.enableIRIn();
  #if DEBUG
    Serial.println("IR Enabled");
    Serial.println("-------------------------------------------------------------");
  #endif

  //HealthBar animation
  #if DEBUG
    Serial.println("Filling HealthBar");
  #endif
  #ifdef RED
    colorWipe(red, 50);
  #endif
  #ifdef GREEN
    colorWipe(green, 50);
  #endif
  #ifdef BLUE
    colorWipe(blue, 50);
  #endif
  #ifdef YELLOW
    colorWipe(yellow, 50);
  #endif
  #if DEBUG
    Serial.println("HealthBar filled");
    Serial.println("-------------------------------------------------------------");
  #endif

  //Obtain threshold
  #if DEBUG
    Serial.println("Obtaining threshold");
  #endif
  threshold = getThreshold(LDR_PIN, 100, 0.9);
  #if DEBUG
    Serial.println("Threshold obtained");
    Serial.println("-------------------------------------------------------------");
  #endif

  addr = EEPROM.getAddress(sizeof(int));  //Initialise the addr for Arduino hard drive memory

  readScores(); //Read past scores
}

void loop()
{
  bool value = isShot(LDR_PIN, threshold);
  unsigned long currentMillis1 = millis(), currentMillis2 = millis(), currentMillis3 = millis();
  const unsigned resultLength = results.bits;
	const unsigned resultsValue = results.value;

  if (!digitalRead(RESET_PIN))
  {
    #if DEBUG
      Serial.println("RESET");
    #endif
    #ifdef RED
      R = 1000;
      G = 0;
      B = 0;
      Y = 0;
    #endif
    #ifdef GREEN
      R = 0;
      G = 1000;
      B = 0;
      Y = 0;
    #endif
    #ifdef BLUE
      R = 0;
      G = 0;
      B = 1000;
      Y = 0;
    #endif
    #ifdef YELLOW
      R = 0;
      G = 0;
      B = 0;
      Y = 1000;
    #endif
  }

  //When no laser hitting LDR
  if (value == 0)
  {
    deactivated(); //deactivate the white neopixels
  }

  //When laser is hitting LDR
  //Incur damage when shot according to type of attack
  if (value == 1)
  {
    activated();  //light up the white neopixels to show that it can now be attacked
    if (irrecv.decode(&results))
    {
      const unsigned resultsLength = results.bits;
		  const unsigned resultsValue = results.value;
      #if DEBUG
        Serial.println();
        Serial.print("DMG received! ");
        Serial.print("Damage code: ");
        Serial.println(resultsValue);
        Serial.println("-------------------------------------------------------------");
      #endif
      irrecv.resume(); // Receive the next value
		  irrecv.enableIRIn();
      if(resultsLength <= 16){
        switch (resultsValue)
        {
          case ATTACKED_R1:
            R += dmg1;
            break;
          case ATTACKED_G1:
            G += dmg1;
            break;
          case ATTACKED_B1:
            B += dmg1;
            break;
          case ATTACKED_Y1:
            Y += dmg1;
            break;
          case ATTACKED_R2:
            R += dmg2;
            break;
          case ATTACKED_G2:
            G += dmg2;
            break;
          case ATTACKED_B2:
            B += dmg2;
            break;
          case ATTACKED_Y2:
            Y += dmg2;
            break;
          case ATTACKED_R3:
            R += dmg3;
            break;
          case ATTACKED_G3:
            G += dmg3;
            break;
          case ATTACKED_B3:
            B += dmg3;
            break;
          case ATTACKED_Y3:
            Y += dmg3;
            break;
          case ATTACKED_R4:
            R += dmg4;
            break;
          case ATTACKED_G4:
            G += dmg4;
            break;
          case ATTACKED_B4:
            B += dmg4;
            break;
          case ATTACKED_Y4:
            Y += dmg4;
            break;
          default:
            delay(50);
            break;
        }
      }
    }
  }

  //Ratio of damage dealt
  avgR = roundf((R/(R+G+B+Y))*LIFE);
  avgG = roundf(((G/(R+G+B+Y))*LIFE)+avgR);
  avgB = roundf(((B/(R+G+B+Y))*LIFE)+avgG);
  avgY = roundf(((Y/(R+G+B+Y))*LIFE)+avgB);

  //Tower attacking
  if (currentMillis1 - previousMillis1 >= 1000)
  {
    previousMillis1 = currentMillis1;

    //Buzzer Beep
    digitalWrite(BUZZER_PIN, HIGH);
    delay(50);
    digitalWrite(BUZZER_PIN, LOW);

    //Tower fires
    #if DEBUG
      Serial.println("Fire");
      Serial.println("-------------------------------------------------------------");
    #endif
    for (int m = 0; m < 3; m++)
    {
      irsend.sendSony(DPS, 12);
      delay(40);
    }
    irrecv.enableIRIn();
    irrecv.resume();
  }

  //Tower death pulse
  if (currentMillis2 - previousMillis2 >= 30000)
  {
    previousMillis2 = currentMillis2;

    //Buzzer warning
    for (int x = 0; x < 20; x++)
    {
      digitalWrite(BUZZER_PIN, HIGH);
      delay(50);
      digitalWrite(BUZZER_PIN, LOW);
      delay(50);
    }
    delay(1000);

    //Tower fires
    #if DEBUG
      Serial.println("Deathpulse");
      Serial.println("-------------------------------------------------------------");
    #endif
    for (int m = 0; m < 3; m++)
    {
      irsend.sendSony(DEATHPULSE, 12);
      delay(40);
    }
    irrecv.enableIRIn();
    irrecv.resume();
  }

  delay(100);

  //Neopixels depict ratio of damage dealt by clan
  noInterrupts();
  for (int i = 0 ; i < LIFE ; i++)
  {
    if (i < avgR)
    {
      Health_Pixels.setPixelColor(i, red);
    }
    else if (i<avgG){
      Health_Pixels.setPixelColor(i, green);
    }
    else if(i<avgB){
      Health_Pixels.setPixelColor(i, blue);
    }
    else{
      Health_Pixels.setPixelColor(i, yellow);
    }
  }

  Health_Pixels.show();

  interrupts();
  irrecv.resume();
  irrecv.enableIRIn();

  #if DEBUG
    Serial.println("SCORES");
    Serial.print("Number of red pixels: ");
    Serial.println(avgR);
    Serial.print("Number of green pixels: ");
    Serial.println(avgG - avgR);
    Serial.print("Number of blue pixels: ");
    Serial.println(avgB - avgG);
    Serial.print("Number of yellow pixels: ");
    Serial.println(avgY - avgB);
    Serial.println("-------------------------------------------------------------");
  #endif

  //Update Scores
  if (currentMillis3 - previousMillis3 >= 90000)
  {
    previousMillis3 = currentMillis3;
    updateScores(R, G, B, Y);
  }
}
