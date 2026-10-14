#ifndef LIBRTREASM_H
#define LIBRTREASM_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Reassembly of messages that arrive in sequenced fragments.
 *
 * The names of the routines and of their parameters are inferred.
 */

/** The number of fragment slots of a reassembly buffer. */
#define RT_REASSEMBLER_SLOT_COUNT 64

/** The largest fragment payload a slot stores, in bytes. */
#define RT_REASSEMBLER_PAYLOAD_SIZE 504

/** The bits of a fragment sequence word that give the sequence number. */
#define RT_REASSEMBLER_SEQUENCE_MASK 0x1fffffff

/** The bit of a fragment sequence word that marks the first fragment of a message. */
#define RT_REASSEMBLER_FIRST 0x20000000

/** The bit of a fragment sequence word that marks the last fragment of a message. */
#define RT_REASSEMBLER_LAST 0x40000000

struct rtDmePeer;

/** One fragment slot of a reassembly buffer. */
typedef struct {
    unsigned int sequence; /*!< The sequence word of the fragment, flag bits included. */
    unsigned int length;   /*!< The bytes of the payload, or 0 when the slot is free. */
    unsigned char payload[RT_REASSEMBLER_PAYLOAD_SIZE]; /*!< The bytes of the fragment. */
} rtReassemblerSlot;

/** A buffer that reassembles the fragments of large messages. */
typedef struct {
    struct rtDmePeer *owner; /*!< The peer the fragments come from. */
    /**
     * The sequence word the next message starts from. After a message completes it is the
     * sequence word of the first fragment of that message, flag bits included.
     */
    unsigned int nextSequence;
    unsigned int lastSequence; /*!< The sequence word of the newest last fragment stored. */
    unsigned int slotCount;    /*!< The number of occupied slots. */
    rtReassemblerSlot slots[RT_REASSEMBLER_SLOT_COUNT]; /*!< The slots, indexed by sequence. */
} rtReassembler;

/**
 * Create a cleared reassembly buffer.
 *
 * @param owner The peer the fragments come from.
 * @return The buffer, or null.
 * @ghidraAddress NTSC-U/C: 0x002ed750
 * @ghidraAddress PAL: 0x002f76c0
 */
rtReassembler *rtReassemblerCreate(struct rtDmePeer *owner);

/**
 * Release a reassembly buffer.
 *
 * @param reassembler The buffer, or null.
 * @ghidraAddress NTSC-U/C: 0x002ed7dc
 * @ghidraAddress PAL: 0x002f774c
 */
void rtReassemblerDestroy(rtReassembler *reassembler);

/**
 * Store a fragment in a reassembly buffer.
 *
 * A fragment larger than RT_REASSEMBLER_PAYLOAD_SIZE or with no bytes only marks its slot with
 * the sequence word.
 *
 * @param reassembler The buffer.
 * @param sequence The sequence word of the fragment with its first and last fragment bits.
 * @param data The bytes of the fragment.
 * @param length The number of bytes.
 * @ghidraAddress NTSC-U/C: 0x002ed804
 * @ghidraAddress PAL: 0x002f7774
 */
void rtReassemblerStore(rtReassembler *reassembler,
                        unsigned int sequence,
                        const void *data,
                        unsigned int length);

/**
 * Report whether a reassembly buffer has a complete message.
 *
 * When a message is complete, the routine moves the start of the next message to it.
 *
 * @param reassembler The buffer.
 * @return The bytes of the complete message, or 0 when no message is complete.
 * @ghidraAddress NTSC-U/C: 0x002ed8f4
 * @ghidraAddress PAL: 0x002f7864
 */
unsigned int rtReassemblerHasMessage(rtReassembler *reassembler);

/**
 * Copy the next complete message out of a reassembly buffer and free its slots.
 *
 * @param reassembler The buffer.
 * @param buffer Receives the message.
 * @param size The size of the buffer in bytes.
 * @return The bytes of the message, 0 when no message is complete, or -1 when the message does
 * not fit.
 * @ghidraAddress NTSC-U/C: 0x002eda7c
 * @ghidraAddress PAL: 0x002f79ec
 */
int rtReassemblerRead(rtReassembler *reassembler, void *buffer, unsigned int size);

#ifdef __cplusplus
}
#endif

#endif
