#define F_CPU 480000

#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

uint8_t occount = 0;
static uint8_t s=0xaa,a=0;

static uint8_t rnd(void) {
    s^=s<<3;
    s^=s>>5;
    s^=a++>>2;
    return s;
}

ISR(TIM0_OVF_vect) {
    ++occount;
}

int main(void) {
    uint8_t power = 0xff;
    uint8_t cap = 40;
    int8_t direction = 1;
    int8_t hold = -1;
    set_sleep_mode(SLEEP_MODE_IDLE);
    PORTB = 0xff;
    DDRB |= (1 << PB1);
    TCCR0B |= (1<<CS01);
    TCCR0A |= (1 << WGM01) | (1 << WGM00);
    TCCR0A |= (1 << COM0B1) | (1 << COM0B0);
    TIMSK0 |= (1 << TOIE0);
    ADCSRA &= ~(1<<ADEN);
    while (PINB & (1<<PB3)) {
        ++s;
    }
    sei();
    while(1) {
        if (occount >= cap) {
            cli();
            if (power == 0xff || power <= 0x70) {
                direction = (int8_t)(direction * -1);
                hold = (int8_t)(hold * direction);
                cap = (uint8_t)(rnd() % 112);
            }
            power += direction;
            if (hold == 1)
                OCR0B = power;
            occount = 0;
            sei();
        }
        sleep_mode();
    }
}
