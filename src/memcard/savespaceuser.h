#pragma once

/**
 * Receiver of the space a save needs, from the tasks that measure the space.
 *
 * The RTTI includes the class name and records no base. The vptr is the only member, and the one
 * method has no body in this class.
 */
class SaveSpaceUser {
public:
    /**
     * Learn the outcome of the measurement.
     *
     * MemcardTask::sStatus is kStatusOk when the save fits.
     *
     * @param nNeeded The kilobytes the save needs.
     */
    virtual void OnSaveSpace(int nNeeded) = 0;
};
