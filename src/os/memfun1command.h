#pragma once

#include "os/command.h"

/**
 * Command that calls a member function of an object with one argument.
 *
 * The RTTI records each instantiation under its template arguments, the member function type, the
 * object class, and the argument type, and records Command as the base. Each instantiation has
 * its own out-of-line copies of the members, and each address listed is one instantiation's.
 *
 * @tparam Function The member function type.
 * @tparam Object The class of the object.
 * @tparam Argument The type of the argument.
 */
template <typename Function, typename Object, typename Argument>
class MemFun1Command : public Command {
public:
    /**
     * Bind a member function and its argument to an object.
     *
     * @param pObject The object.
     * @param pfnMember The member function.
     * @param argument The argument.
     */
    MemFun1Command(Object *pObject, Function pfnMember, Argument argument)
        : mObject(pObject), mMember(pfnMember), mArgument(argument) {
    }

    /**
     * Release the command.
     *
     * @ghidraAddress NTSC-U/C: 0x00337f20
     * @ghidraAddress PAL: 0x003a54d0
     * @ghidraAddress NTSC-U/C: 0x00337f98
     * @ghidraAddress PAL: 0x003a5548
     * @ghidraAddress NTSC-U/C: 0x00338010
     * @ghidraAddress PAL: 0x003a55c0
     * @ghidraAddress NTSC-U/C: 0x0033bdb8
     * @ghidraAddress PAL: 0x003a92f0
     * @ghidraAddress NTSC-U/C: 0x0033c138
     * @ghidraAddress PAL: 0x003a9670
     * @ghidraAddress NTSC-U/C: 0x0033ca40
     * @ghidraAddress PAL: 0x003a9f78
     * @ghidraAddress NTSC-U/C: 0x0033f338
     * @ghidraAddress PAL: 0x003ac870
     * @ghidraAddress NTSC-U/C: 0x00341040
     * @ghidraAddress PAL: 0x003ae578
     * @ghidraAddress NTSC-U/C: 0x00341130
     * @ghidraAddress PAL: 0x003ae668
     * @ghidraAddress NTSC-U/C: 0x0034b6e0
     * @ghidraAddress PAL: 0x003b8b10
     */
    ~MemFun1Command() override = default;

    /**
     * Call the member function with the argument.
     *
     * @ghidraAddress NTSC-U/C: 0x003414b0
     * @ghidraAddress PAL: 0x003ae9e8
     * @ghidraAddress NTSC-U/C: 0x00338180
     * @ghidraAddress PAL: 0x003a5730
     * @ghidraAddress NTSC-U/C: 0x00338200
     * @ghidraAddress PAL: 0x003a57b0
     * @ghidraAddress NTSC-U/C: 0x00338280
     * @ghidraAddress PAL: 0x003a5830
     * @ghidraAddress NTSC-U/C: 0x0033be30
     * @ghidraAddress PAL: 0x003a9368
     * @ghidraAddress NTSC-U/C: 0x0033c4a8
     * @ghidraAddress PAL: 0x003a99e0
     * @ghidraAddress NTSC-U/C: 0x0033cab8
     * @ghidraAddress PAL: 0x003a9ff0
     * @ghidraAddress NTSC-U/C: 0x0033f998
     * @ghidraAddress PAL: 0x003aced0
     * @ghidraAddress NTSC-U/C: 0x003413a8
     * @ghidraAddress PAL: 0x003ae8e0
     * @ghidraAddress NTSC-U/C: 0x0034b850
     * @ghidraAddress PAL: 0x003b8c80
     */
    void Execute() override {
        (mObject->*mMember)(mArgument);
    }

    Object *mObject;    /*!< The object. */
    Function mMember;   /*!< The member function. */
    Argument mArgument; /*!< The argument. */
};

/**
 * Allocate a command that calls a member function of an object with one argument.
 *
 * @param pObject The object.
 * @param pfnMember The member function.
 * @param argument The argument.
 * @return The command, with no reference taken.
 * @ghidraAddress NTSC-U/C: 0x00340ab8
 * @ghidraAddress PAL: 0x003adff0
 * @ghidraAddress NTSC-U/C: 0x00337348
 * @ghidraAddress PAL: 0x003a48f8
 * @ghidraAddress NTSC-U/C: 0x003373c0
 * @ghidraAddress PAL: 0x003a4970
 * @ghidraAddress NTSC-U/C: 0x003379e8
 * @ghidraAddress PAL: 0x003a4f98
 * @ghidraAddress NTSC-U/C: 0x0033bae8
 * @ghidraAddress PAL: 0x003a9020
 * @ghidraAddress NTSC-U/C: 0x0033bea0
 * @ghidraAddress PAL: 0x003a93d8
 * @ghidraAddress NTSC-U/C: 0x0033c910
 * @ghidraAddress PAL: 0x003a9e48
 * @ghidraAddress NTSC-U/C: 0x0033eb88
 * @ghidraAddress PAL: 0x003ac0c0
 * @ghidraAddress NTSC-U/C: 0x00340c20
 * @ghidraAddress PAL: 0x003ae158
 * @ghidraAddress NTSC-U/C: 0x0034b5a8
 * @ghidraAddress PAL: 0x003b89d8
 */
template <typename Function, typename Object, typename Argument>
Command *NewMemFun1Command(Object *pObject, Function pfnMember, Argument argument) {
    return new MemFun1Command<Function, Object, Argument>(pObject, pfnMember, argument);
}
