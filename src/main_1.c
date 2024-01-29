#include <avr/io.h>
#include <avr/interrupt.h>

#define MAX_CHARGE_VOLTAGE 420 // Максимальное напряжение заряда для 18650 (4.2В * 100)
#define LED_RED     (1<<PB0)
#define LED_GREEN   (1<<PB1)
#define LED_BLUE    (1<<PB2)
#define MOSFET      (1<<PB3)

volatile uint16_t adc_result0 = 0;
volatile uint16_t adc_result1 = 0;
volatile uint8_t channel = 0;

void ADC_init() {
    ADMUX = (1<<REFS0); // Использовать AVcc в качестве опорного напряжения
    ADCSRA = (1<<ADEN) | (1<<ADIE) | (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0); // Включить АЦП, прерывания и установить делитель частоты
}

void ADC_start_conversion(uint8_t channel) {
    ADMUX = (ADMUX & 0xF8) | channel; // Выбрать канал ADC
    ADCSRA |= (1<<ADSC); // Начать преобразование
}

ISR(ADC_vect) {
    if (channel == 0) {
        adc_result0 = ADC;
        channel = 1;
    } else {
        adc_result1 = ADC;
        channel = 0;
    }

    if (adc_result0 != 0) {
        PORTB |= LED_BLUE; // Включить синий светодиод, если питание подключено
    }

    if (channel == 1) {
        ADC_start_conversion(1);
    } else {
        ADC_start_conversion(0);
        if (adc_result0 != 0) {
            int16_t charge_diff = adc_result0 - adc_result1;
            if (charge_diff < MAX_CHARGE_VOLTAGE) {
                PORTB |= MOSFET; // Продолжить зарядку
                PORTB |= LED_RED; // Включить красный светодиод
                PORTB &= ~LED_GREEN; // Выключить зеленый светодиод
            } else {
                PORTB &= ~MOSFET; // Отключить зарядку
                PORTB |= LED_GREEN; // Включить зеленый светодиод
                PORTB &= ~LED_RED; // Выключить красный светодиод
            }
        }
    }
}

int main() {
    DDRB = LED_RED | LED_GREEN | LED_BLUE | MOSFET; // Настроить порты на вывод
    PORTB |= MOSFET;
    ADC_init();
    sei(); // Разрешить глобальные прерывания
    ADC_start_conversion(0);

    while (1) {
        // Основной цикл программы
    }
}
