#pragma once

#include "os/command.h"
#include "os/factory.h"

/**
 * Command that a recording saves and rebuilds through Factory<Command>.
 *
 * The RTTI includes the template with the identifier and the derived class as its arguments. Every
 * member is inline. Each instance registers the derived class under the identifier during static
 * initialisation.
 *
 * @tparam kId The identifier.
 * @tparam T The derived class, which has a constructor that takes a BinStream.
 */
template <int kId, typename T>
class SerializableCommand : public Command {
public:
    /** Construct the command. */
    SerializableCommand() {
        (void)sRegistration; // Referenced so that the registration of T is instantiated.
    }

    /**
     * Report that a recording saves the command.
     *
     * @return True.
     */
    bool IsSerializable() override {
        return true;
    }

    /**
     * Report the identifier a recording rebuilds the command by.
     *
     * @return kId.
     */
    int GetSerialId() override {
        return kId;
    }

private:
    /** Registration of T in Factory<Command> under kId. */
    class Registration {
    public:
        /** Register T. */
        Registration() {
            Factory<Command>::template Register<T>(kId);
        }
    };

    static Registration sRegistration; /*!< The registration of T. */
};

template <int kId, typename T>
typename SerializableCommand<kId, T>::Registration SerializableCommand<kId, T>::sRegistration;
