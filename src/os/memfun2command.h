#pragma once

#include "os/command.h"

/**
 * Command that calls a member function of an object with two arguments.
 *
 * The RTTI records each instantiation under its template arguments, the member function type, the
 * object class, and the two argument types, and records Command as the base.
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

    /** Call the member function with the arguments. */
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
 * The address listed is that of the instantiation for RemixHUD with two bool arguments.
 *
 * @param pObject The object.
 * @param pfnMember The member function.
 * @param argument1 The first argument.
 * @param argument2 The second argument.
 * @return The command, with no reference taken.
 * @ghidraAddress NTSC-U/C: 0x0034c150
 */
template <typename Function, typename Object, typename Argument1, typename Argument2>
Command *
NewMemFun2Command(Object *pObject, Function pfnMember, Argument1 argument1, Argument2 argument2) {
    return new MemFun2Command<Function, Object, Argument1, Argument2>(
        pObject, pfnMember, argument1, argument2);
}
