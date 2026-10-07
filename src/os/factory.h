#pragma once

#include <map>

#include "os/binstream.h"

/**
 * Registry of the classes derived from a base that a stream can rebuild, by identifier.
 *
 * The RTTI includes the template with the argument Command, and the nested FactoryUnitBase and
 * FactoryUnit. Every member is inline.
 *
 * @tparam Base The base of the classes.
 */
template <typename Base>
class Factory {
public:
    /**
     * Builder of one class.
     *
     * The RTTI includes the nested name.
     */
    class FactoryUnitBase {
    public:
        /** Release the builder. */
        virtual ~FactoryUnitBase() {
        }

        /**
         * Construct an object of the class from a stream.
         *
         * @param stream The stream to read from.
         * @return The object.
         */
        virtual Base *Create(BinStream &stream) = 0;
    };

    /**
     * Builder of the class T.
     *
     * The RTTI includes the nested name.
     *
     * @tparam T The class, which has a constructor that takes a BinStream.
     */
    template <typename T>
    class FactoryUnit : public FactoryUnitBase {
    public:
        /**
         * Construct a T from a stream.
         *
         * @param stream The stream to read from.
         * @return The object.
         */
        Base *Create(BinStream &stream) override {
            return new T(stream);
        }
    };

    /**
     * Report the builders by identifier, constructing the registry on first use.
     *
     * The registry is destroyed at exit.
     *
     * @return The builders.
     * @ghidraAddress NTSC-U/C: 0x00344f68
     * @ghidraAddress PAL: 0x003b24a0
     */
    static std::map<int, FactoryUnitBase *> &GetUnits() {
        static std::map<int, FactoryUnitBase *> units;
        return units;
    }

    /**
     * Register a builder of the class T under an identifier.
     *
     * @tparam T The class.
     * @param nId The identifier.
     */
    template <typename T>
    static void Register(int nId) {
        std::map<int, FactoryUnitBase *> &units = GetUnits();
        FactoryUnitBase *pUnit = new FactoryUnit<T>;
        units[nId] = pUnit;
    }
};
