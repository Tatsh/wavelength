#pragma once

/**
 * One MIDI channel message as the EE sends it in a timed batch.
 *
 * Every member is a byte. A message therefore has byte alignment and may sit at any offset of an
 * RPC buffer.
 */
struct MidiEvent {
    unsigned char mStatus; /*!< Status byte. */
    unsigned char mData1;  /*!< First data byte. */
    unsigned char mData2;  /*!< Second data byte. */
    unsigned char mDelay;  /*!< Milliseconds from the batch's arrival until the message is due. */
};
