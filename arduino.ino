#include <FastLED.h>
#include <LiquidCrystal.h>

#define LED_PIN     6 
#define steamerPin  10
#define NUM_LEDS    60
#define BRIGHTNESS  255
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];

#define startButton 13
#define emergencyButton 8
#define UPDATES_PER_SECOND 60
#define JoyStick_X A1

CRGBPalette16 currentPalette;
TBlendType    currentBlending;

extern CRGBPalette16 myRedWhiteBluePalette;
extern const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM;


LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
#define pwmPin 9

int i, x, y, tmp;
long time, cd, digit;
bool left, right, start;

void setup() {
    delay( 1000 ); // power-up safety delay

    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
    FastLED.setBrightness(  BRIGHTNESS );
    currentPalette = RainbowColors_p;
    currentBlending = LINEARBLEND;

    for (i=0; i<60; i++) {
        leds[i] = CRGB::Green;
    }
    FastLED.show();

    pinMode(steamerPin, OUTPUT);
    pinMode(startButton, INPUT);
    pinMode(emergencyButton, INPUT);
    pinMode (JoyStick_X, INPUT);
    pinMode(pwmPin, OUTPUT);

    analogWrite(pwmPin, 120);


    lcd.begin(16, 2);
    lcd.setCursor(0,0);
    lcd.print("Setting time...");

    cd = 180;
    start = false;
}


void loop()
{
    x = analogRead (JoyStick_X);
    if ( x > 650 ) {
        lcd.setCursor(0,1);
        lcd.print("          ");
        lcd.setCursor(0,1);
        lcd.print("     -10s");
        while ( analogRead(JoyStick_X) > 650 ) {
            delay(20);
        }
        cd = cd - 10;
        if (cd < 10) cd = 10;
        lcd.setCursor(0,1);
        lcd.print("          ");
    }

    if ( x < 200) {
        lcd.setCursor(0,1);
        lcd.print("          ");
        lcd.setCursor(0,1);
        lcd.print("     +10s");
        while ( analogRead(JoyStick_X) < 200 ) {
            delay(20);
        }
        cd = cd + 10;
        if (cd > 999) cd = 999;
        lcd.setCursor(0,1);
        lcd.print("          ");
    }

    if (digitalRead(startButton)) {
        start = true;
        for (i=0; i<60; i++) {
            leds[i] = CRGB::Yellow;
        }
        FastLED.show();
        lcd.setCursor(0,0);
        lcd.print("Starting in 3s");
        FastLED.setBrightness(  10 );
        FastLED.show();
        delay(1000);
        FastLED.setBrightness( 20 );
        FastLED.show();
        lcd.setCursor(0,0);
        lcd.print("Starting in 2s");
        delay(1000);
        lcd.setCursor(0,0);
        lcd.print("Starting in 1s");
        FastLED.setBrightness(  BRIGHTNESS );
        FastLED.show();
        digitalWrite(steamerPin,HIGH);
        delay(1000);
        digitalWrite(steamerPin,LOW);
        lcd.setCursor(0,0);
        lcd.print("                      ");
        lcd.setCursor(0,0);
        lcd.print("Time remaining:");
        cd = cd*1000+millis();
    }

    while (start == true) {
        tmp = (cd - millis())/1000 + 1;
        if (tmp == 99) {
            lcd.setCursor(0,1);
            lcd.print("     ");
        }
        if (tmp == 9) {
            lcd.setCursor(0,1);
            lcd.print("     ");
        }
        lcd.setCursor(0,1);
        lcd.print(tmp);
        lcd.print(" s");
        for (i=0; i<60; i++) {
            leds[i] = CRGB::Red;
        }
        FastLED.show();

        if (digitalRead(emergencyButton)) {
            for (i=0; i<60; i++) {
                leds[i] = CRGB::Yellow;
            }
            FastLED.show();
            lcd.setCursor(0,0);
            lcd.print("                ");
            lcd.setCursor(0,0);
            lcd.print("EMERGENCY STOP");
            lcd.setCursor(0,1);
            lcd.print("                    ");
            time = millis();
            digitalWrite(steamerPin,HIGH);
            delay(1000);
            digitalWrite(steamerPin,LOW);

            while (millis() - time < 20000) {
                if (digitalRead(startButton)) {
                    delay(50); 
                    if (digitalRead(startButton)) delay(50); break;
                }
            }
            start = false;
            lcd.setCursor(0,0);
            lcd.print("                    ");
            lcd.setCursor(0,0);
            lcd.print("SETTING TIME...");
            lcd.setCursor(0,1);
            lcd.print("                    ");
            cd = 180;
            for (i=0; i<60; i++) {
                leds[i] = CRGB::Green;
            }
            delay(100);
        }

        if (tmp <= 0 ) {
            lcd.setCursor(0,0);
            lcd.print("                ");
            lcd.setCursor(0,0);
            lcd.print("IRONING COMPLETE");
            lcd.setCursor(0,1);
            lcd.print("                    ");
            lcd.setCursor(0,1);
            lcd.print("PLEASE WAIT.");
            digitalWrite(steamerPin,HIGH);
            delay(1000);
            digitalWrite(steamerPin,LOW);
            start = false;
            time = millis();

            while (true) {
                ChangePalettePeriodically();
                static uint8_t startIndex = 0;
                startIndex = startIndex + 1; /* motion speed */
                FillLEDsFromPaletteColors( startIndex);
                FastLED.show();
                FastLED.delay(1000 / UPDATES_PER_SECOND);
                if (millis() > time + 30000) break;
            }

            lcd.setCursor(0,0);
            lcd.print("                    ");
            lcd.setCursor(0,0);
            lcd.print("SETTING TIME...");
            lcd.setCursor(0,1);
            lcd.print("                    ");
            cd = 180;
            for (i=0; i<60; i++) {
                leds[i] = CRGB::Green;
            }
        }
    }

    if (cd < 100 && digit == 3) {
        lcd.setCursor(0,1);
        lcd.print("      ");
        digit = 2;
    }
    if (cd >= 100) digit = 3;
    lcd.setCursor(0,1);
    lcd.print(cd);
    FastLED.show();
}


