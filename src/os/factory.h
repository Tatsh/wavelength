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
        /**
         * Release the builder.
         *
         * @ghidraAddress NTSC-U/C: 0x003477b0
         * @ghidraAddress PAL: 0x003b4b78
         */
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
         * Release the builder.
         *
         * The addresses are of the builders of the input commands of World, in the order of the
         * events RotateEvent, PlayNoteEvent, BtnEvent<8>, StickEvent<2>, StickEvent<6>,
         * BtnEvent<3>, BtnEvent<4>, BtnEvent<5>, ChangeSectionEvent, BtnEvent<9>, and BtnEvent<10>.
         *
         * @ghidraAddress NTSC-U/C: 0x00346c78
         * @ghidraAddress PAL: 0x003b40c0
         * @ghidraAddress NTSC-U/C: 0x00346cf8
         * @ghidraAddress PAL: 0x003b4140
         * @ghidraAddress NTSC-U/C: 0x00346d78
         * @ghidraAddress PAL: 0x003b41c0
         * @ghidraAddress NTSC-U/C: 0x00346df8
         * @ghidraAddress PAL: 0x003b4240
         * @ghidraAddress NTSC-U/C: 0x00346e78
         * @ghidraAddress PAL: 0x003b42c0
         * @ghidraAddress NTSC-U/C: 0x00346ef8
         * @ghidraAddress PAL: 0x003b4340
         * @ghidraAddress NTSC-U/C: 0x00346f78
         * @ghidraAddress PAL: 0x003b43c0
         * @ghidraAddress NTSC-U/C: 0x00346ff8
         * @ghidraAddress PAL: 0x003b4440
         * @ghidraAddress NTSC-U/C: 0x00347078
         * @ghidraAddress PAL: 0x003b44c0
         * @ghidraAddress NTSC-U/C: 0x003470f8
         * @ghidraAddress PAL: 0x003b4540
         * @ghidraAddress NTSC-U/C: 0x00347178
         * @ghidraAddress PAL: 0x003b45c0
         */
        ~FactoryUnit() override = default;

        /**
         * Construct a T from a stream.
         *
         * The addresses are in the order of those of the destructor.
         *
         * @param stream The stream to read from.
         * @return The object.
         * @ghidraAddress NTSC-U/C: 0x00347718
         * @ghidraAddress PAL: 0x003b4ae0
         * @ghidraAddress NTSC-U/C: 0x003476c0
         * @ghidraAddress PAL: 0x003b4a88
         * @ghidraAddress NTSC-U/C: 0x00347658
         * @ghidraAddress NTSC-U/C: 0x003475d0
         * @ghidraAddress NTSC-U/C: 0x00347548
         * @ghidraAddress NTSC-U/C: 0x003474e0
         * @ghidraAddress NTSC-U/C: 0x00347478
         * @ghidraAddress NTSC-U/C: 0x00347410
         * @ghidraAddress NTSC-U/C: 0x003473b8
         * @ghidraAddress PAL: 0x003b47e0
         * @ghidraAddress NTSC-U/C: 0x00347350
         * @ghidraAddress NTSC-U/C: 0x003472e8
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
