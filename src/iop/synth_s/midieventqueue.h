#pragma once

#include "synth_s/midievent.h"

/**
 * Ring buffers of MIDI messages stamped with the system clock time they fall due.
 *
 * The class has no RTTI, and its name is inferred from its role. Every member is static. There are
 * two queues, but only the first is ever filled or drained. A push that would fill a queue empties
 * it first.
 */
class MidiEventQueue {
public:
    static constexpr int kCapacity = 500; /*!< Slots of each queue. */

    /** The queues Push() selects. */
    enum Queue {
        kPrimaryQueue = 0,   /*!< The queue Pop() drains. */
        kSecondaryQueue = 1, /*!< Any other value; never drained. */
    };

    /**
     * Append a message.
     *
     * @param event The message.
     * @param timeLow Low word of the due time.
     * @param timeHigh High word of the due time.
     * @param queue #kPrimaryQueue, or any other value for the secondary queue.
     * @ghidraAddress NTSC-U/C: 0x00000d7c
     * @ghidraAddress PAL: 0x00000d7c
     */
    static void Push(MidiEvent event, unsigned int timeLow, unsigned int timeHigh, int queue);

    /**
     * Remove the oldest message of the primary queue when it is due.
     *
     * A message counts as due when its high word is below the current one, or when the current
     * low word has reached its low word whatever the high words.
     *
     * @param event Receives the message.
     * @param timeLow Low word of the current time.
     * @param timeHigh High word of the current time.
     * @return True when a message was removed.
     * @ghidraAddress NTSC-U/C: 0x00000ef0
     * @ghidraAddress PAL: 0x00000ef0
     */
    static bool Pop(MidiEvent *event, unsigned int timeLow, unsigned int timeHigh);

private:
    static unsigned int sRead;                         /*!< Next slot Pop() reads. */
    static unsigned int sWrite;                        /*!< Next slot Push() writes. */
    static unsigned int sSecondaryRead;                /*!< Read slot of the secondary queue. */
    static unsigned int sSecondaryWrite;               /*!< Write slot of the secondary queue. */
    static MidiEvent sEvents[kCapacity];               /*!< Messages. */
    static unsigned int sTimeLow[kCapacity];           /*!< Low words of the due times. */
    static unsigned int sTimeHigh[kCapacity];          /*!< High words of the due times. */
    static MidiEvent sSecondaryEvents[kCapacity];      /*!< Messages of the secondary queue. */
    static unsigned int sSecondaryTimeLow[kCapacity];  /*!< Low words of the secondary queue. */
    static unsigned int sSecondaryTimeHigh[kCapacity]; /*!< High words of the secondary queue. */
};
