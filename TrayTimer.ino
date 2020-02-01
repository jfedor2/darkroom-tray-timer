#include <avr/sleep.h>

#define BUTTON_PIN 0
#define BUTTON_INT_MASK PCINT0
#define BUZZER_PIN 1
#define NOTE 262
#define LED_PIN 2

// Using core from https://code.google.com/archive/p/arduino-tiny/
// "ATtiny85 @ 1 MHz (internal oscillator; BOD disabled)"

ISR (PCINT0_vect) {}

void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  ADCSRA &= ~(1 << ADEN); // disable ADC
  GIMSK |= (1 << PCIE); // enable pin change interrupts
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
}

int current_index = 0;

// sequences are alternating beep and silence durations in milliseconds
// sequence ends with 0
long sequences[] = {
  200, 55800, 200, 800, 200, 800, 200, 800, 1000, 0,
  200, 100, 200, 8500, 1000, 0,
  200, 100, 200, 100, 200, 55200, 200, 800, 200, 800, 200, 800, 1000, 0,
  200, 100, 200, 100, 200, 100, 200, 115700, 1000, 100, 1000, 100, 1000, 0,
};

int sequences_length = sizeof(sequences) / sizeof(sequences[0]);

void loop() {
  PCMSK |= 1 << BUTTON_INT_MASK; // set interrupt mask to wake on button press
  // go to sleep
  sleep_mode();
  // ... and we're awake

  PCMSK &= ~(1 << BUTTON_INT_MASK); // ignore interrupts for now

  bool beep = true;

  long sequence_start_time = millis();

  while (sequences[current_index] != 0) {
    long start_time = millis();
    if (beep) {
      tone(BUZZER_PIN, NOTE);
      digitalWrite(LED_PIN, HIGH);
    } else {
      noTone(BUZZER_PIN);
      digitalWrite(LED_PIN, LOW);
    }
    beep = !beep;
    while (millis() - start_time < sequences[current_index]) {
      if (millis() - sequence_start_time > 1000L) {
        int button = digitalRead(BUTTON_PIN);
        if (button == LOW) {
          noTone(BUZZER_PIN);
          digitalWrite(LED_PIN, LOW);
          while (sequences[++current_index] != 0) {
          }
          sequence_start_time = millis();
          beep = true;
          break;
        }
      }
    }
    current_index++;
    current_index %= sequences_length;
  }
  noTone(BUZZER_PIN);
  digitalWrite(LED_PIN, LOW);
  current_index++;
  current_index %= sequences_length;
}
