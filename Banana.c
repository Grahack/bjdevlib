/*
 * Custom firmware for the BJ-TB5
 * @file    Banana.c
 *
 * Greatly inspired by an example file
 */

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>

#include "bjdevlib_tb.h"
#include "lcd_tb.h"

#define MIDI_CHANNEL 9
uint8_t noteNumbers[5] = {100, 103, 102, 101, 104};
char* display[5] = {"PREV", "REC ", "PLAY", "NEXT", "CLR "};

int main(void)
{
    initBjDevLib();

    LCDInit(LS_ULINE);
    LcdHideCursor();
    LCDWriteString((char*)"   Banana FC");
    
    ledSetColorAll(COLOR_RED,    true);
    ledSetColor(0, COLOR_YELLOW, true);
    ledSetColor(1, COLOR_RED,    true);
    ledSetColor(2, COLOR_GREEN,  true);
    ledSetColor(3, COLOR_YELLOW, true);
    ledSetColor(4, COLOR_RED,    true);

    ButtonEvent lastButtonEvent;
    uint8_t buttonNumber;
    
    while(1)
    {
        lastButtonEvent = getButtonLastEvent();
        if(lastButtonEvent.actionType_ == BUTTON_PUSH)
        {
            buttonNumber = lastButtonEvent.buttonNum_;
            midiSendNoteOn(noteNumbers[buttonNumber], 127, MIDI_CHANNEL);
            LCDGotoXY(5, 1);
            LCDWriteString(display[buttonNumber]);
        }
    }
}
