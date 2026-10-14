#include "rt/rtreassembler.h"

#include <stddef.h>
#include <string.h>

#include "rt/rtmemory.h"

enum {
    kSlotIndexMask = RT_REASSEMBLER_SLOT_COUNT - 1,
    // A stored fragment is preserved when its sequence distance from the new one, minus one, does
    // not exceed this bound.
    kStaleDistanceLimit = 0x0ffffffd,
};

rtReassembler *rtReassemblerCreate(struct rtDmePeer *owner) {
    rtReassembler *reassembler = NULL;
    unsigned int i;

    RtAllocZeroedBlock((void **)&reassembler, sizeof(rtReassembler));
    if (reassembler == NULL) {
        return reassembler;
    }
    reassembler->owner = owner;
    reassembler->nextSequence = 0;
    reassembler->lastSequence = 0;
    reassembler->slotCount = 0;
    for (i = 0; i < RT_REASSEMBLER_SLOT_COUNT; ++i) {
        reassembler->slots[i].sequence = 0;
        reassembler->slots[i].length = 0;
    }
    return reassembler;
}

void rtReassemblerDestroy(rtReassembler *reassembler) {
    if (reassembler != NULL) {
        RtFreeBlock((void **)&reassembler);
    }
}

void rtReassemblerStore(rtReassembler *reassembler,
                        unsigned int sequence,
                        const void *data,
                        unsigned int length) {
    rtReassemblerSlot *slot = &reassembler->slots[sequence & kSlotIndexMask];
    unsigned int number = sequence & RT_REASSEMBLER_SEQUENCE_MASK;

    if (slot->length != 0) {
        // Yes, the binary masks the stored sequence word with the slot index mask here.
        if (((((slot->sequence & kSlotIndexMask) - number) & RT_REASSEMBLER_SEQUENCE_MASK) - 1) <=
            kStaleDistanceLimit) {
            return;
        }
        slot->length = 0;
        --reassembler->slotCount;
    }
    if ((sequence & RT_REASSEMBLER_LAST) != 0 &&
        ((number - (reassembler->lastSequence & RT_REASSEMBLER_SEQUENCE_MASK)) &
         RT_REASSEMBLER_SLOT_COUNT) != 0) {
        reassembler->lastSequence = sequence;
    }
    slot->sequence = sequence;
    if (length == 0 || length > RT_REASSEMBLER_PAYLOAD_SIZE) {
        return;
    }
    memcpy(slot->payload, data, length);
    slot->length = length;
    ++reassembler->slotCount;
}

unsigned int rtReassemblerHasMessage(rtReassembler *reassembler) {
    unsigned int next = reassembler->nextSequence;
    unsigned int expected = next & RT_REASSEMBLER_SEQUENCE_MASK;
    unsigned int start = next & kSlotIndexMask;
    unsigned int index;
    unsigned int scanned;
    unsigned int total = 0;
    unsigned int first = 0;
    rtReassemblerSlot *slot;

    if (reassembler->slotCount == 0) {
        return 0;
    }
    index = start;
    if (reassembler->slots[start].length == 0 ||
        (reassembler->slots[start].sequence & RT_REASSEMBLER_FIRST) == 0) {
        // Find the next stored first fragment. A full turn stops on the starting slot.
        do {
            index = (index + 1) & kSlotIndexMask;
        } while (index != start &&
                 (reassembler->slots[index].length == 0 ||
                  (reassembler->slots[index].sequence & RT_REASSEMBLER_FIRST) == 0));
        expected = reassembler->slots[index].sequence & RT_REASSEMBLER_SEQUENCE_MASK;
        if (reassembler->nextSequence >= expected) {
            return 0;
        }
        reassembler->nextSequence = expected;
        index = reassembler->slots[index].sequence & kSlotIndexMask;
    }
    for (scanned = 0; scanned < RT_REASSEMBLER_SLOT_COUNT;
         ++scanned, index = (index + 1) & kSlotIndexMask) {
        slot = &reassembler->slots[index];
        if ((slot->sequence & RT_REASSEMBLER_FIRST) != 0) {
            first = slot->sequence;
            total = slot->length;
            expected = first & RT_REASSEMBLER_SEQUENCE_MASK;
        } else if (total == 0 || slot->length == 0 ||
                   expected != (slot->sequence & RT_REASSEMBLER_SEQUENCE_MASK)) {
            total = 0;
            continue;
        } else {
            total += slot->length;
            if ((slot->sequence & RT_REASSEMBLER_LAST) != 0) {
                break;
            }
        }
        ++expected;
    }
    if (scanned == RT_REASSEMBLER_SLOT_COUNT) {
        total = 0;
    }
    if (total != 0) {
        reassembler->nextSequence = first;
    }
    return total;
}

int rtReassemblerRead(rtReassembler *reassembler, void *buffer, unsigned int size) {
    unsigned char *bytes = buffer;
    unsigned int length = rtReassemblerHasMessage(reassembler);
    unsigned int expected;
    unsigned int index;
    unsigned int scanned;
    rtReassemblerSlot *slot;

    if (length == 0) {
        return 0;
    }
    if (size < length) {
        return -1;
    }
    expected = reassembler->nextSequence & RT_REASSEMBLER_SEQUENCE_MASK;
    index = reassembler->nextSequence & kSlotIndexMask;
    length = 0;
    if (reassembler->slotCount == 0) {
        return 0;
    }
    for (scanned = 0; scanned < RT_REASSEMBLER_SLOT_COUNT;
         ++scanned, index = (index + 1) & kSlotIndexMask) {
        slot = &reassembler->slots[index];
        if ((slot->sequence & RT_REASSEMBLER_FIRST) != 0) {
            // Yes, the binary does not test the last fragment bit of a first fragment.
            memcpy(&bytes[length], slot->payload, slot->length);
            length += slot->length;
            expected = slot->sequence & RT_REASSEMBLER_SEQUENCE_MASK;
            slot->length = 0;
            --reassembler->slotCount;
        } else if (length == 0 || slot->length == 0 ||
                   expected != (slot->sequence & RT_REASSEMBLER_SEQUENCE_MASK)) {
            length = 0;
            continue;
        } else {
            memcpy(&bytes[length], slot->payload, slot->length);
            length += slot->length;
            slot->length = 0;
            --reassembler->slotCount;
            if ((slot->sequence & RT_REASSEMBLER_LAST) != 0) {
                break;
            }
        }
        ++expected;
    }
    if (scanned == RT_REASSEMBLER_SLOT_COUNT) {
        length = 0;
    }
    if (length != 0) {
        reassembler->nextSequence = expected + 1;
    }
    return (int)length;
}
