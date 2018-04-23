#include <stdio.h>
#include <stdlib.h>

// for signal catching
#include <errno.h>
#include <signal.h>
#include <string.h> // for fprintf

#include <wiringPi.h>
#include <sr595.h>
#include <pcf8591.h>

#ifndef	TRUE
#  define	TRUE  (1==1)
#  define	FALSE (1==2)
#endif

#define ADDR_SR595 100
#define BITS 8

#define ADDR_8591 200
const int i2cAddr = 0x4F;

#define DATAPIN  21 // data blue (pin 14)
#define LATCHPIN 22 // latch green (pin 12)
#define CLOCKPIN 23 // clock yellow (pin 11)

const int ledPins[] = { 25, 26 }; // for bar graph leds 9 and 10

void onOff(_Bool b)
{
    int thisLed;

    for (thisLed = 0; thisLed < 10; thisLed++)
    {
        if (thisLed < 8) digitalWrite(ADDR_SR595 + thisLed, b);
        else digitalWrite(ledPins[thisLed - 8], b);
    }
}

void doGraph(int num)
{
    int thisLed;
    _Bool toggle;

    if (num < 0 || num > 10) return;
    if (num == 0) onOff(0);
    else if (num == 10) onOff(1);
    else
    {
        for (thisLed = 0; thisLed < 10; thisLed++)
        {
            toggle = (thisLed < num);
            if (thisLed < 8) digitalWrite(ADDR_SR595 + thisLed, toggle);
            else digitalWrite(ledPins[thisLed - 8], toggle);
        }
    }
}

int makeNumber(int v)
{
    int n = 0;

    if (v == 0) n = 0;
    if (v > 127 && v < 140) n = 0;
    else if (v > 126 && v < 144) n = 1;
    else if (v > 116 && v < 155) n = 2;
    else if (v > 110 && v < 160) n = 3;
    else if (v > 100 && v < 170) n = 4;
    else if (v >  94 && v < 176) n = 5;
    else if (v >  79 && v < 192) n = 6;
    else if (v >  73 && v < 197) n = 7;
    else if (v >  68 && v < 202) n = 8;
    else if (v >  63 && v < 208) n = 9;
    else if (v <  52 || v > 218) n = 10;
    return n;
}

static void die(int sig)
{
    onOff(0);
    if (sig != 0 && sig != 2) (void)fprintf(stderr, "caught signal %s\n", strsignal(sig));
    if (sig == 2) (void)fprintf(stderr, "Exiting due to Ctrl + C (%s)\n", strsignal(sig));
    exit(0);
}

int main(int argc, char **argv)
{
    int number = 0;
    int value;

    (void)signal(SIGINT, die);
    (void)signal(SIGHUP, die);
    (void)signal(SIGTERM, die);
    (void)signal(SIGABRT, die);
    if (wiringPiSetup() == -1)
    {
        fprintf(stdout, "[%s:%d] [%s] Error trying to setup wiringPi - oops: %s\n", __FILE__, __LINE__, __func__, strerror(errno));
        exit(1);
    }
    sr595Setup(ADDR_SR595, BITS, DATAPIN, CLOCKPIN, LATCHPIN);
    pcf8591Setup(ADDR_8591, i2cAddr);
    pinMode(ledPins[0], OUTPUT);
    pinMode(ledPins[1], OUTPUT);
    onOff(0);
    while (1)
    {
        value = analogRead(ADDR_8591 + 0);
        number = makeNumber(value);
        doGraph(number);
    }
    onOff(0);
    return EXIT_SUCCESS;
}

