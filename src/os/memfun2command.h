#pragma once

#include "os/command.h"

/**
 * Command that calls a member function of an object with two arguments.
 *
 * The RTTI records each instantiation under its template arguments, the member function type, the
 * object class, and the two argument types, and records Command as the base. Each instantiation
 * has its own out-of-line copies of the members, and each address listed is one instantiation's.
 *
 * @tparam Function The member function type.
 * @tparam Object The class of the object.
 * @tparam Argument1 The type of the first argument.
 * @tparam Argument2 The type of the second argument.
 */
template <typename Function, typename Object, typename Argument1, typename Argument2>
class MemFun2Command : public Command {
public:
    /**
     * Bind a member function and its arguments to an object.
     *
     * @param pObject The object.
     * @param pfnMember The member function.
     * @param argument1 The first argument.
     * @param argument2 The second argument.
     */
    MemFun2Command(Object *pObject, Function pfnMember, Argument1 argument1, Argument2 argument2)
        : mObject(pObject), mMember(pfnMember), mArgument1(argument1), mArgument2(argument2) {
    }

    /**
     * Release the command.
     *
     * @ghidraAddress NTSC-U/C: 0x0033f2c0
     * @ghidraAddress PAL: 0x003ac7f8
     * @ghidraAddress NTSC-U/C: 0x0033f4a0
     * @ghidraAddress PAL: 0x003ac9d8
     * @ghidraAddress NTSC-U/C: 0x0033f518
     * @ghidraAddress PAL: 0x003aca50
     * @ghidraAddress NTSC-U/C: 0x0033f608
     * @ghidraAddress PAL: 0x003acb40
     * @ghidraAddress NTSC-U/C: 0x0034c398
     * @ghidraAddress PAL: 0x003b97c8
     */
    ~MemFun2Command() override = default;

    /**
     * Call the member function with the arguments.
     *
     * @ghidraAddress NTSC-U/C: 0x0033f680
     * @ghidraAddress PAL: 0x003acbb8
     * @ghidraAddress NTSC-U/C: 0x0033f798
     * @ghidraAddress PAL: 0x003accd0
     * @ghidraAddress NTSC-U/C: 0x0033f820
     * @ghidraAddress PAL: 0x003acd58
     * @ghidraAddress NTSC-U/C: 0x0033fa18
     * @ghidraAddress PAL: 0x003acf50
     * @ghidraAddress NTSC-U/C: 0x0034c518
     * @ghidraAddress PAL: 0x003b9948
     */
    void Execute() override {
        (mObject->*mMember)(mArgument1, mArgument2);
    }

    Object *mObject;      /*!< The object. */
    Function mMember;     /*!< The member function. */
    Argument1 mArgument1; /*!< The first argument. */
    Argument2 mArgument2; /*!< The second argument. */
};

/**
 * Allocate a command that calls a member function of an object with two arguments.
 *
 * Each address listed is one instantiation's.
 *
 * @param pObject The object.
 * @param pfnMember The member function.
 * @param argument1 The first argument.
 * @param argument2 The second argument.
 * @return The command, with no reference taken.
 * @ghidraAddress NTSC-U/C: 0x0034c150
 * @ghidraAddress PAL: 0x003b9580
 * @ghidraAddress NTSC-U/C: 0x0033eb00
 * @ghidraAddress PAL: 0x003ac038
 * @ghidraAddress NTSC-U/C: 0x0033ede0
 * @ghidraAddress PAL: 0x003ac318
 * @ghidraAddress NTSC-U/C: 0x0033ee68
 * @ghidraAddress PAL: 0x003ac3a0
 * @ghidraAddress NTSC-U/C: 0x0033ef98
 * @ghidraAddress PAL: 0x003ac4d0
 */
template <typename Function, typename Object, typename Argument1, typename Argument2>
Command *
NewMemFun2Command(Object *pObject, Function pfnMember, Argument1 argument1, Argument2 argument2) {
    return new MemFun2Command<Function, Object, Argument1, Argument2>(
        pObject, pfnMember, argument1, argument2);
}
