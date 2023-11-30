#include <avr/io.h>
#include <stdint.h>
#include <stdbool.h>
#include <avr/interrupt.h>

enum {
  FOSC          = 16000000,           // Microcontroller frequency
  VREF          = 5000,               // Reference voltage mV
  ADC_BIT       = 1024,               //
  LOW_CHARGE    = 553,                // ADC parameter equal to 2.7V
  HIGH_CHARGE   = 840,                // ADC parameter equal to 4.2V
};

enum adcChan {
  ADC0,
  ADC1,
} adcState;                           // ADC channel current state

typedef struct battery_voltage {
  uint16_t vMeas_1;                   // First measurment
  uint16_t vMeas_2;                   // Second measurment
  uint16_t vTotal;                    // Voltage calculations result
  bool flag;                          // Measurment status (true/false)
} battery; 

static battery bt1 = {0};

static inline void portInit();
static inline void adcInit();
static inline void adc_chSelect
  (enum adcChan channel);

static inline void batVoltage
  (enum adcChan channel, battery *b);

static inline void ledControl
  (battery *b);

int main(void)
{
  portInit();
  adcInit();

  volatile enum adcChan 
    adcState = ADC0;
  bt1.flag = false;
//  PORTB |= (1<<PORT2);                // Turn on the Red LED (5V)

  sei();

  while(1) {
    ledControl(&bt1);
  }
  return 0;
}

static inline void portInit() {
  DDRC &= ~(1<<DDC0);                 // Set the ADC0 port to read
  DDRC &= ~(1<<DDC1);                 // Set the ADC1 port to read
  DDRB |= (1<<DDB0);                  // Set the LED ports to write
  DDRB |= (1<<DDB1);                  // ...
  DDRB |= (1<<DDB2);                  // Set the PowerSwitch to write
  PORTB &= ~(1<<PORTB0);              // Set 0V to PowerSwitch port (off)
  PORTB &= ~(1<<PORTB2);              // Set the RedLED ports to 0V (off)
  PORTB &= ~(1<<PORTB1);              // Set the GreenLED ports to 0V (off)
}

static inline void adcInit() {
  ADCSRA |= (1<<ADEN);                // Turn the ADC on  
  ADCSRA |= (1<<ADATE);               // Auto triggering of the ADC is enabled
  ADCSRA |= (1<<ADPS2);               // Set the prescaler of freqency to 128 
  ADCSRA |= (1<<ADPS1);               // it will be 125 kHz for ATmega328P
  ADCSRA |= (1<<ADPS0);               // ...
  ADMUX &= ~(1<<REFS1);               // Set the 5 voltage reference from AVcc
  ADMUX |= (1<<REFS0);                // ...
  ADMUX &= ~(1<<ADLAR);               // right adjusten of the data presentation
  ADCSRA |= (1<<ADIE);                // ADC interrupt enable
  ADCSRA |= (1<<ADSC);                // Start the conversion
}

static inline void adc_chSelect(enum adcChan channel) {
  ADMUX &= ~(0xF);                    // Set the MUX bits to the 0
  switch(channel) {                   // Switch channel for measurment
    case ADC0:
      ADMUX |= 0x00;
      break;
    case ADC1:
      ADMUX |= 0x01;
      break;
  }
}

static inline void ledControl(battery *b) {
  battery bt = *b;
  if(bt.flag) {
    if(bt.vTotal>= HIGH_CHARGE) {
      PORTB &= ~(1<<PORT2);           // Turn off the Red LED (0V)
      PORTB |= (1<<PORT1);            // Turn on the Green Led (5V)
      PORTB &= ~(1<<PORTB0);          // Set 0V to PowerSwitch port (off)
    } else if
      (bt.vTotal<= HIGH_CHARGE) {
      PORTB |= (1<<PORTB0);           // Set 5V to PowerSwitch port (on)
      PORTB &= ~(1<<PORT1);           // Turn off the Green LED (0V)
      PORTB |= (1<<PORT2);            // Turn on the Red LED (5V)
    }
  } 
}

static inline void batVoltage         // Saving measurments to the variable
  (enum adcChan channel, battery *b) {
  switch(channel) {
    case ADC0:
      b->vMeas_1 = ADC;
      b->flag = false;                // While the measurment is not finished
      break;                          // flag will be false
    case ADC1:
      b->vMeas_2 = ADC;
      b->vTotal = b->vMeas_1 - 
                    b->vMeas_2;
      b->flag = true;                 // When the all measurments are finished
      break;                          // flag will be true
  }
}


ISR(ADC_vect) {
  if(adcState > ADC1) {
    adcState = ADC0;
  };
  adc_chSelect(adcState);             // Switch to the next channel
  batVoltage(adcState, &bt1);         // Making a battery measurment
  adcState++;                         // Set flag to the next channel
}


