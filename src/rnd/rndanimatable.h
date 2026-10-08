#pragma once

#include <list>

#include "os/binstream.h"
#include "os/prnstream.h"
#include "rnd/rndobject.h"

/**
 * Mix-in for an object driven by a frame, which filters the frame and passes it on to a list of
 * other animatables.
 *
 * The RTTI includes the class name and records RndObject as a virtual base. Every entry of mAnims
 * registers this object as a referrer through RndObject::AddRef(). The filters are owned outright.
 */
class RndAnimatable : public virtual RndObject {
public:
    /** The type tags of the filters, as Save() writes them. */
    enum FilterType {
        kFilterScaleOffset = 0, /*!< ScaleOffset. */
        kFilterMinMaxLoop = 1,  /*!< MinMaxLoop. */
        kFilterZeroOrder = 2,   /*!< ZeroOrder. */
        kFilterFirstOrder = 3,  /*!< FirstOrder. */
        kFilterSecondOrder = 4, /*!< SecondOrder. */
    };

    /**
     * One stage of the frame filter chain.
     *
     * The RTTI includes the nested class name. The class declares no destructor.
     */
    class Filter {
    public:
        /**
         * Filter a frame.
         *
         * @param fValue The frame.
         * @return The filtered frame.
         */
        virtual float Apply(float fValue) = 0;

        /**
         * Undo Apply() as far as the stage allows.
         *
         * @param fValue The filtered frame.
         * @return The frame.
         */
        virtual float Unapply(float fValue) = 0;

        /**
         * Write the parameters.
         *
         * @param stream The stream to write to.
         */
        virtual void Print(PrnStream &stream) const = 0;

        /**
         * Write the parameters.
         *
         * @param stream The stream to write to.
         */
        virtual void Save(BinStream &stream) = 0;

        /**
         * Read what Save() wrote.
         *
         * @param stream The stream to read from.
         */
        virtual void Load(BinStream &stream) = 0;

        /**
         * Report the type tag.
         *
         * @return The FilterType.
         */
        virtual int Type() = 0;

        /**
         * Copy the parameters of a stage of the same type.
         *
         * @param pSource The stage to copy from.
         */
        virtual void Copy(const Filter *pSource) = 0;
    };

    /** Stage that scales the frame and adds an offset. */
    class ScaleOffset : public Filter {
    public:
        /**
         * Scale and offset a frame.
         *
         * @param fValue The frame.
         * @return The frame times mScale plus mOffset.
         * @ghidraAddress NTSC-U/C: 0x0037e770
         */
        float Apply(float fValue) override {
            return (fValue * mScale) + mOffset;
        }

        /**
         * Remove the offset and the scale.
         *
         * @param fValue The filtered frame.
         * @return The frame.
         * @ghidraAddress NTSC-U/C: 0x0037e788
         */
        float Unapply(float fValue) override {
            return (fValue - mOffset) / mScale;
        }

        /**
         * Write the scale and the offset.
         *
         * @param stream The stream to write to.
         * @ghidraAddress NTSC-U/C: 0x0021c468
         * @ghidraAddress PAL: 0x00225280
         */
        void Print(PrnStream &stream) const override;

        /**
         * Write the scale and the offset.
         *
         * @param stream The stream to write to.
         * @ghidraAddress NTSC-U/C: 0x0037e7a0
         */
        void Save(BinStream &stream) override {
            stream.WriteEndian(&mScale, sizeof(mScale));
            stream.WriteEndian(&mOffset, sizeof(mOffset));
        }

        /**
         * Read the scale and the offset.
         *
         * @param stream The stream to read from.
         * @ghidraAddress NTSC-U/C: 0x0037e800
         */
        void Load(BinStream &stream) override {
            stream.ReadEndian(&mScale, sizeof(mScale));
            stream.ReadEndian(&mOffset, sizeof(mOffset));
        }

        /**
         * Report the type tag.
         *
         * @return kFilterScaleOffset.
         * @ghidraAddress NTSC-U/C: 0x0037e850
         */
        int Type() override {
            return kFilterScaleOffset;
        }

        /**
         * Copy another stage.
         *
         * @param pSource The stage to copy from.
         * @ghidraAddress NTSC-U/C: 0x0037e858
         */
        void Copy(const Filter *pSource) override {
            *this = *static_cast<const ScaleOffset *>(pSource);
        }

        float mScale;  /*!< The multiplier. */
        float mOffset; /*!< The addend. */
    };

