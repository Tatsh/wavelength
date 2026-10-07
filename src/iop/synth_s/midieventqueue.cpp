#include "synth_s/midieventqueue.h"

// NTSC-U/C: 0x00006a10
unsigned int MidiEventQueue::sRead;
// NTSC-U/C: 0x00006a14
unsigned int MidiEventQueue::sWrite;
// NTSC-U/C: 0x00006a18
unsigned int MidiEventQueue::sSecondaryRead;
// NTSC-U/C: 0x00006a1c
unsigned int MidiEventQueue::sSecondaryWrite;
// NTSC-U/C: 0x00006a20
MidiEvent MidiEventQueue::sEvents[kCapacity];
// NTSC-U/C: 0x000071f0
unsigned int MidiEventQueue::sTimeLow[kCapacity];
// NTSC-U/C: 0x000079c0
unsigned int MidiEventQueue::sTimeHigh[kCapacity];
// NTSC-U/C: 0x00008190
MidiEvent MidiEventQueue::sSecondaryEvents[kCapacity];
// NTSC-U/C: 0x00008960
unsigned int MidiEventQueue::sSecondaryTimeLow[kCapacity];
// NTSC-U/C: 0x00009130
unsigned int MidiEventQueue::sSecondaryTimeHigh[kCapacity];

void MidiEventQueue::Push(MidiEvent event, unsigned int timeLow, unsigned int timeHigh, int queue) {
    if (queue != kPrimaryQueue) {
        if (((sSecondaryWrite + 1) % kCapacity) == sSecondaryRead) {
            sSecondaryRead = 0;
            sSecondaryWrite = 0;
        }
        sSecondaryEvents[sSecondaryWrite] = event;
        sSecondaryTimeLow[sSecondaryWrite] = timeLow;
        sSecondaryTimeHigh[sSecondaryWrite] = timeHigh;
        if (++sSecondaryWrite >= kCapacity) {
            sSecondaryWrite = 0;
        }
        return;
    }
    if (((sWrite + 1) % kCapacity) == sRead) {
        sRead = 0;
        sWrite = 0;
    }
    sEvents[sWrite] = event;
    sTimeLow[sWrite] = timeLow;
    sTimeHigh[sWrite] = timeHigh;
    if (++sWrite >= kCapacity) {
        sWrite = 0;
    }
}

bool MidiEventQueue::Pop(MidiEvent *event, unsigned int timeLow, unsigned int timeHigh) {
    bool due = false;
    if (sRead != sWrite) {
        if (sTimeHigh[sRead] < timeHigh) {
            due = true;
        } else if (!(timeLow < sTimeLow[sRead])) {
            due = true;
        }
    }
    if (due) {
        *event = sEvents[sRead];
        if (++sRead >= kCapacity) {
            sRead = 0;
        }
    }
    return due;
}
