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

#define MM2_CHANNEL 3  // chan 4 is Micromonsta 2
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
 *  * <> to scroll through the tracks, down to prepare for a Mute/Unmute
 *                                     up to prepare for an Unmute/mute
 *                                     middle pos is do not touch this track
 *  +----------------+
 *  |        12345678|  Currently editing track 2 (shows MUX instead of v^-).
 *  |        -M-v--^-|  On button 3 switch: mute track 4, unmute track 7.
 *  +----------------+
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

// TB-5 setup buttons
#define TB_R 5
#define TB_L 6
#define TB_U 7
#define TB_D 8
#define TB_O 9
#define TB_N 10

// Display positions
uint8_t xTextForHandButtons = 8;
uint8_t yTextForHandButtons = 1;
uint8_t xTextForFootButtons = 0;
uint8_t yTextForFootButtons = 0;
// M8 tracks handling
bool tracks_switch_state = false;  // start unswitched
uint8_t tracks_currently_edited = 8;  // 8 is "track 9" (hides the MUX)
bool tracks_states[8] = {true, true, true, true, true, true, true, true};
// 0=mute on switch, 1=do not touch, 2=unmute on switch
uint8_t tracks_conf[8] = {1, 1, 1, 1, 1, 1, 1, 1};
// relevant MIDI notes
int M8_mute_notes[8] = {12, 13, 14, 15, 16, 17, 18, 19};
// Trigger handling
uint8_t state = 0;    // 0=idle, 1=looking for peak, 2=ignore aftershocks
uint8_t maxRead = 0;  // used to detect the peak of the piezo signal
uint16_t refresh_kick = 25500; // number of loops to wait for next detection
uint16_t ticks_kick  = 0;      // if >0 we have to wait
uint8_t v1min = 221;  // min value read
uint8_t v1max = 255;  // max value read
uint8_t v2min = 10;   // min MIDI velocity
uint8_t v2max = 127;  // max MIDI velocity
uint8_t margin = 15;  // piezo is very sensitive, used to prevent false +

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

void updateMutesAfterButtons()
{
    for (int i=0; i<8; i++)
    {
        uint8_t track_conf = tracks_conf[i];
        if (i == tracks_currently_edited) track_conf = track_conf + 10;
        LCDGotoXY(xTextForHandButtons + i, yTextForHandButtons);
        switch (track_conf)
        {
            case 0:
                LCDWriteString("v");
                break;
            case 1:
                LCDWriteString("-");
                break;
            case 2:
                LCDWriteString("^");
                break;
            case 10:
                LCDWriteString("M");
                break;
            case 11:
                LCDWriteString("X");
                break;
            case 12:
                LCDWriteString("U");
                break;
        }
    }
}

void updateAfterButtons(uint8_t buttonNumber, uint8_t status)
{
    if (status == BUTTON_PUSH)
    {
        if (buttonNumber < 5)
        {
            LCDGotoXY(xTextForFootButtons, yTextForFootButtons);
            char* display[5] = {"DOWN   ",
                                "CUE    ",
                                "SW-T   ",
                                " UP    ",
                                "PLAY   "};
            LCDWriteString(display[buttonNumber]);
        }
        else if (buttonNumber == TB_U)
        {
            if (tracks_conf[tracks_currently_edited] < 2)
                tracks_conf[tracks_currently_edited]++;
            updateMutesAfterButtons();
        }
        else if (buttonNumber == TB_D)
        {
            if (tracks_conf[tracks_currently_edited] > 0)
                tracks_conf[tracks_currently_edited]--;
            updateMutesAfterButtons();
        }
        else if (buttonNumber == TB_L || buttonNumber == TB_R)
        {
            updateMutesAfterButtons();
        }
    }
    else if (status == BUTTON_RELEASE)
    {
        if (buttonNumber < 5)
        {
            LCDGotoXY(xTextForFootButtons, yTextForFootButtons);
            LCDWriteString("BotBoss");
        }
    }
    else
    {
        LOG(SEV_INFO, "Unknown button status");
    }
}

