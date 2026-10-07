#pragma once

#include "os/command.h"

/**
 * Command that calls a member function of an object.
 *
 * The RTTI records each instantiation under its template arguments, the member function type and
 * the object class, and records Command as the base. Each instantiation has its own out-of-line
 * copies of the members, and the addresses listed are those of the instantiation for
 * ScratchTrack.
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
     * Call the member function.
     *
     * @ghidraAddress NTSC-U/C: 0x0033ff40
     * @ghidraAddress PAL: 0x003ad478
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
 * @param pObject The object.
 * @param pfnMember The member function.
 * @return The command, with no reference taken.
 * @ghidraAddress NTSC-U/C: 0x0033fdc8
 * @ghidraAddress PAL: 0x003ad300
 */
template <typename Function, typename Object>
Command *NewMemFunCommand(Object *pObject, Function pfnMember) {
    return new MemFunCommand<Function, Object>(pObject, pfnMember);
}
