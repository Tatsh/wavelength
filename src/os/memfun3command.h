#pragma once

#include "os/command.h"

/**
 * Command that calls a member function of an object with three arguments.
 *
 * The RTTI records each instantiation under its template arguments, the member function type, the
 * object class, and the three argument types, and records Command as the base.
 *
 * @tparam Function The member function type.
 * @tparam Object The class of the object.
 * @tparam Argument1 The type of the first argument.
 * @tparam Argument2 The type of the second argument.
 * @tparam Argument3 The type of the third argument.
 */
template <typename Function,
          typename Object,
          typename Argument1,
          typename Argument2,
          typename Argument3>
class MemFun3Command : public Command {
public:
    /**
     * Bind a member function and its arguments to an object.
     *
     * @param pObject The object.
     * @param pfnMember The member function.
     * @param argument1 The first argument.
     * @param argument2 The second argument.
     * @param argument3 The third argument.
     */
    MemFun3Command(Object *pObject,
                   Function pfnMember,
                   Argument1 argument1,
                   Argument2 argument2,
                   Argument3 argument3)
        : mObject(pObject), mMember(pfnMember), mArgument1(argument1), mArgument2(argument2),
          mArgument3(argument3) {
    }

    /** Call the member function with the arguments. */
    void Execute() override {
        (mObject->*mMember)(mArgument1, mArgument2, mArgument3);
    }

    Object *mObject;      /*!< The object. */
    Function mMember;     /*!< The member function. */
    Argument1 mArgument1; /*!< The first argument. */
    Argument2 mArgument2; /*!< The second argument. */
    Argument3 mArgument3; /*!< The third argument. */
};

/**
 * Command that calls a member function with three bound arguments and a fourth default argument.
 *
 * The member function takes a fourth `int` parameter whose default is -1, and the command passes
 * that default.
 *
 * @tparam Result The return type of the member function.
 * @tparam Owner The class that declares the member function.
 * @tparam Parameter1 The type of the first parameter.
 * @tparam Parameter2 The type of the second parameter.
 * @tparam Parameter3 The type of the third parameter.
 * @tparam Object The class of the object.
 * @tparam Argument1 The type of the first argument.
 * @tparam Argument2 The type of the second argument.
 * @tparam Argument3 The type of the third argument.
 */
template <typename Result,
          typename Owner,
          typename Parameter1,
          typename Parameter2,
          typename Parameter3,
          typename Object,
          typename Argument1,
          typename Argument2,
          typename Argument3>
class MemFun3Command<Result (Owner::*)(Parameter1, Parameter2, Parameter3, int),
                     Object,
                     Argument1,
                     Argument2,
                     Argument3> : public Command {
public:
    /** The member function type. */
    using Function = Result (Owner::*)(Parameter1, Parameter2, Parameter3, int);

    /** The fourth argument the member function receives. */
    static constexpr int kDefaultArgument = -1;

    /**
     * Bind a member function and its arguments to an object.
     *
     * @param pObject The object.
     * @param pfnMember The member function.
     * @param argument1 The first argument.
     * @param argument2 The second argument.
     * @param argument3 The third argument.
     */
    MemFun3Command(Object *pObject,
                   Function pfnMember,
                   Argument1 argument1,
                   Argument2 argument2,
                   Argument3 argument3)
        : mObject(pObject), mMember(pfnMember), mArgument1(argument1), mArgument2(argument2),
          mArgument3(argument3) {
    }

    /**
     * Call the member function with the arguments and the default fourth argument.
     *
     * @ghidraAddress NTSC-U/C: 0x00341298
     * @ghidraAddress PAL: 0x003ae7d0
     */
    void Execute() override {
        (mObject->*mMember)(mArgument1, mArgument2, mArgument3, kDefaultArgument);
    }

    Object *mObject;      /*!< The object. */
    Function mMember;     /*!< The member function. */
    Argument1 mArgument1; /*!< The first argument. */
    Argument2 mArgument2; /*!< The second argument. */
    Argument3 mArgument3; /*!< The third argument. */
};

/**
 * Allocate a command that calls a member function of an object with three arguments.
 *
 * @param pObject The object.
 * @param pfnMember The member function.
 * @param argument1 The first argument.
 * @param argument2 The second argument.
 * @param argument3 The third argument.
 * @return The command, with no reference taken.
 * @ghidraAddress NTSC-U/C: 0x00340d08
 * @ghidraAddress PAL: 0x003ae240
 */
template <typename Function,
          typename Object,
          typename Argument1,
          typename Argument2,
          typename Argument3>
Command *NewMemFun3Command(Object *pObject,
                           Function pfnMember,
                           Argument1 argument1,
                           Argument2 argument2,
                           Argument3 argument3) {
    return new MemFun3Command<Function, Object, Argument1, Argument2, Argument3>(
        pObject, pfnMember, argument1, argument2, argument3);
}
