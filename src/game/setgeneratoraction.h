#pragma once

#include "game/triggeraction.h"
#include "math/vector2.h"
#include "math/vector3.h"
#include "rnd/generator.h"
#include "rnd/transanim.h"
#include "script/dataarray.h"

/**
 * The `set_generator` action, which changes the path and the ranges of a generator.
 *
 * The RTTI records the class as deriving from TriggerAction. The object is 0x40 bytes. Node 1 names
 * the generator. The optional values are `path`, `rate`, `scale`, and `path_var`.
 */
class SetGeneratorAction : public TriggerAction {
public:
    /**
     * Read the action.
     *
     * @param pAction The node.
     * @ghidraAddress NTSC-U/C: 0x00201630
     * @ghidraAddress PAL: 0x0020a3d0
     */
    explicit SetGeneratorAction(DataArray *pAction);

    /**
     * Write the values the node set into the generator.
     *
     * A new path covers its whole length.
     *
     * @ghidraAddress NTSC-U/C: 0x002029d8
     * @ghidraAddress PAL: 0x0020b778
     */
    void Exec() override;

private:
    // The values the node set.
    enum Flag {
        kFlagPath = 1,
        kFlagRate = 2,
        kFlagScale = 4,
        kFlagPathVar = 8,
    };

    int mFlags;
    Rnd::Generator *mGenerator;
    Rnd::TransAnim *mPath;
    Vector2 mRate;
    Vector2 mScale;
    alignas(16) Vector3 mPathVar;
};
