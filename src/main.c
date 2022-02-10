/**
 * https://github.com/pineshrine/uno-7s595-1/
 * author : Matsu @pineshrine
 * just as hobby work at midnight
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <util/delay.h>
#include <avr/eeprom.h>

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

volatile struct time_time
{
    int hour;
    int minute;
    int second;
    int msecond;
};

volatile struct time_time time_timer ={0,0,0,0};

volatile int setting_flg = 0;
volatile int configure_start = 0;
volatile int mode_flg = 0;
volatile int cfg_digit = 0;
volatile int c00 = 0;
volatile int c01 = 0;

volatile unsigned char input[8];
volatile unsigned char input_length;

void dr_595(unsigned int d1_value, unsigned int d2_value, unsigned int d3_value, unsigned int d4_value, unsigned int colon);
void dr595_spec(int value,int digit);
void display_quad7seg_time(int dot);
void send_char(unsigned char data);
void send_msg(unsigned char *str);
void send_msg_r(unsigned char *str);
void send_msg_n(unsigned char *str);
void input_logic(void);

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
void display_quad7seg_time(int dot)
{
        int d1 = time_timer.hour / 10;
        int d2 = time_timer.hour % 10;
        int d3 = time_timer.minute /10;
        int d4 = time_timer.minute %10;
        for (int i = 0; i < 100; i++)
        {
            dr_595(d1,d2,d3,d4,dot);
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

void input_logic(void)
{
    char *buf[128];

    if (configure_start == 0)
    {
        time_timer.hour = 0;
        time_timer.minute = 0;
        time_timer.second = 0;
        time_timer.msecond = 0;
        mode_flg = 1;
        configure_start = 1;
        sprintf(buf,"%s","config start,c00,c01:");
        send_msg(buf);
        sprintf(buf,"%d,",c00);
        send_msg(buf);
        sprintf(buf,"%d",c01);
        send_msg_n(buf);
    }else if ((c00 == 1) && (mode_flg < 2))
    {
            mode_flg++;
            sprintf(buf,"%s","mode++:");
            send_msg(buf);
            sprintf(buf,"%d",mode_flg);
            send_msg_n(buf);
            c00 = 0;
    } else if ((c01 == 1) && (mode_flg < 3))
    {
        c01 = 0;
        if (mode_flg == 1)
        {
            if (time_timer.hour < 23)
            {
                time_timer.hour++;
                time_timer.second = 0;
                time_timer.msecond = 0;
                sprintf(buf,"%s","hour++");
                send_msg_n(buf);
            } else {
                time_timer.hour = 0;
                time_timer.second = 0;
                time_timer.msecond = 0;
                sprintf(buf,"%s","hour = 0");
                send_msg_n(buf);
            }
        } else if (mode_flg == 2)
        {
            if (time_timer.minute < 59)
            {
                time_timer.minute++;
                time_timer.second = 0;
                time_timer.msecond = 0;
                sprintf(buf,"%s","min++");
                send_msg_n(buf);
            } else {
                time_timer.minute = 0;
                time_timer.second = 0;
                time_timer.msecond = 0;
                sprintf(buf,"%s","min = 0");
                send_msg_n(buf);
            }
        }
    } else {
        mode_flg = 0;
        configure_start = 0;
        c00 = 0;
        c01 = 0;
        sprintf(buf,"%s","config end");
        eeprom_time_save();
        send_msg_n(buf);
    }
}

void eeprom_time_save(void)
{
    eeprom_busy_wait();
    eeprom_update_block(&time_timer,0x0000,sizeof(time_timer));
    send_msg_n("time saved.");
}

void eeprom_time_read(void)
{
    eeprom_busy_wait();
    eeprom_read_block(&time_timer,0x0000,sizeof(time_timer));
    send_msg_n("time readed.");
}

ISR(USART_RX_vect)
{
    //cli();
    unsigned char buf = UDR0;
    if (buf == 0x1A)
    {
        setting_flg = 1;
        return;
    }
    if ((buf != '\n') && (setting_flg == 1))
    {
        input[input_length++] = buf;
    }
    if (((input_length >= 7) || (buf == '\n')) && (setting_flg == 1))
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

        //flag on
        setting_flg = 0;

        //reset buffer pointer
        input_length = 0;

        //disable rx
        UCSR0B &= ~((1<<RXEN0)|(1<<RXCIE0));

        eeprom_time_save();
    }
    //sei();
}

ISR(TIMER0_COMPA_vect)
{
    if (++time_timer.msecond >= 60)
    {
        time_timer.msecond = 0;
        time_timer.second++;
    }
    if (time_timer.second >= 60)
    {
        time_timer.second = 0;
        time_timer.minute++;
        
    }
    if(time_timer.minute >= 60)
    {
        time_timer.minute = 0;
        time_timer.hour++;
    }
    if (time_timer.hour >= 24)
    {
        time_timer.hour = 0;
    }
}

ISR(PCINT1_vect)
{
    _delay_ms(10); 
    c00 = !(PINC & 0x01);
    c01 = !(PINC & 0x02);
    PCIFR |= (1<<PCIF1);
    if ((c00 == 01) || (c01 == 1))
    {
        input_logic();
    }
}

int main(void)
{
    // HC595 drive
    DDRB |= ((1<<DDB0)|(1<<DDB1)|(1<<DDB2));
    //7seg cathode
    DDRD |= ((1<<DDD2)|(1<<DDD3)|(1<<DDD4)|(1<<DDD5)|(1<<DDD6));
    //sw input uses pc0,1 port, avoid pc4,5 for future use(i2c)
    DDRC &= ~((1<<DDC0)|(1<<DDC1));

    //input pull-up
    PORTC |= ((1<<PORTC0)|(1<<PORTC1));
    //allow expernal pin intr INT8-14
    PCICR |= (1<<PCIE1);
    //allow intr mask for PCINT7,8 (pc0=pcint8 pc1=pcint9)
    PCMSK1 |= ((1<<PCINT8)|(1<<PCINT9));

    //usart configuration for terminal monitor
    UBRR0 = ((F_CPU/(16UL*9600UL))-1); //9600 is BAUD, magic word
    UCSR0A = UCSR0B = UCSR0C = 0; // clear all registers
    UCSR0B |= ((1<<TXEN0)|(1<<RXEN0)|(1<<RXCIE0)); // enable tx,rx,rx interrupt
    UCSR0C |= ((1<<UCSZ01)|(1<<UCSZ00)); 

    //timer0 configuration for clock intr
    TCNT0 = 0; //init timer0
    TCCR0A |= (1<<WGM01); // CTC type
    TCCR0B |= ((1<<CS02)|(1<<CS00)); //1024 divide, thus 1ms is 250
    OCR0A = 250-1; // set timer0 compare register A
    TIMSK0 |= (1<<OCIE0A); //set timer0 intr mask reg to compare match A

    char *buf[128];

    eeprom_time_read();

    sprintf(buf,"%s","started. input ctrl-z then HHMMSS:");
    send_msg_n(buf);
    sei();

    while (1)
    {
        if (setting_flg == 0)
        {
            sprintf(buf,"%d ",time_timer.second);
            send_msg_r(buf);
        }
        if ((time_timer.second % 2) > 0 && configure_start == 0)
        {
            display_quad7seg_time(1);
        } else if (configure_start == 0)
        {
            display_quad7seg_time(0);
        } else
        {
            display_quad7seg_time(0);
        }
    }

    return 0;
}
