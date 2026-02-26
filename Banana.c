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

#define MM2_CHANNEL 0  // chan 1 is Micromonsta 2
#define M8_CHANNEL  7  // chan 8 on the M8

/*  +---------+
 *  | 4     5 |   foot buttons, numbered from 0 to 4 in this firmware
 *  |    X    |   X is where you find the hand buttons
 *  | 1  2  3 |          labelled Inc(>) Dec(<) Up Down Load/OK Setup/No
 *  +---------+          numbered   5      6     7   8     9      10
 *
 *  * 1 to 5 control the M8:
 *    * 4/1 is Up/Down, 2 is Left+Play (to cue a row)
 *    * 5 is just Play but also used to stop playback
 *    * 3 is Mute the selected tracks (use the arrows, see below)
 *  * <> to scroll through the tracks, down to prepare for a Mute
 */

// M8 keys to MIDI notes (Play Shift Edit Option Left Right Up Down)
#define M8_P 0
#define M8_S 1
#define M8_E 2
#define M8_O 3
#define M8_L 4
#define M8_R 5
#define M8_U 6
#define M8_D 7

void expPedalsCallback(PedalNumber n, uint8_t pos)
{
    // convert from 50->127 to 0->127
    float min = 50.0;
    float max = 127.0;
    pos = (uint8_t)((float)pos-min)*(max/(max-min));
    midiSendControlChange(7, pos, MM2_CHANNEL); // CC, val, chan
    LCDWriteIntXY(4, 1, pos, 3);
}

int main(void)
{
    initBjDevLib();

    expRegisterPedalChangePositionCallback(expPedalsCallback);

    LCDInit(LS_ULINE);
    LcdHideCursor();
    LCDWriteString((char*)"   Banana FC");
    LCDGotoXY(0, 1);
    LCDWriteString((char*)"EXP:");
    
    ledSetColorAll(COLOR_BLACK,  true);
    ledSetColor(0, COLOR_YELLOW, true);
    ledSetColor(1, COLOR_GREEN,  true);
    ledSetColor(2, COLOR_BLACK,  true);
    ledSetColor(3, COLOR_YELLOW, true);
    ledSetColor(4, COLOR_RED,    true);

    ButtonEvent lastButtonEvent;
    
    uint8_t xTextForButtons = 8;
    while(1)
    {
        expProcess();  // exp pedal continuous scan, see the callback
        lastButtonEvent = getButtonLastEvent();
        if(lastButtonEvent.actionType_ == BUTTON_PUSH)
        {
            LOG(SEV_INFO, "Debug works : %d", 2+3);
            uint8_t buttonNumber = lastButtonEvent.buttonNum_;
            LCDGotoXY(xTextForButtons, 1);
            char* display[11] = {"DOWN", "CUE ", "SUST", " UP ", "PLAY",
                                "5   ", "6   ", "7   ", "8   ", "9   ", "10  "};
            LCDWriteString(display[buttonNumber]);
            switch (buttonNumber)
            {
                case 0:
                    midiSendNoteOn(M8_D, 127, M8_CHANNEL);
                    midiSendNoteOff(M8_D, 127, M8_CHANNEL);
                    break;
                case 1:
                    midiSendNoteOn(M8_L, 127, M8_CHANNEL);
                    midiSendNoteOn(M8_P, 127, M8_CHANNEL);
                    midiSendNoteOff(M8_P, 127, M8_CHANNEL);
                    midiSendNoteOff(M8_L, 127, M8_CHANNEL);
                    break;
                case 2:
                    midiSendControlChange(64, 127, MM2_CHANNEL);
                    break;
                case 3:
                    midiSendNoteOn(M8_U, 127, M8_CHANNEL);
                    midiSendNoteOff(M8_U, 127, M8_CHANNEL);
                    break;
                case 4:
                    midiSendNoteOn(M8_P, 127, M8_CHANNEL);
                    midiSendNoteOff(M8_P, 127, M8_CHANNEL);
                    break;
            }
        }
        else if(lastButtonEvent.actionType_ == BUTTON_RELEASE)
        {
            uint8_t buttonNumber = lastButtonEvent.buttonNum_;
            LCDGotoXY(xTextForButtons, 1);
            LCDWriteString("    ");
            switch (buttonNumber)
            {
                case 0:
                    break;
                case 1:
                    break;
                case 2:
                    midiSendControlChange(64, 0, MM2_CHANNEL);
                    break;
                case 3:
                    break;
                case 4:
                    break;
            }
        }
    }
}
