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
        lastButtonEvent = getButtonLastEvent();
        if(lastButtonEvent.actionType_ == BUTTON_PUSH)
        {
            buttonNumber = lastButtonEvent.buttonNum_;
            midiSendNoteOn(noteNumbers[buttonNumber], 127, MIDI_CHANNEL);
            LCDGotoXY(5, 1);
            LCDWriteString(display[buttonNumber]);
        }

        uint8_t expRead = adcRead8MsbBit(EXP_P1_PIN);

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
            // we have a peak so we send MIDI
            uint8_t velo = (maxRead-v1min)*v2gap/v1gap + v2min;
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
                LCDWriteString("    ");
            }
        }
    }
}
