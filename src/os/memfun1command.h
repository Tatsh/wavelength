#pragma once

#include "os/command.h"

/**
 * Command that calls a member function of an object with one argument.
 *
 * The RTTI records each instantiation under its template arguments, the member function type, the
 * object class, and the argument type, and records Command as the base. The addresses listed are
 * those of the instantiation for SoloGameLogic with a bool argument.
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
     * Call the member function with the argument.
     *
     * @ghidraAddress NTSC-U/C: 0x003414b0
     * @ghidraAddress PAL: 0x003ae9e8
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
 */
template <typename Function, typename Object, typename Argument>
Command *NewMemFun1Command(Object *pObject, Function pfnMember, Argument argument) {
    return new MemFun1Command<Function, Object, Argument>(pObject, pfnMember, argument);
}
