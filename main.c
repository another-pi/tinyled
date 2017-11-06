// #define F_CPU 480000 // already defined in makefile

#include <avr/interrupt.h>
#include <avr/sleep.h>

#define MAX(a,b) (((a)>(b))?(a):(b))
#define MIN(a,b) (((a)<(b))?(a):(b))

#define PWM_BRIGHT 0x80
#define PWM_DIM 0xff
#define FLICKER_START 0xf8
#define FLICKER_STOP  0xa0
#define SPEED_D 0x0f
#define SPEED_MIN 0x1e
#define SPEED_MAX 0xb4

register uint8_t occount asm("r2");
register uint8_t s asm("r3"); // lcg var
register uint8_t a asm("r4"); // lcg var

static uint8_t rand(void) {
    s ^= s << 3;
    s ^= s >> 5;
    s ^= a++ >> 2;
    return s;
}

ISR(TIM0_OVF_vect) {
    ++occount;
}

int main(void) {
    uint8_t power = 0xff; // inital state of pwm leds = dark
    uint8_t cap = 40; // initial updimming speed = moderate
    int8_t direction = 1; // inital dimming direction = raise
    int8_t hold = -1; // (weird math) skip two sequential /\ phases, invert to skip \/
    int8_t accel = 0; // dimming accelleration dispersion value
    int8_t flicker = 0; 
    occount = 0;

    set_sleep_mode(SLEEP_MODE_IDLE);  // only disable cpu and flash clk while sleeping
    PORTB = 0xff;  // enable pullup resistors for powersaving
    DDRB |= (1 << PB1);  // set pwm port as output
    TCCR0B |= (1 << CS01);  // timer prescaler == 8
    TCCR0A |= (1 << WGM01)  | (1 << WGM00); // set pwm to fast_pwm mode
    TCCR0A |= (1 << COM0B1) | (1 << COM0B0); // set pwm to inverting mode
    TIMSK0 |= (1 << TOIE0); // enable timer interrupt

    s = 173;  // seed(chosen by fair dice roll. guaranteed to be random)

    ADCSRA &= ~(1 << ADEN);  //disable ADC
    
    sei();
    while(1) {
        if (occount >= cap) { // cap sets the delay for updating pwm value(dimming speed)
            cli(); // we don't need interrupts while updating pwm
            if (power == PWM_DIM || power <= PWM_BRIGHT) { // change fade direction after reaching max\min values
                direction = (int8_t)(direction * -1);
                hold = (int8_t)(hold * direction); // skip every two dimming phases
                cap = (uint8_t)(MAX(rand() % SPEED_MAX, SPEED_MIN));
            }
            power += direction; // update base value for PWM
            if (hold == 1) { // skip every two phases(adds delay between dimming cycles)
                flicker = (int8_t)(rand() & 0x3) - 1;
                accel = (rand() % SPEED_D) * flicker;
                cap = MIN(cap+accel, SPEED_MAX); // accellerate dimming by random amount
                if (power > FLICKER_STOP && power < FLICKER_START)
                    power += flicker; // only flicker when brightness is low
                OCR0B = power; // push result to PWM
            }
            occount = 0;
            sei();
        }
        sleep_mode(); // sleep until timer interrupt is triggered
    }
}