    /** Stage that clamps the frame to a range, or wraps it around the range. */
    class MinMaxLoop : public Filter {
    public:
        /**
         * Clamp or wrap a frame.
         *
         * @param fValue The frame.
         * @return The frame in the range.
         * @ghidraAddress NTSC-U/C: 0x0021b890
         * @ghidraAddress PAL: 0x002246a8
         */
        float Apply(float fValue) override;

        /**
         * Report the frame unchanged.
         *
         * @param fValue The filtered frame.
         * @return fValue.
         * @ghidraAddress NTSC-U/C: 0x0021b938
         * @ghidraAddress PAL: 0x00224750
         */
        float Unapply(float fValue) override;

        /**
         * Write the range and the loop flag.
         *
         * @param stream The stream to write to.
         * @ghidraAddress NTSC-U/C: 0x0021c260
         * @ghidraAddress PAL: 0x00225078
         */
        void Print(PrnStream &stream) const override;

        /**
         * Write the range and the loop flag.
         *
         * @param stream The stream to write to.
         * @ghidraAddress NTSC-U/C: 0x0037e880
         */
        void Save(BinStream &stream) override {
            stream.WriteEndian(&mMin, sizeof(mMin));
            stream.WriteEndian(&mMax, sizeof(mMax));
            const char bLoop = static_cast<char>(mLoop);
            stream.Write(&bLoop, sizeof(bLoop));
        }

        /**
         * Read the range and the loop flag.
         *
         * @param stream The stream to read from.
         * @ghidraAddress NTSC-U/C: 0x0037e900
         */
        void Load(BinStream &stream) override {
            stream.ReadEndian(&mMin, sizeof(mMin));
            stream.ReadEndian(&mMax, sizeof(mMax));
            char bLoop;
            stream.Read(&bLoop, sizeof(bLoop));
            mLoop = bLoop != 0;
        }

        /**
         * Report the type tag.
         *
         * @return kFilterMinMaxLoop.
         * @ghidraAddress NTSC-U/C: 0x0037e978
         */
        int Type() override {
            return kFilterMinMaxLoop;
        }

        /**
         * Copy another stage.
         *
         * @param pSource The stage to copy from.
         * @ghidraAddress NTSC-U/C: 0x0037e980
         * @ghidraAddress PAL: 0x003ed098
         */
        void Copy(const Filter *pSource) override {
            *this = *static_cast<const MinMaxLoop *>(pSource);
        }

        float mMin; /*!< The lower end of the range. */
        float mMax; /*!< The upper end of the range. */
        int mLoop;  /*!< Non-zero to wrap around the range rather than clamp. */
    };

    /** Stage that follows the frame by at most a fixed step a call. */
    class ZeroOrder : public Filter {
    public:
        /**
         * Step the level toward a frame.
         *
         * @param fValue The frame.
         * @return The new level.
         * @ghidraAddress NTSC-U/C: 0x0021b940
         * @ghidraAddress PAL: 0x00224758
         */
        float Apply(float fValue) override;

        /**
         * Report the level one step back from a frame.
         *
         * @param fValue The filtered frame.
         * @return The frame.
         * @ghidraAddress NTSC-U/C: 0x0021b990
         * @ghidraAddress PAL: 0x002247a8
         */
        float Unapply(float fValue) override;

        /**
         * Write the level and the step.
         *
         * @param stream The stream to write to.
         * @ghidraAddress NTSC-U/C: 0x0021c2e8
         * @ghidraAddress PAL: 0x00225100
         */
        void Print(PrnStream &stream) const override;

        /**
         * Write the level and the step.
         *
         * @param stream The stream to write to.
         * @ghidraAddress NTSC-U/C: 0x0037e9b8
         */
        void Save(BinStream &stream) override {
            stream.WriteEndian(&mLevel, sizeof(mLevel));
            stream.WriteEndian(&mMaxDelta, sizeof(mMaxDelta));
        }

        /**
         * Read the level and the step.
         *
         * @param stream The stream to read from.
         * @ghidraAddress NTSC-U/C: 0x0037ea18
         */
        void Load(BinStream &stream) override {
            stream.ReadEndian(&mLevel, sizeof(mLevel));
            stream.ReadEndian(&mMaxDelta, sizeof(mMaxDelta));
        }

        /**
         * Report the type tag.
         *
         * @return kFilterZeroOrder.
         * @ghidraAddress NTSC-U/C: 0x0037ea68
         */
        int Type() override {
            return kFilterZeroOrder;
        }

