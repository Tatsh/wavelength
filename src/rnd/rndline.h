#pragma once

#include <list>
#include <vector>

#include "math/color.h"
#include "math/plane.h"
#include "math/vector2.h"
#include "math/vector3.h"
#include "os/binstream.h"
#include "os/prnstream.h"
#include "rnd/rndcollideable.h"
#include "rnd/rnddrawable.h"
#include "rnd/rndmat.h"
#include "rnd/rndmesh.h"
#include "rnd/rndtransformable.h"

/**
 * Line through points, drawn as a ribbon of constant screen width that faces the camera.
 *
 * The RTTI includes the class name and records RndDrawable, RndTransformable, and RndCollideable
 * as bases. The ribbon is a mesh the line rebuilds each time it is drawn. A line of pairs draws
 * each pair of points as a separate segment.
 */
class RndLine : public RndDrawable, public RndTransformable, public RndCollideable {
public:
    /**
     * One point and what drawing derives from it.
     *
     * The RTTI includes the nested class name.
     */
    class Point {
    public:
        /** Construct a white point at the origin. */
        Point() {
            mPoint.x = 0.0f;
            mPoint.y = 0.0f;
            mPoint.z = 0.0f;
            mColor.r = 1.0f;
            mColor.a = 1.0f;
            mColor.g = 1.0f;
            mColor.b = 1.0f;
        }

        Vector3 mPoint;    /*!< The position. */
        Color mColor;      /*!< The colour. */
        Vector3 mCamPoint; /*!< The position in camera space. */
        Vector2 mScreen;   /*!< The position projected onto the screen. */
        Vector2 mDir;      /*!< The screen direction to the next point. */
        Vector2 mOffset;   /*!< The screen offset of the ribbon's edges from the point. */
    };

    /**
     * The mesh vertices of a point.
     *
     * The structure is not polymorphic and emits no RTTI descriptor, so the name is inferred.
     */
    struct PointVerts {
        int mCaps;             /*!< The cap vertex pairs the point has, before or after it. */
        RndMesh::Vert *mVerts; /*!< The point's first vertex. */
    };

    /**
     * Construct a white, unit-width line without points, with caps and a fold at a quarter turn.
     *
     * @param pszName The registry key.
     * @ghidraAddress NTSC-U/C: 0x0022bca0
     * @ghidraAddress PAL: 0x002349e0
     */
    explicit RndLine(const char *pszName);

    /**
     * Delete the mesh.
     *
     * @ghidraAddress NTSC-U/C: 0x00383c50
     * @ghidraAddress PAL: 0x003f22d8
     */
    ~RndLine() override;

    /**
     * Add the material of the mesh to a list.
     *
     * The children are not added.
     *
     * @param objects The list to add to.
     * @ghidraAddress NTSC-U/C: 0x0022c210
     * @ghidraAddress PAL: 0x00234f50
     */
    void ListDrawObjects(std::list<RndObject *> &objects) override;

    /**
     * Add the line, when its mesh has something to draw, then the children, to a list.
     *
     * @param drawables The list to add to.
     * @ghidraAddress NTSC-U/C: 0x0022c2a8
     * @ghidraAddress PAL: 0x00234fe8
     */
    void ListDrawables(std::list<RndDrawable *> &drawables) override;

    /**
     * Set the highlight of the mesh.
     *
     * The line's own highlight is unchanged.
     *
     * @param nHighlight Non-zero to highlight.
     * @ghidraAddress NTSC-U/C: 0x0022a088
     * @ghidraAddress PAL: 0x00232df8
     */
    void SetHighlight(int nHighlight) override;

    /**
     * Build the ribbon facing the current camera and draw it.
     *
     * Points nearer than the camera's near plane are clipped. A line wholly behind it, or with
     * fewer than two points, is not drawn.
     *
     * @return 1, to draw the children.
     * @ghidraAddress NTSC-U/C: 0x0022a9f8
     * @ghidraAddress PAL: 0x00233768
     */
    int DrawShowing() override;

    using RndCollideable::Collide;

    /**
     * Add each face of the mesh the segment strikes, as strikes of the line, then what the
     * children strike, to a list.
     *
     * A hidden line adds nothing.
     *
     * @param segment The segment, in world space.
     * @param collisions The list to add to.
     * @ghidraAddress NTSC-U/C: 0x00229fc0
     * @ghidraAddress PAL: 0x00232d30
     */
    void Collide(const Segment &segment, std::list<Collision> &collisions) override;

    /**
     * Write a description of the line and its bases.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x0022b5f8
     * @ghidraAddress PAL: 0x00234368
     */
    void DumpText(PrnStream &stream) override;

    /**
     * Write the line and its bases.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x0022b770
     * @ghidraAddress PAL: 0x002344e0
     */
    void Save(BinStream &stream) override;

    /**
     * Replace a referenced object with another in the bases.
     *
     * @param pFrom The object being replaced.
     * @param pTo The replacement, or null.
     * @ghidraAddress NTSC-U/C: 0x00229f60
     * @ghidraAddress PAL: 0x00232cd0
     */
    void Replace(RndObject *pFrom, RndObject *pTo) override;

    /**
     * Report the class name.
     *
     * @return sClassName.
     * @ghidraAddress NTSC-U/C: 0x003840e8
     */
    const char *ClassName() const override {
        return sClassName;
    }

    /**
     * Copy another line and its bases, then rebuild the mesh.
     *
     * @param pSource The line to copy from.
     * @param nFlags The set of fields to copy.
     * @ghidraAddress NTSC-U/C: 0x0022bba8
     * @ghidraAddress PAL: 0x002348e8
     */
    void Copy(const RndObject *pSource, int nFlags) override;

