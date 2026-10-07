#pragma once

#include "os/command.h"

/**
 * Command that calls a member function with no arguments on one object.
 *
 * The RTTI includes the template name. The object is the Command words, the object pointer, and
 * the member pointer. The addresses below are those of the `Player` instance.
 *
 * @tparam T The class of the object.
 */
template <typename T>
class MemFunCommand : public Command {
public:
    /** The member function type. */
    using Function = void (T::*)();

    /**
     * Bind a member function to an object.
     *
     * @param pObject The object.
     * @param pFunction The member function.
     */
    MemFunCommand(T *pObject, Function pFunction) : mObject(pObject), mFunction(pFunction) {
    }

    /**
     * Call the member function on the object.
     *
     * @ghidraAddress NTSC-U/C: 0x0033dc50
     * @ghidraAddress PAL: 0x003ab188
     */
    void Execute() override {
        (mObject->*mFunction)();
    }

private:
    T *mObject;         /*!< The object the function is called on. */
    Function mFunction; /*!< The member function. */
};

/**
 * Allocate a MemFunCommand that binds a member function to an object.
 *
 * The addresses are those of the `Player` instance.
 *
 * @tparam T The class of the object.
 * @param pObject The object.
 * @param pFunction The member function.
 * @return The command, with no reference taken.
 * @ghidraAddress NTSC-U/C: 0x0033db28
 * @ghidraAddress PAL: 0x003ab060
 */
template <typename T>
Command *NewMemFunCommand(T *pObject, void (T::*pFunction)()) {
    return new MemFunCommand<T>(pObject, pFunction);
}
