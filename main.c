#define F_CPU 480000

#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

register uint8_t occount asm("r2");
register uint8_t s asm("r3");
register uint8_t a asm("r4");

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
    occount = 0;
    set_sleep_mode(SLEEP_MODE_IDLE);
    PORTB = 0xff;  // enable pullup resistors for powersaving
    DDRB |= (1 << PB1);  // set pwm port as output
    TCCR0B |= (1<<CS01);  // timer prescaler == 8
    TCCR0A |= (1 << WGM01) | (1 << WGM00); // set pwm to fast_pwm mode
    TCCR0A |= (1 << COM0B1) | (1 << COM0B0); // set pwm to inverting mode
    TIMSK0 |= (1 << TOIE0); // enable timer interrupt
    ADCSRA &= ~(1<<ADEN);  //disable ADC
    while (PINB & (1<<PB3)) {
        ++s;  // increment s until button is pressed
    }
    sei();
    while(1) {
        if (occount >= cap) { // cap sets the delay for updating pwm value
            cli(); // we don't need interrupts while updating pwm
            if (power == 0xff || power <= 0x70) { // change fade direction after reaching max\min values
                direction = (int8_t)(direction * -1);
                hold = (int8_t)(hold * direction); // skip every two dimming phases
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
