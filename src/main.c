#include <avr/io.h>
#include <stdint.h>
#include <stdbool.h>
#include <avr/interrupt.h>

enum {
  FOSC          = 16000000,           // Microcontroller frequency
  VREF          = 5000,               // Reference voltage mV
  ADC_BIT       = 1024,               //
  MAX_I         = 60,                 // Number of measurments (16 bit var/1024)
  LOW_CHARGE    = 535,                // ADC parameter equal to 2.7V
  HIGH_CHARGE   = 831,                // ADC parameter equal to 4.2V
};

enum ledList {
  GREEN,                              // The battery is full of charge
  RED,                                // The battry is low of charge
  BLUE,                               // Power-seurce is ON
} led;

enum portList {
  POWER         = 3,                  // MOSFET power-switch port
} port;

enum state {
  OFF,
  ON,
};

typedef struct {
  uint8_t num;
  uint8_t regConf;
} adc_chan;

typedef struct {
  adc_chan adc;
  uint16_t voltage;                   // First measurment
} battery; 

static battery bt1;                   // Initialisation of battery structure

static inline void portInit();
static inline void adcInit();

static inline void charging
  (bool state);

static inline void adc_chSelect       
  (uint8_t chanConfig);               // Init a selected ADC channel

static inline void mosfetPw
  (bool state);

static inline void srcPw_LED
  (bool state);

static inline uint16_t adc(); 

int main(void)
{
  portInit();
  adcInit();

  PORTB |= (1<<POWER);                // Set 5V to PowerSwitch port (on)

  while(1) {
  }
  return 0;
}

ISR(ADC_vect) {

  bt1.voltage = adc();                // Getting ADC measurment result
  uint16_t voltage = bt1.voltage;
 
  if(voltage < LOW_CHARGE) {
    mosfetPw(OFF);
    PORTB &= ~(1<<GREEN);             // Turn off the Green LED (0V)
    PORTB &= ~(1<<RED);               // Turn off the Red LED (0V)
  } else if(voltage >= HIGH_CHARGE)
  {
    charging(OFF);
  } else if((voltage < HIGH_CHARGE)
           & (voltage >= LOW_CHARGE)) 
  {
    charging(ON);
  };

  ADCSRA |= 1<<ADSC;                  // Start the ADC conversion
}

static inline void portInit() {
  DDRC &= ~(1<<DDC0);                 // Set the ADC0 port to read
  DDRC &= ~(1<<DDC1);                 // Set the ADC1 port to read
  DDRB |= (1<<GREEN);                 // Set the LED ports to write
  DDRB |= (1<<RED);                   // ...
  DDRB |= (1<<BLUE);                  // ...
  DDRB |= (1<<POWER);                 // Set the PowerSwitch to write
  PORTB &= ~(1<<RED);                 // Set the RedLED port to 0V (off)
  PORTB &= ~(1<<GREEN);               // Set the GreenLED port to 0V (off)
  PORTB &= ~(1<<BLUE);                // Set the BlueLED port to 0V (off)
}

static inline void adcInit() {
  ADCSRA |= (1<<ADPS2);               // Set the prescaler of freqency to 128 
  ADCSRA |= (1<<ADPS1);               // it will be 125 kHz for ATmega328P
  ADCSRA |= (1<<ADPS0);               // ...
  ADMUX |= (1<<REFS1);                // Set the internal 1.1V voltage reference
  ADMUX |= (1<<REFS0);                // ...
  ADMUX &= ~(1<<ADLAR);               // Right adjusten of the data presentation

  bt1.adc.num = 0;                    // Init the first ADC channel 
  bt1.adc.regConf = 0x00;             // ...
  adc_chSelect(bt1.adc.regConf);      // Turn on the ADC channel

  ADCSRA |= (1<<ADIE);                // ADC interrupt enable
  ADCSRA |= (1<<ADEN);                // Turn the ADC on  
                                      //
  sei();                              // Set the Global Interrupts enable
                                      //
  ADCSRA |= (1<<ADSC);                // Start the conversion
}

static inline uint16_t adc() {
  uint8_t adcLow = ADCL;
  return ADCH << 8 | adcLow; 
}

static inline void adc_chSelect
  (uint8_t chanConfig) {

  uint8_t set = chanConfig;

  ADMUX &= ~(0xF);                    // Set the MUX bits to the 0
  ADMUX |= set;                       // Switch channel for measurment
}

static inline void mosfetPw
  (bool state) {
  if(state) {
    PORTB |= (1<<POWER);              // Turn on the power-switch port (5V)
  } else {
    PORTB &= ~(1<<POWER);             // Turn off the power-switch port (0V)
  };
}

static inline void srcPw_LED
  (bool state) {
  if(state) {
    PORTB |= (1<<BLUE);               // Set the BlueLED port to 5V (on)
  } else {
    PORTB &= ~(1<<BLUE);              // Set the BlueLED port to 0V (off)
  };
}

static inline void charging
  (bool state) {
    srcPw_LED(ON);
  if(state) {
    mosfetPw(ON);  
    PORTB &= ~(1<<GREEN);             // Turn off the Green LED (0V)
    PORTB |= (1<<RED);                // Turn on the Red LED (5V)
  } else {
    mosfetPw(OFF);
    PORTB &= ~(1<<RED);               // Turn off the Red LED (0V)
    PORTB |= (1<<GREEN);              // Turn on the Green Led (5V)
  };
}
