#pragma once

#include "os/command.h"

/**
 * Command that calls a member function of an object with four arguments.
 *
 * The RTTI records each instantiation under its template arguments, the member function type, the
 * object class, and the four argument types, and records Command as the base. Each instantiation
 * has its own out-of-line copies of the members, and each address listed is one instantiation's.
 *
 * @tparam Function The member function type.
 * @tparam Object The class of the object.
 * @tparam Argument1 The type of the first argument.
 * @tparam Argument2 The type of the second argument.
 * @tparam Argument3 The type of the third argument.
 * @tparam Argument4 The type of the fourth argument.
 */
template <typename Function,
          typename Object,
          typename Argument1,
          typename Argument2,
          typename Argument3,
          typename Argument4>
class MemFun4Command : public Command {
public:
    /**
     * Bind a member function and its arguments to an object.
     *
     * @param pObject The object.
     * @param pfnMember The member function.
     * @param argument1 The first argument.
     * @param argument2 The second argument.
     * @param argument3 The third argument.
     * @param argument4 The fourth argument.
     */
    MemFun4Command(Object *pObject,
                   Function pfnMember,
                   Argument1 argument1,
                   Argument2 argument2,
                   Argument3 argument3,
                   Argument4 argument4)
        : mObject(pObject), mMember(pfnMember), mArgument1(argument1), mArgument2(argument2),
          mArgument3(argument3), mArgument4(argument4) {
    }

    /**
     * Release the command.
     *
     * @ghidraAddress NTSC-U/C: 0x0033f428
     * @ghidraAddress PAL: 0x003ac960
     * @ghidraAddress NTSC-U/C: 0x0033f590
     * @ghidraAddress PAL: 0x003acac8
     * @ghidraAddress NTSC-U/C: 0x0034c410
     * @ghidraAddress PAL: 0x003b9840
     */
    ~MemFun4Command() override = default;

    /**
     * Call the member function with the arguments.
     *
     * @ghidraAddress NTSC-U/C: 0x0033f708
     * @ghidraAddress PAL: 0x003acc40
     * @ghidraAddress NTSC-U/C: 0x0033f858
     * @ghidraAddress PAL: 0x003acd90
     * @ghidraAddress NTSC-U/C: 0x0034c488
     * @ghidraAddress PAL: 0x003b98b8
     */
    void Execute() override {
        (mObject->*mMember)(mArgument1, mArgument2, mArgument3, mArgument4);
    }

    Object *mObject;      /*!< The object. */
    Function mMember;     /*!< The member function. */
    Argument1 mArgument1; /*!< The first argument. */
    Argument2 mArgument2; /*!< The second argument. */
    Argument3 mArgument3; /*!< The third argument. */
    Argument4 mArgument4; /*!< The fourth argument. */
};

/**
 * Allocate a command that calls a member function of an object with four arguments.
 *
 * Each address listed is one instantiation's. The first is the instantiation for GfxManager with
 * the arguments of GfxManager::SetRemixPanelText().
 *
 * @param pObject The object.
 * @param pfnMember The member function.
 * @param argument1 The first argument.
 * @param argument2 The second argument.
 * @param argument3 The third argument.
 * @param argument4 The fourth argument.
 * @return The command, with no reference taken.
 * @ghidraAddress NTSC-U/C: 0x0034c1d8
 * @ghidraAddress PAL: 0x003b9608
 * @ghidraAddress NTSC-U/C: 0x0033ed38
 * @ghidraAddress PAL: 0x003ac270
 * @ghidraAddress NTSC-U/C: 0x0033eef0
 * @ghidraAddress PAL: 0x003ac428
 */
template <typename Function,
          typename Object,
          typename Argument1,
          typename Argument2,
          typename Argument3,
          typename Argument4>
Command *NewMemFun4Command(Object *pObject,
                           Function pfnMember,
                           Argument1 argument1,
                           Argument2 argument2,
                           Argument3 argument3,
                           Argument4 argument4) {
    return new MemFun4Command<Function, Object, Argument1, Argument2, Argument3, Argument4>(
        pObject, pfnMember, argument1, argument2, argument3, argument4);
}
