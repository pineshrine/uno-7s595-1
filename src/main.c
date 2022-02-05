/**
 * https://github.com/pineshrine/uno-7s595-1/
 * author : Matsu @pineshrine
 * just as hobby work at midnight
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const int map7seg[11][8] =
{
    {0,0,1,1,1,1,1,1},
    {0,0,0,0,0,1,1,0},
    {0,1,0,1,1,0,1,1},
    {0,1,0,0,1,1,1,1},
    {0,1,1,0,0,1,1,0},
    {0,1,1,0,1,1,0,1},
    {0,1,1,1,1,1,0,1},
    {0,0,0,0,0,1,1,1},
    {0,1,1,1,1,1,1,1},
    {0,1,1,0,1,1,1,1},
    {0,0,0,0,0,0,1,1}
};

struct time_time
{
    int hour;
    int minute;
    int second;
};

struct time_time time_timer;


volatile unsigned char input[8];
volatile unsigned char input_length;

void dr_595(unsigned int d1_value, unsigned int d2_value, unsigned int d3_value, unsigned int d4_value, unsigned int colon);
void display_quad7seg(unsigned int dis_value);
void send_char(unsigned char data);
void send_msg(unsigned char *str);
void send_msg_r(unsigned char *str);
void send_msg_n(unsigned char *str);

//intended for drive 74HC595
void dr_595(unsigned int d1_value, unsigned int d2_value, unsigned int d3_value, unsigned int d4_value, unsigned int colon)
{
    cli();
    //disable cathode pins
    PORTD &= ~((1<<PORTD2)|(1<<PORTD3)|(1<<PORTD4)|(1<<PORTD5)|(1<<PORTD6));

    //seems to be silly but hardcode for 1st try
    //drive 595 for anode pins

    //d1
    for (int i = 0; i < 8; i++)
    {
        //find 0or1 bit of inputted num
        if (map7seg[d1_value][i] == 0)
        {
            PORTB |= (1<<PORTB0);
        } else if (map7seg[d1_value][i] == 1)
        {
            PORTB &= ~(1<<PORTB0);
        }
        //send a clock
        PORTB |= (1<<PORTB1);
        PORTB &= ~(1<<PORTB1);
    }
    //send a latch
    PORTB |= (1<<PORTB2);
    PORTB &= ~(1<<PORTB2);
    //d1 on
    PORTD |= (1<<PORTD2);

    //d2
    for (int i = 0; i < 8; i++)
    {
        //find 0or1 bit of inputted num
        if (map7seg[d2_value][i] == 0)
        {
            PORTB |= (1<<PORTB0);
        } else if (map7seg[d2_value][i] == 1)
        {
            PORTB &= ~(1<<PORTB0);
        }
        //send a clock
        PORTB |= (1<<PORTB1);
        PORTB &= ~(1<<PORTB1);
    }
    //d1 off
    PORTD &= ~(1<<PORTD2);
    //send a latch
    PORTB |= (1<<PORTB2);
    PORTB &= ~(1<<PORTB2);
    //d2 on
    PORTD |= (1<<PORTD3);

    //d3
    for (int i = 0; i < 8; i++)
    {
        //find 0or1 bit of inputted num
        if (map7seg[d3_value][i] == 0)
        {
            PORTB |= (1<<PORTB0);
        } else if (map7seg[d3_value][i] == 1)
        {
            PORTB &= ~(1<<PORTB0);
        }
        //send a clock
        PORTB |= (1<<PORTB1);
        PORTB &= ~(1<<PORTB1);
    }
    //d2 off
    PORTD &= ~(1<<PORTD3);
    //send a latch
    PORTB |= (1<<PORTB2);
    PORTB &= ~(1<<PORTB2);
    //d3 on
    PORTD |= (1<<PORTD4);

    //d4
    for (int i = 0; i < 8; i++)
    {
        //find 0or1 bit of inputted num
        if (map7seg[d4_value][i] == 0)
        {
            PORTB |= (1<<PORTB0);
        } else if (map7seg[d4_value][i] == 1)
        {
            PORTB &= ~(1<<PORTB0);
        }
        //send a clock
        PORTB |= (1<<PORTB1);
        PORTB &= ~(1<<PORTB1);
    }
    //d3 off
    PORTD &= ~(1<<PORTD4);
    //send a latch
    PORTB |= (1<<PORTB2);
    PORTB &= ~(1<<PORTB2);
    //d4 on
    PORTD |= (1<<PORTD5);

    //colon
    for (int i = 0; i < 8; i++)
    {
        //find 0or1 bit of inputted num
        if (map7seg[10][i] == 0)
        {
            PORTB |= (1<<PORTB0);
        } else if (map7seg[10][i] == 1)
        {
            PORTB &= ~(1<<PORTB0);
        }
        //send a clock
        PORTB |= (1<<PORTB1);
        PORTB &= ~(1<<PORTB1);
    }
    //d4 off
    PORTD &= ~(1<<PORTD5);
    //send a latch
    PORTB |= (1<<PORTB2);
    PORTB &= ~(1<<PORTB2);
    //d6 blink
    if (colon == 1)
    {
        PORTD |= (1<<PORTD6);
    }
    sei();

}

//display 4 digits value on OSL40391-IG quad7seg-led array via dr_595 func
void display_quad7seg(unsigned int dis_value)
{
    int d1 = dis_value / 1000;
    dis_value %= 1000;
    int d2 = dis_value / 100;
    dis_value %= 100; 
    int d3 = dis_value / 10;
    dis_value %= 10;
    int d4 = dis_value;
    
    for (int i = 0; i < 1000; i++)
    {
        dr_595(d1,d2,d3,d4,1);
    }
    for (int i = 0; i < 1000; i++)
    {
        dr_595(d1,d2,d3,d4,0);
    }
}

//send atomic char to usart(arduino uno way)
void send_char(unsigned char data)
{
	while(!(UCSR0A & (1<<UDRE0)));
    UDR0 = data;
}

//send msg without cr or lf
void send_msg(unsigned char *str)
{
    while (*str != '\0')
    {
        send_char(*str++);
    }
}

//send line with CR before start 
void send_msg_r(unsigned char *str)
{
    send_char(13);
    while (*str != '\0')
    {
        send_char(*str++);
    }
}

//send msg with crlf.
void send_msg_n(unsigned char *str)
{
    while (*str != '\0')
    {
        send_char(*str++);
    }
    send_char(10);
    send_char(13);
}

ISR(USART_RX_vect)
{
    unsigned char buf = UDR0;
    if (buf != '\n')
    {
        input[input_length++] = buf;
    }
    if (input_length >= 7 || buf == '\n')
    {
        //a little bit tricky cuz I'm not good to code ^^;
        //there is a more better way
        unsigned long hhmmss;
        char *e;
        send_msg_n(input);
        char *ret[32];

        const char hoge1[] = "debug:";send_msg(hoge1);
        hhmmss = strtoul(input,&e,10);
        int hh = hhmmss / 10000;
        int mm = (hhmmss / 100) % 100;
        int ss = hhmmss % 100;
        time_timer.hour = hh;
        time_timer.minute = mm;
        time_timer.second = ss;
        sprintf(ret,"hr:%d",time_timer.hour);
        send_msg_n(ret);
        sprintf(ret,"mi:%d",time_timer.minute);
        send_msg_n(ret);
        sprintf(ret,"se:%d",time_timer.second);
        send_msg_n(ret);

        input_length = 0;
    }
}

int main(void)
{
    // HC595 drive
    DDRB |= ((1<<DDB0)|(1<<DDB1)|(1<<DDB2));
    //7seg cathode
    DDRD |= ((1<<DDD2)|(1<<DDD3)|(1<<DDD4)|(1<<DDD5)|(1<<DDD6));

    //usart configuration for terminal monitor
    UBRR0 = ((F_CPU/(16UL*9600UL))-1); //9600 is BAUD, magic word
    UCSR0A = UCSR0B = UCSR0C = 0; // clear all registers
    UCSR0B |= ((1<<TXEN0)|(1<<RXEN0)|(1<<RXCIE0)); // enable tx,rx,rx interrupt
    UCSR0C |= ((1<<UCSZ01)|(1<<UCSZ00)); 

    //timer configuration for clock intr
    //WIP

    char *buf[128];

    sprintf(buf,"%s","started. input HHMMSS:");
    send_msg_n(buf);


    while (1)
    {
        for (int i = 0; i < 10000; i++)
        {
            display_quad7seg(i);
            _delay_us(10);
            //sprintf(buf,"%d ",i);
            //send_msg_r(buf);
        }
    }

    return 0;
}
