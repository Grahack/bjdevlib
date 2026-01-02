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

#define MIDI_CHANNEL 7  // chan 8 on the M8

/*  +---------+
 *  | 4     5 |   BJ buttons, numbered from 0 in this firmware
 *  |         |
 *  | 1  2  3 |   For buttons 1 2 3  I want: UP LEFT+PLAY DOWN
 *  +---------+
 *
 *  M8 controls
 *  key  P S E O L R U D
 *  note 0 1 2 3 4 5 6 7
 */

#define M8_P 0
#define M8_S 1
#define M8_E 2
#define M8_O 3
#define M8_L 4
#define M8_R 5
#define M8_U 6
#define M8_D 7

char* display[5] = {" UP ", "CUE ", "DOWN", "    ", "    "};

int main(void)
{
    initBjDevLib();

    LCDInit(LS_ULINE);
    LcdHideCursor();
    LCDWriteString((char*)"   Banana FC");
    
    ledSetColorAll(COLOR_BLACK,  true);
    ledSetColor(0, COLOR_YELLOW, true);
    ledSetColor(1, COLOR_GREEN,  true);
    ledSetColor(2, COLOR_YELLOW, true);
    ledSetColor(3, COLOR_RED,    true);
    ledSetColor(4, COLOR_RED,    true);

    ButtonEvent lastButtonEvent;
    
    while(1)
    {
        lastButtonEvent = getButtonLastEvent();
        if(lastButtonEvent.actionType_ == BUTTON_PUSH)
        {
            uint8_t buttonNumber = lastButtonEvent.buttonNum_;
            LCDGotoXY(5, 1);
            LCDWriteString(display[buttonNumber]);
            switch (buttonNumber)
            {
                case 0:
                    midiSendNoteOn(M8_U, 127, MIDI_CHANNEL);
                    midiSendNoteOff(M8_U, 127, MIDI_CHANNEL);
                    break;
                case 1:
                    midiSendNoteOn(M8_L, 127, MIDI_CHANNEL);
                    midiSendNoteOn(M8_P, 127, MIDI_CHANNEL);
                    midiSendNoteOff(M8_P, 127, MIDI_CHANNEL);
                    midiSendNoteOff(M8_L, 127, MIDI_CHANNEL);
                    break;
                case 2:
                    midiSendNoteOn(M8_D, 127, MIDI_CHANNEL);
                    midiSendNoteOff(M8_D, 127, MIDI_CHANNEL);
                    break;
            }
        }
    }
}