void FillLEDsFromPaletteColors( uint8_t colorIndex)
{
    uint8_t brightness = 255;

    for( int i = 0; i < NUM_LEDS; i++) {
        leds[i] = ColorFromPalette( currentPalette, colorIndex, brightness, currentBlending);
        colorIndex += 3;
    }
}

void ChangePalettePeriodically()
{
    uint8_t secondHand = (millis() / 1000) % 60;
    static uint8_t lastSecond = 99;

    if( lastSecond != secondHand) {
        lastSecond = secondHand;
        if( secondHand ==  0)  {
            currentPalette = RainbowColors_p;
            currentBlending = LINEARBLEND;
        }
        if( secondHand == 10)  {
            currentPalette = RainbowStripeColors_p;
            currentBlending = NOBLEND;
        }
        if( secondHand == 15)  {
            currentPalette = RainbowStripeColors_p;
            currentBlending = LINEARBLEND;
        }
        if( secondHand == 20)  {
            SetupPurpleAndGreenPalette();
            currentBlending = LINEARBLEND;
        }
        if( secondHand == 25)  {
            SetupTotallyRandomPalette();
            currentBlending = LINEARBLEND;
        }
        if( secondHand == 30)  {
            SetupBlackAndWhiteStripedPalette();
            currentBlending = NOBLEND;
        }
        if( secondHand == 35)  {
            SetupBlackAndWhiteStripedPalette();
            currentBlending = LINEARBLEND;
        }
        if( secondHand == 40)  {
            currentPalette = CloudColors_p;
            currentBlending = LINEARBLEND;
        }
        if( secondHand == 45)  {
            currentPalette = PartyColors_p;
            currentBlending = LINEARBLEND;
        }
        if( secondHand == 50)  {
            currentPalette = myRedWhiteBluePalette_p;
            currentBlending = NOBLEND;
        }
        if( secondHand == 55)  {
            currentPalette = myRedWhiteBluePalette_p;
            currentBlending = LINEARBLEND;
        }
    }
}

void SetupTotallyRandomPalette()
{
    for( int i = 0; i < 16; i++) {
        currentPalette[i] = CHSV( random8(), 255, random8());
    }
}

void SetupBlackAndWhiteStripedPalette()
{
    fill_solid( currentPalette, 16, CRGB::Black);
    currentPalette[0] = CRGB::White;
    currentPalette[4] = CRGB::White;
    currentPalette[8] = CRGB::White;
    currentPalette[12] = CRGB::White;

}

void SetupPurpleAndGreenPalette()
{
    CRGB purple = CHSV( HUE_PURPLE, 255, 255);
    CRGB green  = CHSV( HUE_GREEN, 255, 255);
    CRGB black  = CRGB::Black;

    currentPalette = CRGBPalette16(
                         green,  green,  black,  black,
                         purple, purple, black,  black,
                         green,  green,  black,  black,
                         purple, purple, black,  black );
}


const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM =
{
    CRGB::Red,
    CRGB::Gray, 
    CRGB::Blue,
    CRGB::Black,

    CRGB::Red,
    CRGB::Gray,
    CRGB::Blue,
    CRGB::Black,

    CRGB::Red,
    CRGB::Red,
    CRGB::Gray,
    CRGB::Gray,
    CRGB::Blue,
    CRGB::Blue,
    CRGB::Black,
    CRGB::Black
};

