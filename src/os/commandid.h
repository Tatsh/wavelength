#pragma once

/**
 * Tag that a Scheduler stores beside each queued command.
 *
 * The class is not polymorphic and has no RTTI. The name is inferred. The object is one word.
 */
class CommandId {
public:
    /**
     * Construct a tag from its value.
     *
     * @param nValue The value.
     * @ghidraAddress NTSC-U/C: 0x00281770
     * @ghidraAddress PAL: 0x0028b070
     */
    explicit CommandId(int nValue);

    /**
     * Copy another tag.
     *
     * @param other The tag.
     * @ghidraAddress NTSC-U/C: 0x00281780
     * @ghidraAddress PAL: 0x0028b080
     */
    CommandId(const CommandId &other);

    /**
     * Copy another tag.
     *
     * @param other The tag.
     * @return The tag.
     * @ghidraAddress NTSC-U/C: 0x00281790
     * @ghidraAddress PAL: 0x0028b090
     */
    CommandId &operator=(const CommandId &other);

private:
    int mValue; /*!< The value. */
};

/**
 * The tag with value 0, which the scheduler stores when a caller passes no tag.
 *
 * @ghidraAddress NTSC-U/C: 0x00440d60
 */
extern CommandId TheDefaultCommandId;