int main(void)
{
    initBjDevLib();

    expRegisterPedalChangePositionCallback(expPedalsCallback);

    LCDInit(LS_ULINE);
    LcdHideCursor();
    // "BotBoss" is meant to be overwritten on first foot press.
    LCDWriteString((char*)"BotBoss 12345678");
    LCDGotoXY(0, 1);
    LCDWriteString((char*)"EXP:");
    updateMutesAfterButtons();
    
    ledSetColorAll(COLOR_BLACK,  true);
    ledSetColor(0, COLOR_YELLOW, true);
    ledSetColor(1, COLOR_GREEN,  true);
    ledSetColor(2, COLOR_BLACK,  true);
    ledSetColor(3, COLOR_YELLOW, true);
    ledSetColor(4, COLOR_RED,    true);

    ButtonEvent lastButtonEvent;

    while (1)
    {
        lastButtonEvent = getButtonLastEvent();
        if (lastButtonEvent.actionType_ == BUTTON_PUSH)
        {
            LOG(SEV_INFO, "Debug works : %d", 2+3);
            uint8_t buttonNumber = lastButtonEvent.buttonNum_;
            switch (buttonNumber)
            {
                case 0:  // button 1
                    midiSendNoteOn(M8_D, 127, M8_CHANNEL);
                    midiSendNoteOff(M8_D, 127, M8_CHANNEL);
                    break;
                case 1:  // button 2
                    midiSendNoteOn(M8_L, 127, M8_CHANNEL);
                    midiSendNoteOn(M8_P, 127, M8_CHANNEL);
                    midiSendNoteOff(M8_P, 127, M8_CHANNEL);
                    midiSendNoteOff(M8_L, 127, M8_CHANNEL);
                    break;
                case 2:  // button 3
                    midiSendControlChange(64, 127, MM2_CHANNEL);
                    break;
                case 3:  // button 4
                    midiSendNoteOn(M8_U, 127, M8_CHANNEL);
                    midiSendNoteOff(M8_U, 127, M8_CHANNEL);
                    break;
                case 4:  // button 5
                    midiSendNoteOn(M8_P, 127, M8_CHANNEL);
                    midiSendNoteOff(M8_P, 127, M8_CHANNEL);
                    break;
                case 5:  // Inc or Right
                    if (tracks_currently_edited < 8) tracks_currently_edited++;
                    break;
                case 6:  // Dec or Left
                    if (tracks_currently_edited > 0) tracks_currently_edited--;
                    break;
                case 7:  // Up
                    break;
                case 8:  // Down
                    break;
                case 9:  // Load / OK
                    break;
                case 10: // Setup / No
                    break;
            }
            updateAfterButtons(buttonNumber, BUTTON_PUSH);
        }
        else if(lastButtonEvent.actionType_ == BUTTON_RELEASE)
        {
            uint8_t buttonNumber = lastButtonEvent.buttonNum_;
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
            updateAfterButtons(buttonNumber, BUTTON_RELEASE);
        }

        // Read EXP P1 port to handle the exp pedal
        // See the callback before main
        expProcess();

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
            char str[6];
            sprintf(str, "%d", maxRead);
            LCDGotoXY(0, 1);
            LCDWriteString("   ");
            LCDGotoXY(0, 1);
            LCDWriteString(str);
            // we have a peak so we send MIDI
            uint8_t velo = 0;
            // I wanted to compute velo between v2min and v2max
            const uint8_t v1gap = v1max-v1min;
            const uint8_t v2gap = v2max-v2min;
            // but I can only detect 3 states :/
            velo = v2gap;  // dummy, just to avoid compiler warnings
            if(maxRead < v1min + 4*v1gap/5)
            {
                velo = 64;
            } else if(maxRead < v1min + 4*v1gap/5)
            {
                velo = 110;
            } else {
                velo = 127;
            }
            // 38 is the MIDI num of the kick
            midiSendNoteOn(38, velo, M8_CHANNEL);
            LCDGotoXY(12, 1);
            LCDWriteString("KICK");
            midiSendNoteOff(38, 0, M8_CHANNEL);
            // and we go to Ignore aftershock state
            ticks_kick = refresh_kick;
            state = 2;
        }
        break;

        // Ignore Aftershock state: wait for things to be quiet again.
        default:
            ticks_kick--;
            if(ticks_kick == 0)
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
