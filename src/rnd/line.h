#pragma once

#include <vector>

#include "math/vector3.h"
#include "os/dbg.h"
#include "os/hxstr.h"
#include "rnd/collideable.h"
#include "rnd/drawable.h"
#include "rnd/mat.h"
#include "rnd/object.h"
#include "rnd/stream.h"
#include "rnd/transformable.h"

namespace Rnd {

/**
 * A strip drawn through a run of points.
 *
 * The RTTI records the class as `RndLine`, with the bases Rnd::Drawable, Rnd::Transformable, and
 * Rnd::Collideable. Only the members the recovered routines use are declared.
 */
class Line : public Drawable, public Transformable, public Collideable {
public:
    /** One point of the line, 0x50 bytes. */
    struct Point {
        Vector3 mPos;                    // +0x00 The position.
        unsigned char mReserved10[0x40]; // +0x10
    };

    /**
     * Release the line.
     *
     * @ghidraAddress NTSC-U/C: 0x00383c50
     * @ghidraAddress PAL: 0x003f22d8
     */
    virtual ~Line();

    /**
     * Write the line as text.
     *
     * @param sink The output.
     * @ghidraAddress NTSC-U/C: 0x0022b5f8
     * @ghidraAddress PAL: 0x00234368
     */
    virtual void DumpText(Dbg &sink);

    /**
     * Write the line to a stream.
     *
     * @param stream The stream.
     * @ghidraAddress NTSC-U/C: 0x0022b770
     * @ghidraAddress PAL: 0x002344e0
     */
    virtual void Save(Stream &stream);

    /**
     * Replace a referenced object.
     *
     * @param pFrom The object to replace.
     * @param pTo The replacement.
     * @ghidraAddress NTSC-U/C: 0x00229f60
     * @ghidraAddress PAL: 0x00232cd0
     */
    virtual void Replace(Object *pFrom, Object *pTo);

    /**
     * Report the class name.
     *
     * @return The class name.
     * @ghidraAddress NTSC-U/C: 0x003840e8
     */
    virtual const HxStr &ClassName() const;

    /**
     * Copy another line.
     *
     * @param pSource The line to copy.
     * @param nFlags The copy flags.
     * @ghidraAddress NTSC-U/C: 0x0022bba8
     * @ghidraAddress PAL: 0x002348e8
     */
    virtual void Copy(const Object *pSource, unsigned nFlags);

    /**
     * Read the line from a stream.
     *
     * @param stream The stream.
     * @ghidraAddress NTSC-U/C: 0x0022b9e0
     * @ghidraAddress PAL: 0x00234738
     */
    virtual void Load(Stream &stream);

    /**
     * Report the material the line draws with.
     *
     * @return The material.
     * @ghidraAddress NTSC-U/C: 0x00229f50
     * @ghidraAddress PAL: 0x00232cc0
     */
    Mat *GetMat() const;

    /**
     * Set the number of points and rebuild the strip.
     *
     * @param nPoints The number of points.
     * @ghidraAddress NTSC-U/C: 0x0022afe0
     * @ghidraAddress PAL: 0x00233d50
     */
    void SetNumPoints(int nPoints);

    /**
     * Move one point.
     *
     * @param nIndex The point.
     * @param pos The position.
     * @ghidraAddress NTSC-U/C: 0x0022b4b8
     * @ghidraAddress PAL: 0x00234228
     */
    void SetPoint(int nIndex, const Vector3 &pos);

    /**
     * Set the material the line draws with.
     *
     * @param pMat The material.
     * @ghidraAddress NTSC-U/C: 0x0022af80
     * @ghidraAddress PAL: 0x00233cf0
     */
    void SetMat(Mat *pMat);

    /**
     * Set the angle the strip folds at along its length.
     *
     * @param flAngle The angle in radians.
     * @ghidraAddress NTSC-U/C: 0x0022afb0
     * @ghidraAddress PAL: 0x00233d20
     */
    void SetFoldAngle(float flAngle);

    /**
     * Registered class name of Rnd::Line, the string "Line".
     *
     * @ghidraAddress NTSC-U/C: 0x003b09d4
     */
    static HxStr sClassName;

    float mWidth;               /*!< The width of the strip. */
    std::vector<Point> mPoints; /*!< The points. */
};

} // namespace Rnd