        /**
         * Copy another stage.
         *
         * @param pSource The stage to copy from.
         * @ghidraAddress NTSC-U/C: 0x0037ea70
         * @ghidraAddress PAL: 0x003ed188
         */
        void Copy(const Filter *pSource) override {
            *this = *static_cast<const ZeroOrder *>(pSource);
        }

        float mLevel;    /*!< The current level. */
        float mMaxDelta; /*!< The largest step a call. */
    };

    /** Stage that moves the level toward the frame by a fixed ratio of the gap. */
    class FirstOrder : public Filter {
    public:
        /**
         * Move the level toward a frame.
         *
         * @param fValue The frame.
         * @return The new level.
         * @ghidraAddress NTSC-U/C: 0x0037eaa0
         * @ghidraAddress PAL: 0x003ed1b8
         */
        float Apply(float fValue) override {
            mLevel += (fValue - mLevel) * mRatio;
            return mLevel;
        }

        /**
         * Report the frame that leads from the level to a filtered frame.
         *
         * @param fValue The filtered frame.
         * @return The frame.
         * @ghidraAddress NTSC-U/C: 0x0037eac0
         * @ghidraAddress PAL: 0x003ed1d8
         */
        float Unapply(float fValue) override {
            return (mLevel - (fValue * mRatio)) / (1.0f - mRatio);
        }

        /**
         * Write the level and the ratio.
         *
         * @param stream The stream to write to.
         * @ghidraAddress NTSC-U/C: 0x0021c350
         * @ghidraAddress PAL: 0x00225168
         */
        void Print(PrnStream &stream) const override;

        /**
         * Write the level and the ratio.
         *
         * @param stream The stream to write to.
         * @ghidraAddress NTSC-U/C: 0x0037eae8
         */
        void Save(BinStream &stream) override {
            stream.WriteEndian(&mLevel, sizeof(mLevel));
            stream.WriteEndian(&mRatio, sizeof(mRatio));
        }

        /**
         * Read the level and the ratio.
         *
         * @param stream The stream to read from.
         * @ghidraAddress NTSC-U/C: 0x0037eb48
         */
        void Load(BinStream &stream) override {
            stream.ReadEndian(&mLevel, sizeof(mLevel));
            stream.ReadEndian(&mRatio, sizeof(mRatio));
        }

        /**
         * Report the type tag.
         *
         * @return kFilterFirstOrder.
         * @ghidraAddress NTSC-U/C: 0x0037eb98
         */
        int Type() override {
            return kFilterFirstOrder;
        }

        /**
         * Copy another stage.
         *
         * @param pSource The stage to copy from.
         * @ghidraAddress NTSC-U/C: 0x0037eba0
         * @ghidraAddress PAL: 0x003ed2b8
         */
        void Copy(const Filter *pSource) override {
            *this = *static_cast<const FirstOrder *>(pSource);
        }

        float mLevel; /*!< The current level. */
        float mRatio; /*!< The share of the gap closed a call. */
    };

    /** Stage that follows the frame like a damped spring. */
    class SecondOrder : public Filter {
    public:
        /**
         * Construct a stage at rest.
         *
         * Only the velocity is set. The constructor has no out-of-line copy.
         */
        SecondOrder() : mVel(0.0f) {
        }

        /**
         * Advance the spring toward a frame.
         *
         * @param fValue The frame.
         * @return The new level.
         * @ghidraAddress NTSC-U/C: 0x0021b9d0
         * @ghidraAddress PAL: 0x002247e8
         */
        float Apply(float fValue) override;

        /**
         * Report the level one step back.
         *
         * @param fValue The filtered frame. The routine ignores it.
         * @return The level less the velocity.
         * @ghidraAddress NTSC-U/C: 0x0021ba08
         * @ghidraAddress PAL: 0x00224820
         */
        float Unapply(float fValue) override;

        /**
         * Write the level, the spring, the damper, and the velocity.
         *
         * @param stream The stream to write to.
         * @ghidraAddress NTSC-U/C: 0x0021c3b8
         * @ghidraAddress PAL: 0x002251d0
         */
        void Print(PrnStream &stream) const override;

        /**
         * Write the level, the spring, and the damper.
         *
         * The velocity is not written.
         *
         * @param stream The stream to write to.
         * @ghidraAddress NTSC-U/C: 0x0037ebc8
         * @ghidraAddress PAL: 0x003ed2e0
         */
        void Save(BinStream &stream) override {
            stream.WriteEndian(&mLevel, sizeof(mLevel));
            stream.WriteEndian(&mSpring, sizeof(mSpring));
            stream.WriteEndian(&mDamper, sizeof(mDamper));
        }

