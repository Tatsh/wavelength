#pragma once

#include "game/triggeraction.h"
#include "rnd/mat.h"
#include "rnd/mesh.h"
#include "script/dataarray.h"

/**
 * The `set_mesh` action, which changes the material of a mesh.
 *
 * The RTTI records the class as deriving from TriggerAction. The object is 0x18 bytes. Node 1 names
 * the mesh, and the optional `mat` the material.
 */
class SetMeshAction : public TriggerAction {
public:
    /**
     * Read the action.
     *
     * @param pAction The node.
     * @ghidraAddress NTSC-U/C: 0x00200a90
     * @ghidraAddress PAL: 0x00209830
     */
    explicit SetMeshAction(DataArray *pAction);

    /**
     * Give the mesh the material, when the node named one.
     *
     * @ghidraAddress NTSC-U/C: 0x002026a8
     */
    void Exec() override;

private:
    // The values the node set.
    enum Flag {
        kFlagMat = 1,
    };

    int mFlags;
    Rnd::Mesh *mMesh;
    Rnd::Mat *mMat;
};
