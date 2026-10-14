#pragma once

#include "os/command.h"

/**
 * Command that calls a member function of an object.
 *
 * The RTTI records each instantiation under its template arguments, the member function type and
 * the object class, and records Command as the base. Each instantiation has its own out-of-line
 * copies of the members, and each address listed is one instantiation's.
 *
 * @tparam Function The member function type.
 * @tparam Object The class of the object.
 */
template <typename Function, typename Object>
class MemFunCommand : public Command {
public:
    /**
     * Construct a command that calls a member function.
     *
     * @param pObject The object.
     * @param pfnMember The member function.
     */
    MemFunCommand(Object *pObject, Function pfnMember) : mObject(pObject), mMember(pfnMember) {
    }

    /**
     * Release the command.
     *
     * @ghidraAddress NTSC-U/C: 0x00334b38
     * @ghidraAddress PAL: 0x003a20e8
     * @ghidraAddress NTSC-U/C: 0x00334bb0
     * @ghidraAddress PAL: 0x003a2160
     * @ghidraAddress NTSC-U/C: 0x00334c28
     * @ghidraAddress PAL: 0x003a21d8
     * @ghidraAddress NTSC-U/C: 0x003354d8
     * @ghidraAddress PAL: 0x003a2a88
     * @ghidraAddress NTSC-U/C: 0x00335550
     * @ghidraAddress PAL: 0x003a2b00
     * @ghidraAddress NTSC-U/C: 0x00337ea8
     * @ghidraAddress PAL: 0x003a5458
     * @ghidraAddress NTSC-U/C: 0x00338088
     * @ghidraAddress PAL: 0x003a5638
     * @ghidraAddress NTSC-U/C: 0x00338698
     * @ghidraAddress PAL: 0x003a5c48
     * @ghidraAddress NTSC-U/C: 0x0033ad50
     * @ghidraAddress PAL: 0x003a8288
     * @ghidraAddress NTSC-U/C: 0x0033b208
     * @ghidraAddress PAL: 0x003a8740
     * @ghidraAddress NTSC-U/C: 0x0033c1b0
     * @ghidraAddress PAL: 0x003a96e8
     * @ghidraAddress NTSC-U/C: 0x0033c2a0
     * @ghidraAddress PAL: 0x003a97d8
     * @ghidraAddress NTSC-U/C: 0x0033c7c0
     * @ghidraAddress PAL: 0x003a9cf8
     * @ghidraAddress NTSC-U/C: 0x0033cf08
     * @ghidraAddress PAL: 0x003aa440
     * @ghidraAddress NTSC-U/C: 0x0033da88
     * @ghidraAddress PAL: 0x003aafc0
     * @ghidraAddress NTSC-U/C: 0x0033dbd8
     * @ghidraAddress PAL: 0x003ab110
     * @ghidraAddress NTSC-U/C: 0x0033f248
     * @ghidraAddress PAL: 0x003ac780
     * @ghidraAddress NTSC-U/C: 0x0033f3b0
     * @ghidraAddress PAL: 0x003ac8e8
     * @ghidraAddress NTSC-U/C: 0x0033fec8
     * @ghidraAddress PAL: 0x003ad400
     * @ghidraAddress NTSC-U/C: 0x00340fc8
     * @ghidraAddress PAL: 0x003ae500
     * @ghidraAddress PAL: 0x003ae5f0
     * @ghidraAddress NTSC-U/C: 0x003411a8
     * @ghidraAddress PAL: 0x003ae6e0
     * @ghidraAddress NTSC-U/C: 0x00344530
     * @ghidraAddress PAL: 0x003b1a68
     * @ghidraAddress NTSC-U/C: 0x00344688
     * @ghidraAddress PAL: 0x003b1bc0
     * @ghidraAddress NTSC-U/C: 0x00345c98
     * @ghidraAddress PAL: 0x003b31d0
     * @ghidraAddress NTSC-U/C: 0x00347e00
     * @ghidraAddress PAL: 0x003b5230
     * @ghidraAddress NTSC-U/C: 0x00349958
     * @ghidraAddress PAL: 0x003b6d88
     * @ghidraAddress NTSC-U/C: 0x0034ab88
     * @ghidraAddress PAL: 0x003b7fb8
     * @ghidraAddress NTSC-U/C: 0x0034b758
     * @ghidraAddress PAL: 0x003b8b88
     * @ghidraAddress NTSC-U/C: 0x0034c0b0
     * @ghidraAddress PAL: 0x003b94e0
     * @ghidraAddress NTSC-U/C: 0x0034c7c0
     * @ghidraAddress PAL: 0x003b9bf0
     * @ghidraAddress NTSC-U/C: 0x0034ca80
     * @ghidraAddress PAL: 0x003b9eb0
     * @ghidraAddress NTSC-U/C: 0x0034cce8
     * @ghidraAddress PAL: 0x003ba118
     * @ghidraAddress NTSC-U/C: 0x0034d208
     * @ghidraAddress PAL: 0x003ba638
     * @ghidraAddress NTSC-U/C: 0x00355e20
     * @ghidraAddress PAL: 0x003c30d0
     * @ghidraAddress NTSC-U/C: 0x00356098
     * @ghidraAddress PAL: 0x003c3348
     * @ghidraAddress NTSC-U/C: 0x003a66c8
     * @ghidraAddress PAL: 0x004153a8
     */
    ~MemFunCommand() override = default;

