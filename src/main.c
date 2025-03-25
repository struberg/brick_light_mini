#include <avr/io.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <string.h>

#include "avr_common/strub_common.h"
#include "avr_common/pt.h"



/**
 * @brief small lighting solution for 4 LEDs
 * 
 * L1   PA2 pin5
 * L2   PA1 pin4
 * L3   PA7 pin3
 * L4   PA6 pin2
 * Btn  PA3 pin7
 */

#define BTN_PORT PORTA
#define BTN_PIN PIN3_bm
#define BTN_PIN_CTRL PORTA_PIN3CTRL

#define BTN_SHORT_TIME_MIN 0x05
#define BTN_SHORT_TIME_MAX 0x40
#define BTN_LONG_TIME_MON  0x0120

#define LED_PORT PORTA
#define LED_1_PIN PIN2_bm
#define LED_2_PIN PIN1_bm
#define LED_3_PIN PIN7_bm
#define LED_4_PIN PIN6_bm

#define TASK_TIMEOUT_bm    0x02
#define TASK_INPUT_bm  0x04
#define TASK_ADC_bm    0x08

#define SHUTDODWN_TIME 0xFF00;

volatile uint8_t taskTriggered = 0; 



void setup_cpu(void) {
    // auf prescaler /16 stellen
    // damit rennt die CPU dann effektiv auf 1,33 MHz
    // attention, this needs the CCP (Configuration Change Protection) procedure!

    CCP = CCP_IOREG_gc;
    CLKCTRL.MCLKCTRLB = (0x03 << 1) | 0x01; // clk / 16

    CLKCTRL.MCLKCTRLA = 0x00;
}


void setup_task_timer(void) {
    TASK_TIMER.CCMP = TASK_TIMER_OVERFLOW; /* Compare or Capture */

    TASK_TIMER.CTRLB = 0 | TCB_CNTMODE_INT_gc; // stick at periodic interrupt mode

    TASK_TIMER.CTRLA = TCB_CLKSEL_CLKDIV1_gc  /* CLK_PER (without Prescaler) */
                    | 1 << TCB_ENABLE_bp   /* Enable: enabled */
                    | 0 << TCB_RUNSTDBY_bp /* Run Standby: disabled */
                    | 0 << TCB_SYNCUPD_bp; /* Synchronize Update: disabled */


    // enable Overflow Interrupt. TOP is CCMP
    TASK_TIMER.INTCTRL = TCB_CAPT_bm;
}

volatile uint16_t timers[3] = {0,};

/**
 * @brief TimerB0 overflow
 * 
 * Setzt die 'taskTriggered' bitmask auf FF.
 * Jeder Task kann dann selber checken ob er schon dran war 
 * indem er ein bit darin repräsentiert.
 */
ISR (TCB0_INT_vect) {
    for (uint8_t i=0; i< 3; i++) {
        if (timers[i] > 0) {
            timers[i]--;
        }
    }
    taskTriggered = 0xFF;

    // special handling in the new tinys. one needs to reset the int flags manually 
    TCB0.INTFLAGS = TCB_CAPT_bm;
}

/**
 * @brief wake up from power down
 */
ISR(PORTA_PORT_vect) {
    // clear any subsequent int flag on the pin

    BTN_PORT.INTFLAGS = 0xFF; // writing 1 will clear the int flags

    // disable the PIN interrupt
    BTN_PIN_CTRL &= ~0x03;
}

void setup_io(void) {
    // set leds to output

    LED_PORT.DIRSET = LED_1_PIN | LED_2_PIN | LED_3_PIN | LED_4_PIN;

    // Button input
    BTN_PORT.DIRCLR = BTN_PIN;
    BTN_PIN_CTRL = PORT_PULLUPEN_bm;
}

void setup_adc(void) {
    // setup the voltage reference
    // only used for measuring the VDD itself
    VREF.CTRLA = VREF_ADC0REFSEL_1V5_gc;
    VREF.CTRLB = VREF_ADC0REFEN_bm;

    // ADC itself
    ADC0.CTRLB = 0; // no accumulation

    ADC0.CTRLC = ADC_REFSEL_VDDREF_gc | ADC_PRESC_DIV256_gc;

    ADC0.CTRLD = ADC_INITDLY_DLY64_gc;

    ADC0.MUXPOS = ADC_MUXPOS_INTREF_gc;

    ADC0.CTRLA =  ADC_RESSEL_10BIT_gc | ADC_ENABLE_bm;
}

