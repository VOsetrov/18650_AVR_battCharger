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

typedef struct battery_voltage {
  enum adcChan {
    ADC0,                             // Channel status ADC0
    ADC1,                             // Channel status ADC1
  } chan;
  uint16_t vMeas_1;                   // First measurment
  uint16_t vMeas_2;                   // Second measurment
  uint16_t vTotal;                    // Voltage calculations result
  bool flag;                          // Measurment status (true/false)
} battery; 

static battery bt1 = {0};             // Initialisation of battery_voltage

static inline void portInit();
static inline void adcInit();
static inline bool pwMonitor
  (enum adcChan channel,
   uint16_t adcRes);                  // Power-source monitor

static inline void charging
  (bool state);

static inline void adc_chSelect       
  (enum adcChan channel);             // Init a selected ADC channel

static inline void batVoltage
  (battery *b, uint16_t result);

static inline void chargeCtrl 
  (battery *b, bool pw);

static inline void mosfetPw
  (bool state);

static inline void srcPw_LED
  (bool state);

static inline uint16_t adc(); 

int main(void)
{
  portInit();
  adcInit();

  bt1.flag = false;

  while(1) {
  }
  return 0;
}

ISR(ADC_vect) {
  uint16_t result = adc();            // Getting ADC measurment result

  bool power = pwMonitor
    (bt1.chan, result);               // Check the Power State 

  if(power) {
    batVoltage(&bt1, result);         // Saving the measurments
    chargeCtrl(&bt1, power);          // Perform charge control
    bt1.chan++;                       // Set flag to the next channel
    if(bt1.chan > ADC1) {
      bt1.chan = ADC0;
    };
    adc_chSelect(bt1.chan);           // Switch to the next channel
  } else {
    srcPw_LED(OFF);                   // Turn off the power LED
    bt1.chan = ADC0;
    adc_chSelect(bt1.chan);
    bt1.vMeas_1 = 0;
    bt1.vMeas_2 = 0;
    bt1.vTotal = 0;
    bt1.flag = false;
  };

  ADCSRA |= 1<<ADSC;                  // Start the ADC conversion
}

static inline void portInit() {
  DDRC &= ~(1<<DDC0);                 // Set the ADC0 port to read
  DDRC &= ~(1<<DDC1);                 // Set the ADC1 port to read
  DDRB |= (1<<DDB0);                  // Set the LED ports to write
  DDRB |= (1<<DDB1);                  // ...
  DDRB |= (1<<DDB2);                  // ...
  DDRB |= (1<<POWER);                 // Set the PowerSwitch to write
  PORTB |= (1<<POWER);                // Set 5V to PowerSwitch port (on)
  PORTB &= ~(1<<RED);                 // Set the RedLED port to 0V (off)
  PORTB &= ~(1<<GREEN);               // Set the GreenLED port to 0V (off)
  PORTB &= ~(1<<BLUE);                // Set the BlueLED port to 0V (off)
}

static inline void adcInit() {
  ADCSRA |= (1<<ADPS2);               // Set the prescaler of freqency to 128 
  ADCSRA |= (1<<ADPS1);               // it will be 125 kHz for ATmega328P
  ADCSRA |= (1<<ADPS0);               // ...
  ADMUX &= ~(1<<REFS1);               // Set the 5 voltage reference from AVcc
  ADMUX |= (1<<REFS0);                // ...
  ADMUX &= ~(1<<ADLAR);               // Right adjusten of the data presentation

  bt1.chan = ADC0;                    // Init the first ADC channel 
  adc_chSelect(bt1.chan);             // Turn on the ADC channel

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
  (enum adcChan channel) {
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

static inline void chargeCtrl
  (battery *b, bool pw) {

  battery bt = *b;

  if(pw) {
    srcPw_LED(ON);
    if(bt.flag) {
      if(bt.vTotal >= HIGH_CHARGE) {
        charging(OFF);
      } else if
        (bt.vTotal <= HIGH_CHARGE) {
        charging(ON);
      };
    }; 
  } else {
    srcPw_LED(OFF);
    PORTB &= ~(1<<RED);             // Turn off the Red LED (0V)
    PORTB &= ~(1<<GREEN);           // Turn off the Green Led (5V)
  };
}

static inline void batVoltage 
  (battery *b,
   uint16_t result) {

  enum adcChan channel = b->chan;

  switch(channel) {
    case ADC0:
      b->vMeas_1 = result;            // Saving the first measurment
      b->flag = false;                // Battery voltage measurment not 
      break;                          // finished yet.
    case ADC1:
      b->vMeas_2 = result;            // Saving the second measurment
      b->vTotal = b->vMeas_1 - 
                    b->vMeas_2;       // Finding the current battery voltage
      b->flag = true;                 // All measurments was finished
      break;                          // ...
  }
}

static inline bool pwMonitor
  (enum adcChan channel, uint16_t adcRes) {
  if(channel == 0 && adcRes == 0) {
    return false;
  } else { return true; };
}