    /**
     * Call the member function.
     *
     * @ghidraAddress NTSC-U/C: 0x0033ff40
     * @ghidraAddress PAL: 0x003ad478
     * @ghidraAddress NTSC-U/C: 0x00334ca0
     * @ghidraAddress PAL: 0x003a2250
     * @ghidraAddress NTSC-U/C: 0x00334d20
     * @ghidraAddress PAL: 0x003a22d0
     * @ghidraAddress NTSC-U/C: 0x00334da0
     * @ghidraAddress PAL: 0x003a2350
     * @ghidraAddress NTSC-U/C: 0x003355c8
     * @ghidraAddress PAL: 0x003a2b78
     * @ghidraAddress NTSC-U/C: 0x00335648
     * @ghidraAddress PAL: 0x003a2bf8
     * @ghidraAddress NTSC-U/C: 0x00338100
     * @ghidraAddress PAL: 0x003a56b0
     * @ghidraAddress NTSC-U/C: 0x00338300
     * @ghidraAddress PAL: 0x003a58b0
     * @ghidraAddress NTSC-U/C: 0x00338710
     * @ghidraAddress PAL: 0x003a5cc0
     * @ghidraAddress NTSC-U/C: 0x0033adc8
     * @ghidraAddress PAL: 0x003a8300
     * @ghidraAddress NTSC-U/C: 0x0033b280
     * @ghidraAddress PAL: 0x003a87b8
     * @ghidraAddress NTSC-U/C: 0x0033c318
     * @ghidraAddress PAL: 0x003a9850
     * @ghidraAddress NTSC-U/C: 0x0033c428
     * @ghidraAddress PAL: 0x003a9960
     * @ghidraAddress NTSC-U/C: 0x0033c8e8
     * @ghidraAddress PAL: 0x003a9e20
     * @ghidraAddress NTSC-U/C: 0x0033cf80
     * @ghidraAddress PAL: 0x003aa4b8
     * @ghidraAddress NTSC-U/C: 0x0033db00
     * @ghidraAddress PAL: 0x003ab038
     * @ghidraAddress NTSC-U/C: 0x0033dc50
     * @ghidraAddress PAL: 0x003ab188
     * @ghidraAddress NTSC-U/C: 0x0033f918
     * @ghidraAddress PAL: 0x003ace50
     * @ghidraAddress NTSC-U/C: 0x0033faa0
     * @ghidraAddress PAL: 0x003acfd8
     * @ghidraAddress NTSC-U/C: 0x00341328
     * @ghidraAddress PAL: 0x003ae860
     * @ghidraAddress NTSC-U/C: 0x00341530
     * @ghidraAddress PAL: 0x003aea68
     * @ghidraAddress NTSC-U/C: 0x003445a8
     * @ghidraAddress PAL: 0x003b1ae0
     * @ghidraAddress NTSC-U/C: 0x00344700
     * @ghidraAddress PAL: 0x003b1c38
     * @ghidraAddress NTSC-U/C: 0x00345d10
     * @ghidraAddress PAL: 0x003b3248
     * @ghidraAddress NTSC-U/C: 0x00347e78
     * @ghidraAddress PAL: 0x003b52a8
     * @ghidraAddress NTSC-U/C: 0x003499d0
     * @ghidraAddress PAL: 0x003b6e00
     * @ghidraAddress NTSC-U/C: 0x0034ac00
     * @ghidraAddress PAL: 0x003b8030
     * @ghidraAddress NTSC-U/C: 0x0034b7d0
     * @ghidraAddress PAL: 0x003b8c00
     * @ghidraAddress NTSC-U/C: 0x0034c128
     * @ghidraAddress PAL: 0x003b9558
     * @ghidraAddress NTSC-U/C: 0x0034c838
     * @ghidraAddress PAL: 0x003b9c68
     * @ghidraAddress NTSC-U/C: 0x0034caf8
     * @ghidraAddress PAL: 0x003b9f28
     * @ghidraAddress NTSC-U/C: 0x0034cd60
     * @ghidraAddress PAL: 0x003ba190
     * @ghidraAddress NTSC-U/C: 0x0034d280
     * @ghidraAddress PAL: 0x003ba6b0
     * @ghidraAddress NTSC-U/C: 0x00355e98
     * @ghidraAddress PAL: 0x003c3148
     * @ghidraAddress NTSC-U/C: 0x00356110
     * @ghidraAddress PAL: 0x003c33c0
     * @ghidraAddress NTSC-U/C: 0x003a6740
     * @ghidraAddress PAL: 0x00415420
     */
    void Execute() override {
        (mObject->*mMember)();
    }

