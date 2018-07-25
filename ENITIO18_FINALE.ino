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
#define LIFE 50
#define ATTACKED_A1 2704
#define ATTACKED_A2 2705
#define ATTACKED_A3 2706
#define ATTACKED_A4 2707
#define ATTACKED_B1 2708
#define ATTACKED_B2 2709
#define ATTACKED_B3 2710
#define ATTACKED_B4 2711
#define ATTACKED_C1 2712
#define ATTACKED_C2 2713
#define ATTACKED_C3 2714
#define ATTACKED_C4 2715
#define ATTACKED_D1 2716
#define ATTACKED_D2 2717
#define ATTACKED_D3 2718
#define ATTACKED_D4 2719
#define DEBUG 1 // 1 for DEBUG ; 0 for PLAY

Adafruit_NeoPixel Health_Pixels = Adafruit_NeoPixel(LIFE, HEALTH_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel Activated_Pixels = Adafruit_NeoPixel(LIFE, ACTIVATED_PIN, NEO_GRB + NEO_KHZ800);
IRrecv irrecv(IR_PIN);
IRsend irsend;
decode_results results;

int addr, x, k, j, threshold, HP = LIFE, DMG = 0, HP2 = -1, HP3 = -1, A, B, C, D;
unsigned long previousMillis1 = 0, previousMillis2 = 0, previousMillis3 = 0;

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

//Attacked animation
void attacked()
{
  noInterrupts();
  Health_Pixels.setPixelColor(HP, Health_Pixels.Color(0, 0, 255));
  Health_Pixels.setPixelColor(HP2, Health_Pixels.Color(0, 0, 255));
  Health_Pixels.setPixelColor(HP3, Health_Pixels.Color(0, 0, 255));
  Health_Pixels.show();
  delay(100);
  interrupts();
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

//Activated animation
void activated()
{
  noInterrupts();
  for (int i = 0; i < LIFE; i++)
  {
    Activated_Pixels.setPixelColor(i, Activated_Pixels.Color(150, 150, 150));
  }
  Activated_Pixels.show();
  interrupts();
}

//Deactivated animation
void deactivated()
{
  noInterrupts();
  for (int i = 0; i < LIFE; i++)
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
  colorWipe(Health_Pixels.Color(150, 0, 0), 50);
  colorWipe(Health_Pixels.Color(150, 150, 0), 50);
  colorWipe(Health_Pixels.Color(0, 150, 0), 50);
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

  //When no Damage
  if (value == 0)
  {
    deactivated();
  }

  //Incur damage when shot according to type of attack
  if (value == 1)
  {
    activated();
    if (irrecv.decode(&results))
    {
#if DEBUG
      Serial.print("Damage code: ");
      Serial.println(results.value);
      Serial.println("-------------------------------------------------------------");
#endif
      switch (results.value)
      {
      case ATTACKED_A1:
        A += 1;
        DMG += 1;
        attacked();
        break;
      case ATTACKED_B1:
        B += 1;
        DMG += 1;
        attacked();
        break;
      case ATTACKED_C1:
        C += 1;
        DMG += 1;
        attacked();
        break;
      case ATTACKED_D1:
        D += 1;
        DMG += 1;
        attacked();
        break;
      case ATTACKED_A2:
        A += 2;
        DMG += 2;
        attacked();
        break;
      case ATTACKED_B2:
        B += 2;
        DMG += 2;
        attacked();
        break;
      case ATTACKED_C2:
        C += 2;
        DMG += 2;
        attacked();
        break;
      case ATTACKED_D2:
        D += 2;
        DMG += 2;
        attacked();
        break;
      case ATTACKED_A3:
        A += 3;
        DMG += 3;
        attacked();
        break;
      case ATTACKED_B3:
        B += 3;
        DMG += 3;
        attacked();
        break;
      case ATTACKED_C3:
        C += 3;
        DMG += 3;
        attacked();
        break;
      case ATTACKED_D3:
        D += 3;
        DMG += 3;
        attacked();
        break;
      case ATTACKED_A4:
        D += 10;
        DMG += 10;
        attacked();
        break;
      case ATTACKED_B4:
        B += 10;
        DMG += 10;
        attacked();
        break;
      case ATTACKED_C4:
        C += 10;
        DMG += 10;
        attacked();
        break;
      case ATTACKED_D4:
        D += 10;
        DMG += 10;
        attacked();
        break;
      default:
        delay(50);
        break;
      }
      irrecv.resume();
    }
  }

#if DEBUG
  Serial.println();
  Serial.print("DMG received: ");
  Serial.println(DMG);
  Serial.println("-------------------------------------------------------------");
#endif

  //Minus HP when stacked DMG hits limit
  if (DMG >= 20)
  {
    HP -= 1;
    HP2 -= 1;
    HP3 -= 1;
    DMG = 0;
  }

  //Reset HP2 to correct value when HP reaches 1
  if (HP == 0)
  {
    HP2 = LIFE;
  }

  //reset HP3 to correct value when HP2 reaches 1
  if (HP2 == 0)
  {
    HP3 = LIFE;
  }

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
      irsend.sendSony(2901, 12);
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
    for (x = 0; x < 10; x++)
    {
      digitalWrite(BUZZER_PIN, HIGH);
      delay(50);
      digitalWrite(BUZZER_PIN, LOW);
      delay(50);
    }
    delay(100);

//Tower fires
#if DEBUG
    Serial.println("Deathpulse");
    Serial.println("-------------------------------------------------------------");
#endif
    for (int m = 0; m < 3; m++)
    {
      irsend.sendSony(2900, 12);
      delay(40);
    }
    irrecv.enableIRIn();
    irrecv.resume();
  }

  delay(100);

  //3 diffent coloured bars indicate 3 stages of health
  noInterrupts();

  for (int i = 0; i < LIFE; i++)
  {
    if (i <= HP && HP >= 0)
    {
      Health_Pixels.setPixelColor(i, Health_Pixels.Color(0, 150, 0));
    }
    else if (i > HP && HP >= 0)
    {
      Health_Pixels.setPixelColor(i, Health_Pixels.Color(150, 150, 0));
    }
    else if (i <= HP2 && HP < 0 && HP2 >= 0)
    {
      Health_Pixels.setPixelColor(i, Health_Pixels.Color(150, 150, 0));
    }
    else if (i > HP2 && HP < 0 && HP2 >= 0)
    {
      Health_Pixels.setPixelColor(i, Health_Pixels.Color(150, 0, 0));
    }
    else if (i < HP3 && HP < 0 && HP2 < 0)
    {
      Health_Pixels.setPixelColor(i, Health_Pixels.Color(150, 0, 0));
    }
    else
    {
      Health_Pixels.setPixelColor(i, Health_Pixels.Color(0, 0, 0));
    }
  }

  Health_Pixels.show();

  interrupts();
  irrecv.enableIRIn();
  irrecv.resume();

#if DEBUG
  Serial.println();
  Serial.print("HP : ");
  Serial.println(HP);
  Serial.print("HP2 : ");
  Serial.println(HP2);
  Serial.print("HP3 : ");
  Serial.println(HP3);
  Serial.println("-------------------------------------------------------------");
#endif

#if DEBUG
  Serial.println();
  Serial.println("SCORES");
  Serial.print("A : ");
  Serial.println(EEPROM.readInt(addr));
  Serial.print("B : ");
  Serial.println(EEPROM.readInt(addr + sizeof(A)));
  Serial.print("C : ");
  Serial.println(EEPROM.readInt(addr + sizeof(B)));
  Serial.print("D : ");
  Serial.println(EEPROM.readInt(addr + sizeof(C)));
  Serial.println("-------------------------------------------------------------");
#endif

  //Update Scores
  if (currentMillis3 - previousMillis3 >= 90000)
  {
    previousMillis3 = currentMillis3;
    updateScores(A, B, C, D);
  }
}