#include <Arduino.h>
#include <FastLED.h>

#define LED_PIN 2
#define SMOKE_PIN 3
#define PUMP_PIN 4
#define RED_PIN 5
#define GREEN_PIN 6
#define TRIGGER_PIN 7
#define ECHO_PIN 8
#define BUTTON_PIN 9

#define ARMED_DELAY 6000
#define WARNING_DELAY 1000
#define FLASH_DELAY 200
#define FIRE_DELAY 50
#define SMOKE_ON_DELAY 1000
#define READ_DELAY 10

#define COLOR_ORDER GRB
#define CHIPSET WS2811
#define NUM_LEDS 82
#define POLLING_DELAY 200
#define FRAMES_PER_SECOND 60

#define COOLING 55
#define SPARKING 120

bool on_fire = false;
bool prev_button = false;
bool gReverseDirection = false;

CRGB leds[NUM_LEDS];
CRGBPalette16 gPal;


void Fire2012WithPalette(int);
void enFuego();

void setup(){
    Serial.begin(9600);
    digitalWrite(GREEN_PIN, HIGH);
    delay(500); // sanity delay

    pinMode(SMOKE_PIN, OUTPUT);
    pinMode(PUMP_PIN, OUTPUT);
    pinMode(RED_PIN, OUTPUT);
    pinMode(GREEN_PIN, OUTPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
    gPal = HeatColors_p;
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
    delay(500);
    FastLED.show();
}

void loop(){
    if(!digitalRead(BUTTON_PIN) && prev_button) {
        on_fire = true;
        enFuego();
    }
    prev_button = digitalRead(BUTTON_PIN);
    delay(READ_DELAY);
}

void Fire2012WithPalette(int count)
{
    for (int i=0; i < NUM_LEDS; i++) {
        int v_limit = 0;
        if (random(3) == 0) {
            long h = random(60);
            if (count == 255) {
                v_limit = 255;
            } else {
                v_limit = count - i;
                v_limit = v_limit < 0 ? 1 : v_limit;
            }
            long v = random(v_limit);
            leds[i] = CHSV(h,255,v);
        }
    }
}

void enFuego() {
    prev_button = false;
    int smoke_count = 0;
    int fire_count = 10; 
    bool smoke_state = false;
    int smoke_off_delay = 1000;
    // int fire_count = 0;
    digitalWrite(GREEN_PIN, LOW);
    digitalWrite(RED_PIN, HIGH);
    unsigned long current_time = millis();
    while (millis() - current_time < ARMED_DELAY) {
        if (!digitalRead(BUTTON_PIN) && prev_button) {
                on_fire = false;
                break;
        }
        prev_button = digitalRead(BUTTON_PIN);
        delay(READ_DELAY);
    }
    digitalWrite(PUMP_PIN, HIGH);
    unsigned long flash_time = millis();
    current_time = flash_time;
    while (on_fire && (millis() - current_time < ARMED_DELAY)) {
        if (!digitalRead(BUTTON_PIN)) {
                on_fire = false;
                break;
        }
        if (millis() - flash_time >= FLASH_DELAY) {
            digitalWrite(RED_PIN, !digitalRead(RED_PIN));
            flash_time = millis();
        }
        delay(READ_DELAY);
    }
    digitalWrite(RED_PIN, HIGH);
    current_time = millis();
    flash_time = current_time;
    while (on_fire) {
        if (!digitalRead(BUTTON_PIN)) {
                on_fire = false;
                break;
        }
        if (millis() - flash_time >= FIRE_DELAY) {
            random16_add_entropy(random());
            Fire2012WithPalette(fire_count);
            FastLED.show();
            fire_count = fire_count < 255 ? fire_count + 5 : fire_count;
            fire_count = fire_count > 255 ? 255 : fire_count;
            flash_time = millis(); 
        }

        if ((smoke_state && millis() - current_time >= SMOKE_ON_DELAY) || (!smoke_state && millis() - current_time >= smoke_off_delay)) {
            smoke_state = !smoke_state;
            digitalWrite(SMOKE_PIN, smoke_state);
            if ((smoke_count / 2) > 3) {
                smoke_off_delay = 5000;
            } else {
                smoke_off_delay = 1000;
            }
            smoke_count += 1;
            current_time = millis();
        }
        delay(READ_DELAY);
    }
    prev_button = false;
    digitalWrite(SMOKE_PIN, LOW);
    delay(500);
    digitalWrite(PUMP_PIN, LOW);
    digitalWrite(RED_PIN, LOW);
    digitalWrite(GREEN_PIN, HIGH);
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
}