    Object *mObject;  /*!< The object. */
    Function mMember; /*!< The member function. */
};

/**
 * Command that calls a member function of an object that takes one float.
 *
 * The member function declaration gave the float a default of -1, which the command passes. The
 * addresses listed are those of the instantiation for GfxManager.
 *
 * @tparam Result The return type of the member function, which the command discards.
 * @tparam Owner The class that declares the member function.
 * @tparam Object The class of the object.
 */
template <typename Result, typename Owner, typename Object>
class MemFunCommand<Result (Owner::*)(float), Object> : public Command {
public:
    /** The member function type. */
    using Function = Result (Owner::*)(float);

    /** The argument the member function receives. */
    static constexpr float kDefaultArgument = -1.0f;

    /**
     * Construct a command that calls a member function.
     *
     * @param pObject The object.
     * @param pfnMember The member function.
     */
    MemFunCommand(Object *pObject, Function pfnMember) : mObject(pObject), mMember(pfnMember) {
    }

    /**
     * Release the command.
     *
     * @ghidraAddress NTSC-U/C: 0x003410b8
     * @ghidraAddress PAL: 0x003ae5f0
     */
    ~MemFunCommand() override = default;

    /**
     * Call the member function with the default argument.
     *
     * @ghidraAddress NTSC-U/C: 0x00341428
     * @ghidraAddress PAL: 0x003ae960
     */
    void Execute() override {
        (mObject->*mMember)(kDefaultArgument);
    }

    Object *mObject;  /*!< The object. */
    Function mMember; /*!< The member function. */
};