    /**
     * Read what Save() wrote, or an earlier version of it, then rebuild the mesh.
     *
     * @param stream The stream to read from.
     * @ghidraAddress NTSC-U/C: 0x0022b9e0
     * @ghidraAddress PAL: 0x00234738
     */
    void Load(BinStream &stream) override;

    /**
     * Report the material of the mesh.
     *
     * @return The material.
     * @ghidraAddress NTSC-U/C: 0x00229f50
     * @ghidraAddress PAL: 0x00232cc0
     */
    RndMat *Mat() const;

    /**
     * Set the material of the mesh.
     *
     * @param pMat The material, or null.
     * @ghidraAddress NTSC-U/C: 0x0022af80
     * @ghidraAddress PAL: 0x00233cf0
     */
    void SetMat(RndMat *pMat);

    /**
     * Set the angle between segments at and beyond which the ribbon folds over.
     *
     * The name is inferred.
     *
     * @param fAngle The angle in radians.
     * @ghidraAddress NTSC-U/C: 0x0022afb0
     * @ghidraAddress PAL: 0x00233d20
     */
    void SetFoldAngle(float fAngle);

    /**
     * Set the number of points, then size the mesh, its texture coordinates, colours, and faces
     * to match.
     *
     * @param nPoints The number of points.
     * @ghidraAddress NTSC-U/C: 0x0022afe0
     * @ghidraAddress PAL: 0x00233d50
     */
    void SetNumPoints(int nPoints);

    /**
     * Set the position of a point.
     *
     * @param nIndex The point.
     * @param point The position.
     * @ghidraAddress NTSC-U/C: 0x0022b4b8
     * @ghidraAddress PAL: 0x00234228
     */
    void SetPoint(int nIndex, const Vector3 &point);

    /**
     * Set the colour of a point and of its mesh vertices.
     *
     * The name is inferred.
     *
     * @param nIndex The point.
     * @param color The colour.
     * @ghidraAddress NTSC-U/C: 0x0022b4d8
     * @ghidraAddress PAL: 0x00234248
     */
    void SetPointColor(int nIndex, const Color &color);

    /**
     * Find the mesh vertices of a point.
     *
     * The name is inferred.
     *
     * @param nIndex The point.
     * @param verts Receives the vertices.
     * @ghidraAddress NTSC-U/C: 0x0022aeb8
     * @ghidraAddress PAL: 0x00233c28
     */
    void FindPointVerts(int nIndex, PointVerts &verts);

    /**
     * Delete the mesh.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x0022b8a0
     * @ghidraAddress PAL: 0x002345f8
     */
    void ReleaseMesh();

    /**
     * Create the mesh for the points.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x0022b8e8
     * @ghidraAddress PAL: 0x00234640
     */
    void BuildMesh();

    /**
     * Fill the mesh positions with a ribbon through a run of points, extending the clipped points
     * outside the run to its ends.
     *
     * The name is inferred.
     *
     * @param pFirst The first point of the run.
     * @param pLast The last point of the run.
     * @ghidraAddress NTSC-U/C: 0x0022a118
     * @ghidraAddress PAL: 0x00232e88
     */
    void BuildStrip(Point *pFirst, Point *pLast);

    /**
     * Fill the mesh positions with one segment of a line of pairs, or collapse it when both
     * points are the same.
     *
     * The name is inferred.
     *
     * @param pFirst The first point.
     * @param pSecond The second point.
     * @ghidraAddress NTSC-U/C: 0x0022a650
     * @ghidraAddress PAL: 0x002333c0
     */
    void BuildPair(Point *pFirst, Point *pSecond);

    /**
     * Create a line.
     *
     * @param pszName The registry key.
     * @return The line.
     * @ghidraAddress NTSC-U/C: 0x003840f8
     * @ghidraAddress PAL: 0x003f2800
     */
    static RndObject *New(const char *pszName) {
        return new RndLine(pszName);
    }

    /**
     * The class name a `.rnd` file writes, `Line`.
     *
     * @ghidraAddress NTSC-U/C: 0x003b09d4
     */
    static const char *sClassName;

    /**
     * The format version Save() writes and Load() accepts.
     *
     * @ghidraAddress NTSC-U/C: 0x003b09d8
     */
    static int sRev;

    RndMat *mMat;               /*!< The material BuildMesh() gives the mesh. */
    float mWidth;               /*!< The width of the ribbon on the screen. */
    std::vector<Point> mPoints; /*!< The points. */
    RndMesh *mMesh;             /*!< The mesh of the ribbon. */
    int mHasCaps;               /*!< Non-zero to cap the ends of the ribbon. */
    int mLinePairs;             /*!< Non-zero to draw each pair of points as its own segment. */
    float mFoldAngle;           /*!< The angle between segments the ribbon folds over at. */
    float mFoldCos;             /*!< The cosine of mFoldAngle. */
};

/**
 * Write a point's position and colour.
 *
 * @param stream The stream to write to.
 * @param point The point.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x0022b590
 * @ghidraAddress PAL: 0x00234300
 */
PrnStream &operator<<(PrnStream &stream, const RndLine::Point &point);

/**
 * Write a point's position and colour.
 *
 * The binary expands the writer inline in the point vector writer at `0x00383608`.
 *
 * @param stream The stream to write to.
 * @param point The point.
 * @return The stream.
 */
BinStream &operator<<(BinStream &stream, const RndLine::Point &point);

/**
 * Read a point the point writer wrote.
 *
 * The binary expands the reader inline in the point vector reader at `0x00383740`.
 *
 * @param stream The stream to read from.
 * @param point Receives the point.
 * @return The stream.
 */
BinStream &operator>>(BinStream &stream, RndLine::Point &point);
