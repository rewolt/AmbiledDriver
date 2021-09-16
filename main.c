// Ambiled Driver - for WS2813 RGB LEDs
// You can use different clock, but make sure to recalculate number of nops in WS_SendZeroBit, WS_SendOneBit

//////////////////////////////////////////////////////////////////////////
// ATMEGA 8 CONFIG
#define F_CPU			16000000UL
#define BAUDRATE		38400UL
#define WS_LEDS_NUMBER	30
#define WS_DDR			DDRC
#define WS_PORT			PORTC
#define WS_PIN			PORTC5
#define INFO_LED_DDR	DDRB
#define INFO_LED_PORT	PORTB
#define INFO_LED_PIN	PORTB0

//////////////////////////////////////////////////////////////////////////
// ATMEGA 16 CONFIG
//#define F_CPU			16000000UL
//#define BAUDRATE		38400UL
//#define WS_LEDS_NUMBER	30
//#define WS_DDR			DDRA
//#define WS_PORT			PORTA
//#define WS_PIN			PORTA1
//#define INFO_LED_DDR	DDRB
//#define INFO_LED_PORT	PORTB
//#define INFO_LED_PIN	PORTB1



#define MYUBRR ((F_CPU / (BAUDRATE * 16)) - 1)
#define waitNops(czas) for (unsigned char i = 0; i < (czas); i++) asm("nop");
#include <avr/io.h>
#include <util/delay.h>
#include <avr/wdt.h>

struct RGB
{
	unsigned char red;
	unsigned char green;
	unsigned char blue;
};

void IndicatorLed_Init();
void WS_Init();
void USART_Init (unsigned int ubrr);
unsigned char USART_Receive();
void USART_Send(unsigned char message);
void WS_SendZeroBit();
void WS_SendOneBit();
void WS_SendReset();
void WS_SendColorPart(unsigned char color);
void WS_SendRGBcolor(unsigned char red, unsigned char green, unsigned char blue);
void WS_SetLineColor(struct RGB *ledColor, unsigned char blue);
void USART_SendString(char *text, unsigned char n);
void LedIndicatorOn();
void LedIndicatorOff();
void IndicatorLed_Blink(unsigned char times, unsigned short delay);

int main(void)
{
	IndicatorLed_Init();
	WS_Init();
	USART_Init(MYUBRR);
	IndicatorLed_Blink(1,1000);
	char *ack = "OK\n";
	
	struct RGB leds[WS_LEDS_NUMBER];
	for (unsigned char i = 0; i < WS_LEDS_NUMBER; i++)
	{
		WS_SendRGBcolor(0, 0, 0);
	}
	
	wdt_enable(WDTO_2S); // watchdog enabled - 2sec
	
	while(1)
	{
		USART_SendString(ack, 3);
		
		for (unsigned char i = 0; i < WS_LEDS_NUMBER; i++)
		{
			leds[i].red = USART_Receive();
			leds[i].green = USART_Receive();
			leds[i].blue = USART_Receive();
		}
		
		for (unsigned char i = 0; i < WS_LEDS_NUMBER; i++)
		{
			WS_SendRGBcolor(leds[i].red, leds[i].green, leds[i].blue);
		}
		WS_SendReset();
		
		wdt_reset(); // watchdog reset
	}
}

void USART_Init (unsigned int ubrr)
{
	UBRRH = (unsigned char)(ubrr>>8);
	UBRRL = (unsigned char)ubrr;
	// enable receiver, transmitter
	UCSRB = (1<<RXEN)|(1<<TXEN);
	// enable UCSRC register write, set parity check None, set 2 bits Stop, set 8-bit data
	UCSRC = (1<<URSEL)|(1<<USBS)|(3<<UCSZ0);
}

unsigned char USART_Receive()
{
	while ( !(UCSRA & (1<<RXC)) )
		;
	return UDR;
}

void USART_Send(unsigned char data)
{
	while ( !( UCSRA & (1<<UDRE)) )
		;
	UDR = data;
}

void USART_SendString(char *text, unsigned char strlen) 
{
	unsigned char i;
	for (i = 0; i < strlen; i++) {
		USART_Send(*(text++));
	}
}

void WS_Init()
{
	WS_DDR |= (1<<WS_PIN);
}

void IndicatorLed_Init()
{
	INFO_LED_DDR |= (1<<INFO_LED_PIN);
}

void WS_SendZeroBit()
{
	WS_PORT |= (1<<WS_PIN); //set high on pin PD0 220ns~380ns
	waitNops(4);
	WS_PORT &= ~(1<<WS_PIN); //set low on pin PD0 580ns~1600ns
	waitNops(10);
}

void WS_SendOneBit()
{
	WS_PORT |= (1<<WS_PIN); //set high on pin PD0 580ns~1600ns
	waitNops(10);
	WS_PORT &= ~(1<<WS_PIN); //set low on pin PD0 220ns~420ns
	waitNops(4);
}

void WS_SendReset()
{
	WS_PORT &= ~(1<<WS_PIN); //set low on pin PD0
	_delay_us(300);
}

void WS_SendColorPart(unsigned char color)
{
	unsigned char mask = 0b10000000;
	
	for(unsigned char i = 0; i < 8; i++)
	{
		if(color & mask)
			WS_SendOneBit();
		else
			WS_SendZeroBit();
		
		mask = mask >> 1;
	}
}

// Send RGB color. Colors are swapped internally like documentation for WS2813 says.
void WS_SendRGBcolor(unsigned char red, unsigned char green, unsigned char blue)
{
	WS_SendColorPart(green);
	WS_SendColorPart(red);
	WS_SendColorPart(blue);
}

void WS_SetLineColor(struct RGB *ledColor, unsigned char ledsNumber)
{
	for (unsigned char i = 0; i < ledsNumber; i++)
	{
		WS_SendRGBcolor(ledColor->red, ledColor->green, ledColor->blue);
	}
	WS_SendReset();
}

void LedIndicatorOn()
{
	INFO_LED_PORT &= ~(1<<INFO_LED_PIN);
}

void LedIndicatorOff()
{
	INFO_LED_PORT |= (1<<INFO_LED_PIN);
}

void IndicatorLed_Blink(unsigned char times, unsigned short delay)
{
	for(unsigned char i = 0; i<times;i++)
	{
		LedIndicatorOn();
		_delay_ms(1000);
		LedIndicatorOff();
		_delay_ms(1000);
	}
}