#pragma once

#include "os/command.h"

/**
 * Command that calls a member function with one argument on one object.
 *
 * The RTTI includes the template name. The object is the Command words, the object pointer, the
 * member pointer, and the argument. The addresses below are those of the `GameLogic` instance
 * with an `int` argument.
 *
 * @tparam T The class of the object.
 * @tparam A The type of the argument.
 */
template <typename T, typename A>
class MemFun1Command : public Command {
public:
    /** The member function type. */
    using Function = void (T::*)(A);

    /**
     * Bind a member function and its argument to an object.
     *
     * @param pObject The object.
     * @param pFunction The member function.
     * @param argument The argument.
     */
    MemFun1Command(T *pObject, Function pFunction, A argument)
        : mObject(pObject), mFunction(pFunction), mArgument(argument) {
    }

    /**
     * Call the member function on the object with the argument.
     *
     * @ghidraAddress NTSC-U/C: 0x00338180
     * @ghidraAddress PAL: 0x003a5730
     */
    void Execute() override {
        (mObject->*mFunction)(mArgument);
    }

private:
    T *mObject;         /*!< The object the function is called on. */
    Function mFunction; /*!< The member function. */
    A mArgument;        /*!< The argument the function is called with. */
};

/**
 * Allocate a MemFun1Command that binds a member function and its argument to an object.
 *
 * The addresses are those of the `GameLogic` instance with an `int` argument.
 *
 * @tparam T The class of the object.
 * @tparam A The type of the argument.
 * @param pObject The object.
 * @param pFunction The member function.
 * @param argument The argument.
 * @return The command, with no reference taken.
 * @ghidraAddress NTSC-U/C: 0x003379e8
 * @ghidraAddress PAL: 0x003a4f98
 */
template <typename T, typename A>
Command *NewMemFun1Command(T *pObject, void (T::*pFunction)(A), A argument) {
    return new MemFun1Command<T, A>(pObject, pFunction, argument);
}
