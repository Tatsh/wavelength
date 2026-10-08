#pragma once

#include "game/triggeraction.h"
#include "rnd/generator.h"
#include "script/dataarray.h"

/**
 * The `generate` action, which makes a generator create one instance.
 *
 * The RTTI records the class as deriving from TriggerAction. The object is 0x10 bytes. Node 1 names
 * the generator.
 */
class GenerateAction : public TriggerAction {
public:
    /**
     * Read the action.
     *
     * @param pAction The node.
     * @ghidraAddress NTSC-U/C: 0x00200740
     * @ghidraAddress PAL: 0x002094e0
     */
    explicit GenerateAction(DataArray *pAction);

    /**
     * Create an instance at the current frame of the generator.
     *
     * @ghidraAddress NTSC-U/C: 0x00202b80
     * @ghidraAddress PAL: 0x0020b920
     */
    void Exec() override;

private:
    Rnd::Generator *mGenerator;
};
