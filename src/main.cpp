#include <Arduino.h>

const double PERIOD = 0.016;

const int NUMBER_LEDS = 4;
const uint8_t PORT_B[] = {PB2, PB4, PB5, PB3};

const int MIN_CHANGE = 3;
int previousPotValue;
int currentPotValue;

volatile uint8_t currentLedIndex = 0;

void setup() {
  // define LED array.
  for (int i = 0; i < NUMBER_LEDS; i++)
  {
    // Define as output pin
    DDRB |= (1 << PORT_B[i]);
    // Initialize pin to off
    PORTB &= ~(1 << PORT_B[i]);
  }

  // A5 (PC5) will be the potentiometer's analog pin.
  DDRC &= ~(1 << PC5);
  // range of 0 up to and including 1023 (1024 values).
  currentPotValue = analogRead(PC5);
  previousPotValue = currentPotValue;

  unsigned int delayInMs = map(currentPotValue, 0, 1023, 500, 50);
  unsigned int numberOfClockTicks = delayInMs / PERIOD;

  // Disble interrupts
  cli();
  // Turn off whatever else was turned on in TCCR1A
  TCCR1A = 0x00;
  // Configrure through TCCR1B for CTC1 and 256 prescaler.
  TCCR1B = 0x00 | (1 << WGM12) | (1 << CS12);
  // Set compare value for OCR1A for CTC mode
  OCR1A = numberOfClockTicks;
  // Enable OCA match interrupt
  TIMSK1 = 0 | (1 << OCIE1A);

  // Re-enable interrupts
  sei();

  // Reset timer to 0
  TCNT1 = 0x00;
}

ISR(TIMER1_COMPA_vect)
{
  PORTB ^= (1 << PORT_B[currentLedIndex]);

  // Toggling the current led off means it went from on to off
  // hence we can set the index to the next led in the array.
  if (PORTB == 0)
  {
    // Taking the next index mod 4 keeping our index a small integer
    // and wrapping around as needed.
    currentLedIndex = (currentLedIndex + 1) % NUMBER_LEDS;
  }
}

void loop() {
  currentPotValue = analogRead(PC5);

  if (abs(currentPotValue - previousPotValue) >= MIN_CHANGE)
  {
    unsigned int delayInMs = map(currentPotValue, 0, 1023, 500, 50);
    unsigned int numberOfClockTicks = delayInMs / PERIOD;
    
    cli();
    OCR1A = numberOfClockTicks;
    sei();
    
    previousPotValue = currentPotValue;
  }
}
