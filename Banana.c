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
    // Only read EXP P2 port
    if (n != 0) return;
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
    
    // Display positions
    uint8_t xTextForButtons = 8;
    // Trigger handling
    uint8_t state = 0;    // 0=idle, 1=looking for peak, 2=ignore aftershocks
    uint8_t maxRead = 0;  // used to detect the peak of the piezo signal
    uint8_t refresh = 255; // number of loops to wait for next detection
    uint8_t ticks = 0;    // if >0 we have to wait
    uint8_t v1min = 221;  // min value read
    uint8_t v1max = 255;  // max value read
    uint8_t v2min = 10;   // min MIDI velocity
    uint8_t v2max = 127;  // max MIDI velocity
    uint8_t margin = 15;  // piezo is very sensitive, used to prevent false +
    uint8_t v1gap = v1max-v1min;  // for curve calculations
    uint8_t v2gap = v2max-v2min;

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

        // Read EXP P2 port to detect trigger input
        uint8_t expRead = adcRead8MsbBit(EXP_P2_PIN);

        // inspiration from:
        // https://forum.pjrc.com/index.php?threads/piezo-velocity.49815/
        switch (state) {
        // Idle state: wait for any reading above threshold.
        case 0:
        if(expRead > v1min + margin)
        {
            maxRead = expRead;
            state = 1;
        }
        break;

        // Peak Tracking state: capture largest reading
        case 1:
        if(expRead > maxRead)
        {
            // still waiting for the peak...
            maxRead = expRead;
        } else {
            // afficher maxRead puis les seuils pour velo 64 110 et 127
            char str[4];
            sprintf(str, "%d", maxRead);
            LCDGotoXY(0, 1);
            LCDWriteString("   ");
            LCDGotoXY(0, 1);
            LCDWriteString(str);
            // we have a peak so we send MIDI
            uint8_t velo = 0;
            if(maxRead < v1min + 4*v1gap/5)
            {
                velo = 64;
            } else if(maxRead < v1min + 4*v1gap/5)
            {
                velo = 110;
            } else {
                velo = 110;
            }
            // 38 is the MIDI num of the kick
            midiSendNoteOn(38, velo, MIDI_CHANNEL);
            LCDGotoXY(12, 1);
            LCDWriteString("KICK");
            midiSendNoteOff(38, 0, MIDI_CHANNEL);
            // and we go to Ignore aftershock state
            ticks = refresh;
            state = 2;
        }
        break;

        // Ignore Aftershock state: wait for things to be quiet again.
        default:
            ticks--;
            if(ticks == 0)
            {
                // go back to idle
                state = 0;
                maxRead = 0;
                LCDGotoXY(12, 1);
                LCDWriteString("    ");
            }
        }
    }
}
