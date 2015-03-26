#include <stdint.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

// Arming inputs
#define ARM_DDR DDRC
#define ARM_PORT PORTC
#define ARM_PINR PINC
#define ARM1_PIN 1
#define ARM2_PIN 2
#define ARM_BUTTON 3
#define ARM1_bm (1<<ARM1_PIN)
#define ARM2_bm (1<<ARM2_PIN)
#define ARM_BUTTON_bm (1<<ARM_BUTTON)
#define ARM_bm	(ARM1_bm|ARM2_bm|ARM_BUTTON_bm)

// Input voltage sense
#define VINSNS_DDR	DDRC
#define VINSNS_PIN	0
#define VINSNS_ADC_CH	0

// Ext Trigger
#define TRIG_DDR	DDRD
#define TRIG_PINR	PIND
#define TRIG1_PIN	2
#define TRIG2_PIN	3

// LED
#define LED_DDR		DDRD
#define LED_PORT	PORTD
#define LED_RED_PIN	4
#define LED_GRN_PIN	5
#define LED_GRN_ON()	LED_PORT |= (1<<LED_GRN_PIN)
#define LED_GRN_OFF()	LED_PORT &= ~(1<<LED_GRN_PIN)
#define LED_RED_ON()	LED_PORT |= (1<<LED_RED_PIN)
#define LED_RED_OFF()	LED_PORT &= ~(1<<LED_RED_PIN)

// Load Switch
#define SW_DDR	DDRD
#define SW_PORT	PORTD
#define SW_PIN	6

#define F_CPU 1000000UL;

#define CUT_TIME 5000

typedef enum
{
    UNKNOWN,
    DISARMED,
    ARMED,
    FIRING,
    FIRED,
}state_t;

static uint32_t RTC_CUT = 6; // 6000=100 min RTC cut threshold
volatile uint32_t rtcCount = 0;
volatile state_t state;

/**
 * 
 */
int8_t initializeRTC(void)
{
    cli();
    TIMSK2 &= ~((1<<TOIE2)|(1<<OCIE2B)|(1<<OCIE2A));// Clear OCIE2x and TOIE2
    ASSR |= (1<<EXCLK)|(1<<AS2);// Set AS2
    TCCR2A = 0x00;
    TCCR2B = (1<<CS22)|(1<<CS21); // CLKDIV 128 (1s ticks)
    _delay_us(10);
    TIFR2 &= ~(1<<TOV2);// Clear IFs
    TIMSK2 |= (1<<TOIE2);// Reenable interrupts
    state = DISARMED;
    sei();
}

/**
 * 
 */
void init(void)
{
    VINSNS_DDR &= ~(1<<VINSNS_PIN);
    ARM_DDR &= ~((1<<ARM_BUTTON)|(1<<ARM1_PIN)|(1<<ARM2_PIN));
    ARM_PORT |= (1<<ARM1_PIN)|(1<<ARM2_PIN)|(ARM_BUTTON); // Pull-ups
    
    // Set up LEDs
    LED_DDR |= (1<<LED_GRN_PIN)|(1<<LED_RED_PIN);
    LED_GRN_OFF();
    LED_RED_OFF();
    
    // Set up switch
    SW_PORT &= ~(1<<SW_PIN);
    SW_DDR |= (1<<SW_PIN);
    
    initializeRTC();
}

ISR(TIMER2_OVF_vect)
{
    rtcCount++;
    if(rtcCount % 1)
	LED_GRN_ON();
    else
	LED_GRN_OFF();
	   
}

/**
 * 
 */
void setSwitch(uint8_t on)
{
    if(on)
    {
	SW_PORT |= (1<<SW_PIN);
    }
    else
    {
	SW_PORT &= ~(1<<SW_PIN);
    }
}

void cut(void)
{
    setSwitch(1);
    _delay_ms(CUT_TIME);
    setSwitch(0);
}

/**
 * 
 */
int main(void)
{
  init();
  int16_t armCount = 0;
  uint8_t arming;
  while(1)
  {
      arming = ARM_PINR & ARM_bm;
      if(~(arming & ARM1_bm) && ~(arming & ARM2_bm))
      {
	  armCount++;
	  if(armCount >= 10)
	  {
	      armCount = 10;
	      state = ARMED;
	      LED_RED_ON();
	  }
	  else
	  {
	      LED_RED_OFF();
	  }
      }
      else
      {
	  armCount--;
	  if(armCount < 0)
	      armCount = 0;
	  if(armCount == 0)
	  {
	      state = DISARMED;
	      LED_RED_OFF();
	  }
      }
      if((rtcCount > RTC_CUT) && (state == ARMED))
      {
	  state = FIRING;
	  cut();
	  rtcCount = 0;
	  state = FIRED;
	  LED_RED_OFF();
      }
  }
  return 0;  
}