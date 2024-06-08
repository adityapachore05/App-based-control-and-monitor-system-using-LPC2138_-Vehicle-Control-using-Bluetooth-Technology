#include<lpc213x.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#define bit(x) (1<<x)
void delay_ms(unsigned int j) /* Function for delay in milliseconds */
{
    unsigned int x,i;
	for(i=0;i<j;i++)
	{
    for(x=0; x<6000; x++);    /* loop to generate 1 millisecond delay with Cclk = 60MHz */
	}
}
void LCD_CMD(char command)
{
	IOPIN0 = ( (IOPIN0 & 0xFFFF00FF) | ((command & 0xF0)<<8) ); 
	IOSET0 = 0x00000040; /* EN = 1 */
	IOCLR0 = 0x00000030; /* RS = 0, RW = 0 */
	delay_ms(5);
	IOCLR0 = 0x00000040; /* EN = 0, RS and RW unchanged(i.e. RS = RW = 0)	 */
	delay_ms(5);
	IOPIN0 = ( (IOPIN0 & 0xFFFF00FF) | ((command & 0x0F)<<12) ); 
	IOSET0 = 0x00000040; /* EN = 1 */
	IOCLR0 = 0x00000030; /* RS = 0, RW = 0 */
	delay_ms(5);
	IOCLR0 = 0x00000040; /* EN = 0, RS and RW unchanged(i.e. RS = RW = 0)	 */
	delay_ms(5);
	
}
void LCD_INIT(void)
{
	IODIR0 = 0x0000FFF0; /* P0.12 to P0.15 LCD Data. P0.4,5,6 as RS RW and EN */
	delay_ms(20);
	LCD_CMD(0x02);  /* Initialize lcd in 4-bit mode */
	LCD_CMD(0x28);  /* 2 lines */
	LCD_CMD(0x0C);   /* Display on cursor off */
	LCD_CMD(0x06);  /* Auto increment cursor */
	LCD_CMD(0x01);   /* Display clear */
	LCD_CMD(0x80);  /* First line first position */
}

void LCD_CHAR (char msg)
{
		IOPIN0 = ( (IOPIN0 & 0xFFFF00FF) | ((msg & 0xF0)<<8) );
		IOSET0 = 0x00000050; /* RS = 1, EN = 1 */
		IOCLR0 = 0x00000020; /* RW = 0 */
		delay_ms(2);
		IOCLR0 = 0x00000040; /* EN = 0, RS and RW unchanged(i.e. RS = 1, RW = 0) */
		delay_ms(5);
		IOPIN0 = ( (IOPIN0 & 0xFFFF00FF) | ((msg & 0x0F)<<12) );
		IOSET0 = 0x00000050; /* RS = 1, EN = 1 */
		IOCLR0 = 0x00000020; /* RW = 0 */
		delay_ms(2);
		IOCLR0 = 0x00000040; /* EN = 0, RS and RW unchanged(i.e. RS = 1, RW = 0) */
		delay_ms(5);
}
void LCD_STRING (char* msg)
{
	unsigned int i=0;
	while(msg[i]!=0)
	{
		LCD_CHAR(msg[i]);
		i++;
	}
}
void LCD_CLEAR_LINE2(void)
{
	unsigned int k;
    LCD_CMD(0xC0);   /* Move cursor to the beginning of the second line */

    for(k = 0; k < 16; k++)
    {
        LCD_CHAR(' ');  /* Write a space */
    }
}
void LCD_UPDATE_LINE2(char* msg)
{
    LCD_CLEAR_LINE2();  /* Clear the second line */

    LCD_CMD(0xC0);   /* Move cursor to the beginning of the second line */
    LCD_STRING(msg); /* Write the new message */
}
void print()
{
	LCD_INIT();
	LCD_STRING("SYSTEM IS ON !!!");
	LCD_CMD(0x01);   /* Clear display */
  delay_ms(10);     /* Wait for the command to complete */
	LCD_STRING("MOVING CONDITION");
}
unsigned int i;

void pll()
{
    PLLCON=0X01;
    PLLCFG=0X24;
    PLLFEED=0XAA;
    PLLFEED=0X55;
    while((PLLSTAT&(1<<10))==0);
    PLLCON=0X03;
    PLLFEED=0XAA;
    PLLFEED=0X55;
    VPBDIV=0x02;                         //pclk=30mhz
}
 
void ser_int()
{
    PINSEL0|=0x05;
    U0LCR=0x83;
    U0DLL=195;
    U0DLM=0;
    U0LCR=0x03;
 
}
 
void tx(unsigned char c)
{
    while((U0LSR&(1<<5))==0);
    U0THR=c;
}
 
char rx()
{
    unsigned char a;
    while((U0LSR&(1<<0))==0);
    a=U0RBR;
    return a;
}

int main()
{
    unsigned char b;
    IODIR1=0xf0000;                     //Declaring as a output
    pll();
    ser_int();
	  print();
    while(1) {
        b=rx();
        tx(b);
        if(b == 'F') {
            //Forward/
						LCD_CMD(0xCC);
            IOSET1=bit(16) | bit(18);           //IN1 = 1, IN3 = 1
            IOCLR1=bit(17) | bit(19);           //IN2 = 0, IN4 = 0
						LCD_UPDATE_LINE2("MOVING FORWARD");
        } else if (b == 'B') {
            //Reverse/
						LCD_CMD(0xCC);
            IOSET1=bit(17) | bit(19);           //IN2 = 1, IN4 = 1
            IOCLR1=bit(16) | bit(18);           //IN1 = 0, IN3 = 0
						LCD_UPDATE_LINE2("MOVING BACKWARD");
        } else if (b == 'R') {
            //Right/
						LCD_CMD(0xCC);
            IOSET1=bit(16) ;                                            //IN1 = 1
            IOCLR1=bit(17) | bit(19) | bit(18);     //IN2 = 0, IN3 = 0, IN4 = 0
						LCD_UPDATE_LINE2("TURNING RIGHT");
        } else if (b == 'L') {
            //Left/
						LCD_CMD(0xCC);
            IOSET1=bit(18) ;                                            //IN3 = 1
            IOCLR1=bit(16) | bit(17) | bit(19);     //IN2 = 0, IN1 = 0, IN4 = 0
						LCD_UPDATE_LINE2("TURNING LEFT");
        } else if (b == 'S') {
            //Off/
						LCD_CMD(0xCC);
            IOCLR1=bit(16) | bit(17) | bit(18) | bit(19); //IN1 = IN2 = IN3 = IN4 = 0
						LCD_UPDATE_LINE2("STOPPING VEHICLE");
						delay_ms(20);
						LCD_UPDATE_LINE2("VEHICLE STOPPED");}
					else
					{
						LCD_UPDATE_LINE2("VEHICLE PARKED");
					}
        
    }
}