        /**
         * Read the level, the spring, and the damper.
         *
         * @param stream The stream to read from.
         * @ghidraAddress NTSC-U/C: 0x0037ec40
         */
        void Load(BinStream &stream) override {
            stream.ReadEndian(&mLevel, sizeof(mLevel));
            stream.ReadEndian(&mSpring, sizeof(mSpring));
            stream.ReadEndian(&mDamper, sizeof(mDamper));
        }

        /**
         * Report the type tag.
         *
         * @return kFilterSecondOrder.
         * @ghidraAddress NTSC-U/C: 0x0037eca0
         */
        int Type() override {
            return kFilterSecondOrder;
        }

        /**
         * Copy another stage.
         *
         * @param pSource The stage to copy from.
         * @ghidraAddress NTSC-U/C: 0x0037eca8
         */
        void Copy(const Filter *pSource) override {
            *this = *static_cast<const SecondOrder *>(pSource);
        }

        float mLevel;  /*!< The current level. */
        float mSpring; /*!< The pull toward the frame. */
        float mDamper; /*!< The drag on the velocity. */
        float mVel;    /*!< The current velocity. */
    };

    /**
     * Construct an animatable at frame 0 without children or filters.
     *
     * The constructor has no out-of-line copy.
     */
    RndAnimatable() : mFrame(0.0f), mFilteredFrame(0.0f) {
    }

    /**
     * Drop the references on the children and delete the filters.
     *
     * @ghidraAddress NTSC-U/C: 0x0037ed08
     * @ghidraAddress PAL: 0x003ed358
     */
    ~RndAnimatable() override;

    /**
     * Report the frame at which the animation ends.
     *
     * The base reports the latest end of the children, each taken back through its filters.
     *
     * @return The frame.
     * @ghidraAddress NTSC-U/C: 0x0021b728
     * @ghidraAddress PAL: 0x00224540
     */
    virtual float EndFrame();

    /**
     * Add the objects the animation uses to a list.
     *
     * The base body adds the objects of the children. The name is inferred.
     *
     * @param objects The list to add to.
     * @ghidraAddress NTSC-U/C: 0x0021b6a0
     * @ghidraAddress PAL: 0x002244b8
     */
    virtual void ListObjects(std::list<RndObject *> &objects);

    /**
     * Apply a filtered frame to this object alone.
     *
     * The base body does nothing. The name is inferred.
     *
     * @param fFrame The filtered frame.
     * @return Non-zero when the children are to receive the frame as well.
     * @ghidraAddress NTSC-U/C: 0x0037e758
     */
    virtual int SetFrameSelf([[maybe_unused]] float fFrame) {
        return 1;
    }

    /**
     * Write a description of the animatable.
     *
     * Nothing is written unless the dump level of the stream is above 0.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x0021ba18
     * @ghidraAddress PAL: 0x00224830
     */
    void DumpText(PrnStream &stream) override;

    /**
     * Write the filters and the names of the children.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x0021bac0
     * @ghidraAddress PAL: 0x002248d8
     */
    void Save(BinStream &stream) override;

    /**
     * Replace a child with another object.
     *
     * A replacement that is not an animatable, or null, removes the child.
     *
     * @param pFrom The object being replaced.
     * @param pTo The replacement, or null.
     * @ghidraAddress NTSC-U/C: 0x0021c4d0
     * @ghidraAddress PAL: 0x002252e8
     */
    void Replace(RndObject *pFrom, RndObject *pTo) override;

    /**
     * Copy the filters, and the children when the flags request them.
     *
     * @param pSource The animatable to copy from.
     * @param nFlags The set of fields to copy. RndObject::kCopyChildLists copies the children.
     * @ghidraAddress NTSC-U/C: 0x0021bd00
     * @ghidraAddress PAL: 0x00224b18
     */
    void Copy(const RndObject *pSource, int nFlags) override;

    /**
     * Read what Save() wrote.
     *
     * @param stream The stream to read from.
     * @ghidraAddress NTSC-U/C: 0x0021bc78
     * @ghidraAddress PAL: 0x00224a90
     */
    void Load(BinStream &stream) override;

    /**
     * Find the animatable whose children include this one.
     *
     * @return The parent, or null.
     * @ghidraAddress NTSC-U/C: 0x0021b5d8
     * @ghidraAddress PAL: 0x002243f0
     */
    RndAnimatable *Parent();

