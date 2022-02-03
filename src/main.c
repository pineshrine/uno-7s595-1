/**
 * Copyright (C) PlatformIO <contact@platformio.org>
 * See LICENSE for details.
 */

#include <avr/io.h>
#include <util/delay.h>

const int map7seg[10][8] =
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
    {0,1,1,0,1,1,1,1}

};

void dr_595(int d1_value, int d2_value, int d3_value, int d4_value)
{
    //disable cathode pins
    PORTD &= ~((1<<PORTD2)|(1<<PORTD3)|(1<<PORTD4)|(1<<PORTD5));

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
    _delay_us(10);

}

void display_quad7seg(int dis_value)
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
        dr_595(d1,d2,d3,d4);
    }
}

int main(void)
{
    // HC595 drive
    DDRB |= ((1<<DDB0)|(1<<DDB1)|(1<<DDB2));
    //7seg cathode
    DDRD |= ((1<<DDD2)|(1<<DDD3)|(1<<DDD4)|(1<<DDD5));

    while (1)
    {
        for (int i = 0; i < 10000; i++)
        {
            display_quad7seg(i);
            _delay_us(10);
        }
    }

    return 0;
}
