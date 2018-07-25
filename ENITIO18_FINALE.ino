#define RED //CHOOSE CLAN: RED GREEN BLUE YELLOW

#include <Arduino.h>
#include <IRremote.h>
#include <Adafruit_NeoPixel.h>
#include <EEPROMex.h>

#define LDR_PIN A0
#define HEALTH_PIN 5
#define BUZZER_PIN 8
#define ACTIVATED_PIN 4
#define IR_PIN 11
#define BRIGHTNESS 50
#define LIFE 50           //no. of pixels showing damage ratio
#define ACTIVATED 50      //no. of pixels showing tower is activated
#define DPS 2901          //Damage per second code
#define DEATHPULSE 2900   //Instant-death code
#define ATTACKED_R1 2704  //Weakest attack
#define ATTACKED_R2 2705  //Weak attack
#define ATTACKED_R3 2706  //Strong attack
#define ATTACKED_R4 2707  //Strongest attack
#define ATTACKED_G1 2708
#define ATTACKED_G2 2709
#define ATTACKED_G3 2710
#define ATTACKED_G4 2711
#define ATTACKED_B1 2712
#define ATTACKED_B2 2713
#define ATTACKED_B3 2714
#define ATTACKED_B4 2715
#define ATTACKED_Y1 2716
#define ATTACKED_Y2 2717
#define ATTACKED_Y3 2718
#define ATTACKED_Y4 2719
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

int addr, threshold, R, G, B, Y, avgR, avgG, avgB, avgY, dmgR, dmgG, dmgB, dmgY;
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

//Update Scores
void updateScores(int A, int B, int C, int D)
{

  #if DEBUG
  Serial.println("updating Scores");
  Serial.println("-------------------------------------------------------------");
  #endif

  EEPROM.updateInt(addr, A);
  addr += sizeof(A);
  EEPROM.updateInt(addr, B);
  addr += sizeof(B);
  EEPROM.updateInt(addr, C);
  addr += sizeof(C);
  EEPROM.updateInt(addr, D);
  addr = 0;
}

void setup()
{
  pinMode(LDR_PIN, INPUT);
  pinMode(IR_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
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
    R = 1000;
  #endif
  #ifdef GREEN
    colorWipe(green, 50);
    G = 1000;
  #endif
  #ifdef BLUE
    colorWipe(blue, 50);
    B = 1000;
  #endif
  #ifdef YELLOW
    colorWipe(yellow, 50);
    C = 1000;
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

  addr = EEPROM.getAddress(sizeof(int));
}

void loop()
{
  int value = isShot(LDR_PIN, threshold);
  unsigned long currentMillis1 = millis(), currentMillis2 = millis(), currentMillis3 = millis();
  const unsigned resultLength = results.bits;
	const unsigned resultsValue = results.value;

  //When no laser hitting LDR
  if (value == 0)
  {
    deactivated();
  }

  //When laser is hitting LDR
  //Incur damage when shot according to type of attack
  if (value == 1)
  {
    activated();
    if (irrecv.decode(&results))
    {
      const unsigned resultLength = results.bits;
		  const unsigned resultsValue = results.value;
      #if DEBUG
        Serial.println();
        Serial.print("DMG received: ");
        Serial.println(DMG);
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
            dmgR += dmg1;
            break;
          case ATTACKED_G1:
            G += dmg1;
            dmgG += dmg1;
            break;
          case ATTACKED_B1:
            B += dmg1;
            dmgB += dmg1;
            break;
          case ATTACKED_Y1:
            Y += dmg1;
            dmgY += dmg1;
            break;
          case ATTACKED_R2:
            R += dmg2;
            dmgR += dmg2;
            break;
          case ATTACKED_G2:
            G += dmg2;
            dmgG += dmg2;
            break;
          case ATTACKED_B2:
            B += dmg2;
            dmgB += dmg2;
            break;
          case ATTACKED_Y2:
            Y += dmg2;
            dmgY += dmg2;
            break;
          case ATTACKED_R3:
            R += dmg3;
            dmgR += dmg3;
            break;
          case ATTACKED_G3:
            G += dmg3;
            dmgG += dmg3
            break;
          case ATTACKED_B3:
            B += dmg3;
            dmgB += dmg3;
            break;
          case ATTACKED_Y3:
            Y += dmg3;
            dmgY += dmg3;
            break;
          case ATTACKED_R4:
            R += dmg4;
            dmgR += dmg4;
            break;
          case ATTACKED_G4:
            G += dmg4;
            dmgG += dmg4;
            break;
          case ATTACKED_B4:
            B += dmg4;
            dmgB += dmg4;
            break;
          case ATTACKED_Y4:
            Y += dmg4;
            dmgY += dmg4;
            break;
          default:
            delay(50);
            break;
        }
      }
    }
  }

  //Ratio of damage dealt
  avgR == round((R/(R+G+B+Y))*LIFE);
  avgG == round(((G/(R+G+B+Y))*LIFE)+avgR);
  avgB == round(((B/(R+G+B+Y))*LIFE)+avgR+avgG);
  avgY == round(((Y/(R+G+B+Y))*LIFE)+avgR+avgG+avgB);

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
    for (x = 0; x < 15; x++)
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

  for (int i = 0 ; i < avgR ; i++)
  {
    Health_Pixels.setPixelColor(i, red);
  }
  for (int i = avgR ; i < avgG ; i++)
  {
    Health_Pixels.setPixelColor(i, green);
  }
  for (int i = avgG ; i < avgB ; i++)
  {
    Health_Pixels.setPixelColor(i, blue);
  }
  for (int i = avgB ; i < avgY ; i++)
  {
    Health_Pixels.setPixelColor(i, yellow);
  }

  Health_Pixels.show();

  interrupts();
  irrecv.resume();
  irrecv.enableIRIn();

  #if DEBUG
    Serial.println();
    Serial.println("SCORES");
    Serial.print("RED : ");
    Serial.println(EEPROM.readInt(addr));
    Serial.print("GREEN : ");
    Serial.println(EEPROM.readInt(addr + sizeof(R)));
    Serial.print("BLUE : ");
    Serial.println(EEPROM.readInt(addr + sizeof(G)));
    Serial.print("YELLOW : ");
    Serial.println(EEPROM.readInt(addr + sizeof(B)));
    Serial.println("-------------------------------------------------------------");
  #endif

  //Update Scores
  if (currentMillis3 - previousMillis3 >= 90000)
  {
    previousMillis3 = currentMillis3;
    updateScores(dmgR, dmgG, dmgB, dmgY);
  }
}