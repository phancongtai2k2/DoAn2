#define F_CPU 8000000UL			/* Define CPU Frequency e.g. here 8MHz */
#include <avr/io.h>			/* Include AVR std. library file */
#include <util/delay.h>			/* Include inbuilt defined Delay header file */
#include <string.h>
#include <stdio.h>

#define LCD_Data_Dir DDRC		/* Define LCD data port direction */
#define LCD_Command_Dir DDRD		/* Define LCD command port direction register */
#define LCD_Data_Port PORTC		/* Define LCD data port */
#define LCD_Command_Port PORTD		/* Define LCD data port */
#define RS PD6		/* Define Register Select (data/command reg.)pin */
#define RW PD5			/* Define Read/Write signal pin */
#define EN PD7			/* Define Enable signal pin */

#define degree_sysmbol 0xdf

void INIT(){
	DDRD |= 0xFF;
	PORTD |= 0xFF;
	
	DDRC |= 0xFF;
	PORTC |= 0xFF;
	
	DDRB |= 0xFF;
	PORTB |= 0xFF;

	DDRA = 0x00;
	PORTA = 0x00;

}

void LCD_Command(unsigned char cmnd)
{
	LCD_Data_Port= cmnd;
	LCD_Command_Port &= ~(1<<RS);	/* RS=0 command reg. */
	LCD_Command_Port &= ~(1<<RW);	/* RW=0 Write operation */
	LCD_Command_Port |= (1<<EN);	/* Enable pulse */
	_delay_us(1);
	LCD_Command_Port &= ~(1<<EN);
	_delay_ms(3);
}

void LCD_Char (unsigned char char_data)	/* LCD data write function */
{
	LCD_Data_Port= char_data;
	LCD_Command_Port |= (1<<RS);	/* RS=1 Data reg. */
	LCD_Command_Port &= ~(1<<RW);	/* RW=0 write operation */
	LCD_Command_Port |= (1<<EN);	/* Enable Pulse */
	_delay_us(1);
	LCD_Command_Port &= ~(1<<EN);
	_delay_ms(1);
}

void LCD_Init ()			/* LCD Initialize function */
{
	LCD_Command_Dir = 0xFF;		/* Make LCD command port direction as o/p */
	LCD_Data_Dir = 0xFF;		/* Make LCD data port direction as o/p */
	_delay_ms(20);			/* LCD Power ON delay always >15ms */
	
	LCD_Command (0x38);		/* Initialization of 16X2 LCD in 8bit mode */
	LCD_Command (0x0C);		/* Display ON Cursor OFF */
	LCD_Command (0x06);		/* Auto Increment cursor */
	LCD_Command (0x01);		/* Clear display */
	LCD_Command (0x80);		/* Cursor at home position */
}

void LCD_String (char *str)		/* Send string to LCD function */
{
	int i;
	for(i=0;str[i]!=0;i++)		/* Send each char of string till the NULL */
	{
		LCD_Char (str[i]);
	}
}

void ADC_Init(){
	DDRA = 0x00;	//   DDRA0.0 = 0     /* Make ADC port as input */
	ADCSRA = 0x87;          /* Enable ADC, with freq/128  */
	ADMUX = 0xC0;           /* Vref = 2.56, ADC channel: 0 */
}

int ADC_Read(char channel)
{
	int Ain,AinLow;
	
	ADMUX=ADMUX|(channel & 0x0f);	/* Set input channel to read */
	ADCSRA |= (1<<ADSC);		/* Start conversion */
	while((ADCSRA&(1<<ADIF))==0);	/* Monitor end of conversion interrupt */
	_delay_ms(1);
	AinLow = (int)ADCL;		/* Read lower byte*/
	Ain = (int)ADCH*256;		/* Read higher 2 bits and Multiply with weight */
	Ain = Ain + AinLow;				
	return(Ain);			/* Return digital value*/
}

void LCD_String_xy (char row, char pos, char *str)/* Send string to LCD with xy position */
{
	if (row == 0 && pos<16)
	LCD_Command((pos & 0x0F)|0x80);	/* Command of first row and required position<16 */
	else if (row == 1 && pos<16)
	LCD_Command((pos & 0x0F)|0xC0);	/* Command of first row and required position<16 */
	LCD_String(str);		/* Call LCD string function */
}

void LCD_Display_float(double value, char* String){
	int convert = value*100;
	int head = convert/100;
	int tail = convert%100;
	if(tail > 10)
	sprintf(String,"%d.%d",head,tail);
	else
	sprintf(String,"%d.0%d",head,tail);
}

int main()
{
	char Temperature[10];
	double voltage;
	double celsius;
	unsigned int ADC_value;
	unsigned int totalADC = 0;
	
	INIT();
	LCD_Init();                 /* initialize 16x2 LCD*/
	ADC_Init();                 /* initialize ADC*/
	
	LCD_String_xy(0,0,"Temperature:");
	
	while(1)
	{
		for(int i=0; i<100; i++){
		totalADC += ADC_Read(1);
		}
		ADC_value = totalADC/100; //calculating average
		voltage = (double)ADC_value*2560/1023;
		celsius = (voltage/10.00); // Fomula: Vout = T*10mV/oC
		LCD_Display_float(celsius, Temperature);
		LCD_String_xy(0,12,Temperature);/* send string data for printing */
		_delay_ms(5000);
		totalADC = 0;
	}	
	return 0;
}
