#pragma once

/**
 * One value at one song position.
 *
 * The template is attested by the mangled name `t9Sequencer1ZPCt7TickObj1ZP7MuseMsg` at
 * `0x008eec48`, which demangles to `Sequencer<TickObj<MuseMsg *> const *>`. That is the one
 * instantiation the image has, and MultiMuse's vector is an array of it.
 *
 * The layout comes from MultiMuse::SaveFields(), which advances eight bytes per element and writes
 * the first word as the position and the second as a message pointer. The class emits
 * no descriptor of its own, which is consistent with it having no virtual member.
 *
 * Both members are public because MultiMuse::SaveFields() and MultiMusePlayer::Start() read them
 * directly and the image exposes no accessor.
 */
template <typename T>
struct TickObj {
    int mPosition; /*!< Song position, in MIDI ticks. +0x00 */
    T mValue;      /*!< The value at that position. +0x04 */
};

/**
 * Order two entries by song position.
 *
 * @param first The first entry.
 * @param second The second entry.
 * @return Whether first comes strictly before second.
 */
template <typename T>
bool operator<(const TickObj<T> &first, const TickObj<T> &second) {
    return first.mPosition < second.mPosition;
}