/**
 * Allocate a command that calls a member function of an object.
 *
 * Each address listed is one instantiation's.
 *
 * @param pObject The object.
 * @param pfnMember The member function.
 * @return The command, with no reference taken.
 * @ghidraAddress NTSC-U/C: 0x0033fdc8
 * @ghidraAddress PAL: 0x003ad300
 * @ghidraAddress NTSC-U/C: 0x003342e8
 * @ghidraAddress PAL: 0x003a1898
 * @ghidraAddress NTSC-U/C: 0x00334610
 * @ghidraAddress PAL: 0x003a1bc0
 * @ghidraAddress NTSC-U/C: 0x00334678
 * @ghidraAddress PAL: 0x003a1c28
 * @ghidraAddress NTSC-U/C: 0x00335128
 * @ghidraAddress PAL: 0x003a26d8
 * @ghidraAddress NTSC-U/C: 0x00335190
 * @ghidraAddress PAL: 0x003a2740
 * @ghidraAddress NTSC-U/C: 0x003372e0
 * @ghidraAddress PAL: 0x003a4890
 * @ghidraAddress NTSC-U/C: 0x00337a60
 * @ghidraAddress PAL: 0x003a5010
 * @ghidraAddress NTSC-U/C: 0x00338630
 * @ghidraAddress PAL: 0x003a5be0
 * @ghidraAddress NTSC-U/C: 0x0033ab88
 * @ghidraAddress PAL: 0x003a80c0
 * @ghidraAddress NTSC-U/C: 0x0033ae48
 * @ghidraAddress PAL: 0x003a8380
 * @ghidraAddress NTSC-U/C: 0x0033bf18
 * @ghidraAddress PAL: 0x003a9450
 * @ghidraAddress NTSC-U/C: 0x0033c018
 * @ghidraAddress PAL: 0x003a9550
 * @ghidraAddress NTSC-U/C: 0x0033c6a8
 * @ghidraAddress PAL: 0x003a9be0
 * @ghidraAddress NTSC-U/C: 0x0033cea0
 * @ghidraAddress PAL: 0x003aa3d8
 * @ghidraAddress NTSC-U/C: 0x0033da20
 * @ghidraAddress PAL: 0x003aaf58
 * @ghidraAddress NTSC-U/C: 0x0033db28
 * @ghidraAddress PAL: 0x003ab060
 * @ghidraAddress NTSC-U/C: 0x0033e220
 * @ghidraAddress PAL: 0x003ab758
 * @ghidraAddress NTSC-U/C: 0x0033ec00
 * @ghidraAddress PAL: 0x003ac138
 * @ghidraAddress NTSC-U/C: 0x00340a50
 * @ghidraAddress PAL: 0x003adf88
 * @ghidraAddress NTSC-U/C: 0x00340b30
 * @ghidraAddress PAL: 0x003ae068
 * @ghidraAddress NTSC-U/C: 0x00340ca0
 * @ghidraAddress PAL: 0x003ae1d8
 * @ghidraAddress NTSC-U/C: 0x003444c8
 * @ghidraAddress PAL: 0x003b1a00
 * @ghidraAddress NTSC-U/C: 0x003445d0
 * @ghidraAddress PAL: 0x003b1b08
 * @ghidraAddress NTSC-U/C: 0x00344e98
 * @ghidraAddress PAL: 0x003b23d0
 * @ghidraAddress NTSC-U/C: 0x00347cc0
 * @ghidraAddress PAL: 0x003b50f0
 * @ghidraAddress NTSC-U/C: 0x003498f0
 * @ghidraAddress PAL: 0x003b6d20
 * @ghidraAddress NTSC-U/C: 0x0034a868
 * @ghidraAddress PAL: 0x003b7c98
 * @ghidraAddress NTSC-U/C: 0x0034b620
 * @ghidraAddress PAL: 0x003b8a50
 * @ghidraAddress NTSC-U/C: 0x0034c048
 * @ghidraAddress PAL: 0x003b9478
 * @ghidraAddress NTSC-U/C: 0x0034c620
 * @ghidraAddress PAL: 0x003b9a50
 * @ghidraAddress NTSC-U/C: 0x0034c948
 * @ghidraAddress PAL: 0x003b9d78
 * @ghidraAddress NTSC-U/C: 0x0034cb20
 * @ghidraAddress PAL: 0x003b9f50
 * @ghidraAddress NTSC-U/C: 0x0034cdc8
 * @ghidraAddress PAL: 0x003ba1f8
 * @ghidraAddress NTSC-U/C: 0x003554c8
 * @ghidraAddress PAL: 0x003c2778
 * @ghidraAddress NTSC-U/C: 0x00355fa0
 * @ghidraAddress PAL: 0x003c3250
 * @ghidraAddress NTSC-U/C: 0x003a6620
 * @ghidraAddress PAL: 0x00415300
 */
template <typename Function, typename Object>
Command *NewMemFunCommand(Object *pObject, Function pfnMember) {
    return new MemFunCommand<Function, Object>(pObject, pfnMember);
}