void adc_enable(void) {
    ADC0.CTRLA |= ADC_ENABLE_bm;
}

void adc_disable(void) {
    ADC0.CTRLA &= ~ADC_ENABLE_bm;
}

void adc_start(void) {
    ADC0.COMMAND |= ADC_STCONV_bm;
}

bool adc_finished(void) {
    return ADC0.INTFLAGS & ADC_RESRDY_bm;
}


void enable_pin_change_interrupt(void) {
    // disable the PIN interrupt
    BTN_PORT.INTFLAGS = 0xFF;
    BTN_PIN_CTRL = PORT_PULLUPEN_bm | PORT_ISC_BOTHEDGES_gc;
}

void disable_pin_change_interrupt(void) {
    // disable the PIN interrupt
    BTN_PIN_CTRL &= ~0x07;
}


void power_down(void) {
    LED_PORT.OUTCLR = LED_1_PIN | LED_2_PIN | LED_3_PIN | LED_4_PIN;

    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable();
    enable_pin_change_interrupt();
    sleep_cpu();
    sleep_disable();
    disable_pin_change_interrupt();
}


void executeCommand(uint8_t command) {
    // reset timeout;
    timers[0] = SHUTDODWN_TIME;

    switch (command) {
        case 1:
            LED_PORT.OUTTGL = LED_1_PIN;
            break;
        case 2:
            LED_PORT.OUTTGL = LED_2_PIN;
            break;
        case 3:
            LED_PORT.OUTTGL = LED_3_PIN;
            break;
        case 4:
            LED_PORT.OUTTGL = LED_4_PIN;
            break;
    
        default:
            break;
    }

    timers[1] = 0xFFFF;
}

bool buttonPressed(void) {
    return !(BTN_PORT.IN & BTN_PIN);
}

void task_input(void) {
    static uint16_t btnOnTime = 0;
    static uint16_t btnOffTime = 0;
    static uint8_t btnClicks = 1;
    
    if (taskTriggered & TASK_INPUT_bm) {
        taskTriggered &= ~TASK_INPUT_bm;

        if (buttonPressed()) {
            btnOnTime++;

            if (btnOffTime > 0) {
                if (btnOffTime < BTN_SHORT_TIME_MAX) {
                    btnClicks++;
                }
                else {
                    btnClicks = 1;
                }

                btnOffTime = 0;
            }
        }
        else {
            if (btnOnTime > 0 && btnOffTime > BTN_SHORT_TIME_MAX) {
                executeCommand(btnClicks);
                btnOnTime = 0;
                btnOffTime = 0;
            }
            if (btnOffTime < 0xFFFF) {
                btnOffTime++;
            }
        }

    }
}

struct t_adc_state {
    struct pt pt;
    uint8_t blink;
    uint16_t timer;
} adc_state = {0,};


PT_THREAD(task_adc(void)) {
    PT_BEGIN(&adc_state.pt);

    if (taskTriggered & TASK_ADC_bm) {
        // only once per timer interrupt
        taskTriggered &= ~TASK_ADC_bm;
        if (adc_state.timer > 0) {
            adc_state.timer--;
            PT_YIELD(&adc_state.pt);
        }            
        adc_state.timer = 50;

        adc_enable();
        adc_start();


        PT_YIELD_UNTIL(&adc_state.pt, adc_finished());

        uint16_t val = ADC0.RES;
        adc_disable();

        if (val >= 500) {
            // Vdd < 3V now
            // let's start a short blink, then turn off
            LED_PORT.OUTSET = LED_1_PIN | LED_2_PIN | LED_3_PIN | LED_4_PIN;
            timers[1] = 50;
            PT_YIELD_UNTIL(&adc_state.pt, timers[1] == 0);

            power_down();
        }
    }
    
    PT_END(&adc_state.pt);
}

void task_timeout(void) {
    if (taskTriggered & TASK_TIMEOUT_bm) {
        // only once per timer interrupt
        taskTriggered &= ~TASK_TIMEOUT_bm;

        if (timers[0] == 0) {
            power_down();
            timers[0] = SHUTDODWN_TIME;
        }
    }
}

int main(void) {
    setup_cpu();
    setup_task_timer();

    setup_io();
    setup_adc();


    timers[0] = SHUTDODWN_TIME;

    sei();

    while(1) {
        task_adc();
        task_input();
        task_timeout();
    }

    return 1;
}