    /**
     * Filter a frame, apply it, and pass the result on to the children.
     *
     * @param fFrame The frame.
     * @ghidraAddress NTSC-U/C: 0x0021b7e8
     * @ghidraAddress PAL: 0x00224600
     */
    void SetFrame(float fFrame);

    /**
     * Run a frame through the filters.
     *
     * @param fValue The frame.
     * @return The filtered frame.
     * @ghidraAddress NTSC-U/C: 0x0021be90
     * @ghidraAddress PAL: 0x00224ca8
     */
    float FilterFrame(float fValue);

    /**
     * Run a filtered frame back through the filters, last first.
     *
     * @param fValue The filtered frame.
     * @return The frame.
     * @ghidraAddress NTSC-U/C: 0x0021bf10
     * @ghidraAddress PAL: 0x00224d28
     */
    float UnfilterFrame(float fValue);

    /**
     * Append a child.
     *
     * An animatable already among the children is not added again.
     *
     * @param pAnim The new child.
     * @return Whether the child was added.
     * @ghidraAddress NTSC-U/C: 0x0021c688
     * @ghidraAddress PAL: 0x002254a0
     */
    bool AddAnim(RndAnimatable *pAnim);

    /**
     * Remove a child.
     *
     * @param pAnim The child.
     * @ghidraAddress NTSC-U/C: 0x0021c780
     * @ghidraAddress PAL: 0x00225598
     */
    void RemoveAnim(RndAnimatable *pAnim);

    /**
     * Remove every child.
     *
     * @ghidraAddress NTSC-U/C: 0x0021c850
     * @ghidraAddress PAL: 0x00225668
     */
    void RemoveAllAnims();

    /**
     * Create a filter stage of a type, with its parameters unset.
     *
     * An unknown type warns and yields null.
     *
     * @param nType The FilterType.
     * @return The stage, or null.
     * @ghidraAddress NTSC-U/C: 0x0021bfc8
     * @ghidraAddress PAL: 0x00224de0
     */
    static Filter *NewFilter(int nType);

    /**
     * The format version Save() writes and Load() accepts.
     *
     * @ghidraAddress NTSC-U/C: 0x003b07f0
     */
    static int sRev;

    std::list<RndAnimatable *> mAnims; /*!< The children, which receive the filtered frame. */
    std::list<Filter *> mFilters;      /*!< The filter chain, applied first to last. */
    float mFrame;                      /*!< The frame SetFrame() last received. */
    float mFilteredFrame;              /*!< The frame after the filters. */

protected:
    /**
     * Drop the references on the children without emptying the list, and delete the filters.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x0021bb20
     * @ghidraAddress PAL: 0x00224938
     */
    void ReleaseObjects();

    /**
     * Take a reference on every child.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x0021bbf0
     * @ghidraAddress PAL: 0x00224a08
     */
    void AcquireAnimRefs();
};

/**
 * Write a filter stage as its type tag and its parameters.
 *
 * @param stream The stream to write to.
 * @param pFilter The stage.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x0021c0a8
 * @ghidraAddress PAL: 0x00224ec0
 */
PrnStream &operator<<(PrnStream &stream, RndAnimatable::Filter *pFilter);

/**
 * Write the name of a filter type.
 *
 * The program has no caller. An unknown type writes nothing.
 *
 * @param stream The stream to write to.
 * @param eType The type.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x0021c1a8
 * @ghidraAddress PAL: 0x00224fc0
 */
PrnStream &operator<<(PrnStream &stream, RndAnimatable::FilterType eType);

/**
 * Read a filter stage as its type tag and its parameters.
 *
 * @param stream The stream to read from.
 * @param pFilter Receives the new stage.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x0021c148
 * @ghidraAddress PAL: 0x00224f60
 */
BinStream &operator>>(BinStream &stream, RndAnimatable::Filter *&pFilter);

/**
 * Write a filter chain as its size followed by each stage's type tag and parameters.
 *
 * @param stream The stream to write to.
 * @param filters The chain.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x0037dd28
 * @ghidraAddress PAL: 0x003ec458
 */
BinStream &operator<<(BinStream &stream, const std::list<RndAnimatable::Filter *> &filters);

/**
 * Read a filter chain the chain writer wrote.
 *
 * @param stream The stream to read from.
 * @param filters Receives the chain.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x0037e050
 * @ghidraAddress PAL: 0x003ec780
 */
BinStream &operator>>(BinStream &stream, std::list<RndAnimatable::Filter *> &filters);
