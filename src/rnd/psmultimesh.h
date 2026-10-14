#pragma once

#include "rnd/rndmultimesh.h"

/**
 * Multimesh, PlayStation 2.
 *
 * The RTTI includes the class name and records RndMultiMesh as the one base. The class adds no
 * data member. The face packet of the mesh is sent to VU1 once for each pass, and the transforms
 * of the instances follow in batches that the instanced microprogram draws the faces with.
 */
class PsMultiMesh : public RndMultiMesh {
public:
    /**
     * Construct a multimesh.
     *
     * @param pszName The registry key.
     * @ghidraAddress NTSC-U/C: 0x0021ab00
     * @ghidraAddress PAL: 0x00223918
     */
    explicit PsMultiMesh(const char *pszName);

    /**
     * @ghidraAddress NTSC-U/C: 0x0037d920
     * @ghidraAddress PAL: 0x003ec050
     */
    ~PsMultiMesh() override {
    }

    /**
     * Draw every instance in each pass of the material of the mesh.
     *
     * Nothing draws without instances, without a mesh, or for a mesh without faces.
     *
     * @return 1, to draw the children.
     * @ghidraAddress NTSC-U/C: 0x0021aa40
     * @ghidraAddress PAL: 0x00223858
     */
    int DrawShowing() override;

    /**
     * Send the settings of the instanced microprogram and the face packet of the mesh, then the
     * transforms in batches, each batch followed by a frame flush.
     *
     * A batch is as many transforms as fit in the VU1 data memory after the face packet. The
     * transforms are sent by reference. The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x0021a4d0
     * @ghidraAddress PAL: 0x002232e8
     */
    void DrawInstances();

    /**
     * Create a multimesh.
     *
     * @param pszName The registry key.
     * @return The multimesh.
     * @ghidraAddress NTSC-U/C: 0x0037dac0
     * @ghidraAddress PAL: 0x003ec1f0
     */
    static RndObject *New(const char *pszName) {
        return new PsMultiMesh(pszName);
    }
};
