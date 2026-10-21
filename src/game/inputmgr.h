#pragma once

#include <cstddef>
#include <memory>
#include <vector>

#include "app/msgsink.h"
#include "game/expressionstick.h"
#include "game/world.h"
#include "msg/joypadinputmsg.h"
#include "os/mem.h"
#include "os/timer.h"

/**
 * Router of controller input to a running world.
 *
 * The RTTI records the class as deriving from MsgSink. The object is 0x34 bytes, and WorldMgr
 * allocates it under the tag "InputMgr". The router turns each button of a controller into the
 * input event of its bound action, repeats a rotation while its button is down, and passes the
 * analogue sticks to the world every frame.
 */
class InputMgr : public MsgSink {
public:
    /**
     * Input state of one controller.
     *
     * The RTTI includes the class name.
     */
    class ControllerData {
    public:
        /** Construct the state with a stopped repeat timer. */
        ControllerData() {
            mRepeatTimer.mCycles = 0;
            mRepeatTimer.mLastMs = 0.0f;
            mRepeatTimer.mRunning = 0;
        }

        /** @return The stick that bends the notes of the player. */
        ExpressionStick *GetStick() {
            return mStick.get();
        }

        /** @return The stick that bends the notes of the player. */
        const ExpressionStick *GetStick() const {
            return mStick.get();
        }

        /**
         * Take the stick that bends the notes of the player.
         *
         * @param pStick The stick. The state deletes it afterwards.
         */
        void SetStick(std::unique_ptr<ExpressionStick> pStick) {
            mStick = std::move(pStick);
        }

        /** Release the stick at the point the destructor originally deleted it. */
        void ResetStick() {
            mStick.reset();
        }

        /** @return The stick whose position goes to the world every frame. */
        int GetPositionStick() const {
            return mPositionStick;
        }

        /**
         * Set the stick whose position goes to the world every frame.
         *
         * @param nPositionStick The stick.
         */
        void SetPositionStick(int nPositionStick) {
            mPositionStick = nPositionStick;
        }

        /** @return The time a rotation button has been down. */
        Timer &GetRepeatTimer() {
            return mRepeatTimer;
        }

        /** @return mRepeatTimer in milliseconds at the last repeat check. */
        float GetRepeatMs() const {
            return mRepeatMs;
        }

        /**
         * Set mRepeatTimer in milliseconds at the last repeat check.
         *
         * @param fRepeatMs The time.
         */
        void SetRepeatMs(float fRepeatMs) {
            mRepeatMs = fRepeatMs;
        }

        /** @return The direction of the rotation that repeats. */
        int GetDirection() const {
            return mDirection;
        }

        /**
         * Set the direction of the rotation that repeats.
         *
         * @param nDirection The direction.
         */
        void SetDirection(int nDirection) {
            mDirection = nDirection;
        }

    private:
        std::unique_ptr<ExpressionStick> mStick; /*!< The stick that bends the notes. */
        int mPositionStick; /*!< The stick whose position goes to the world every frame. */
        Timer mRepeatTimer; /*!< The time a rotation button has been down. */
        float mRepeatMs;    /*!< mRepeatTimer in milliseconds at the last repeat check. */
        int mDirection = 0; /*!< The direction of the rotation that repeats. */
    };

    /**
     * Construct a router for one world.
     *
     * @param pWorld The world input goes to.
     * @ghidraAddress NTSC-U/C: 0x00118068
     * @ghidraAddress PAL: 0x00119800
     */
    explicit InputMgr(World *pWorld);

    /**
     * Allocate a router from the pool heaps under the tag "InputMgr".
     *
     * No out-of-line body exists. WorldMgr::Start() inlines the call with the default alignment.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "InputMgr", 0);
    }

    /**
     * Release the router.
     *
     * @ghidraAddress NTSC-U/C: 0x001185a8
     * @ghidraAddress PAL: 0x00119d40
     */
    ~InputMgr() override;

    /**
     * Route the controller input of one frame to the world.
     *
     * @ghidraAddress NTSC-U/C: 0x00118730
     * @ghidraAddress PAL: 0x00119ec8
     */
    void Poll();

    /**
     * Repeat the rotation of a controller once each repeat interval after the initial delay.
     *
     * @param nPad The controller.
     * @ghidraAddress NTSC-U/C: 0x00118848
     * @ghidraAddress PAL: 0x00119fe0
     */
    void RepeatRotation(int nPad);

    /**
     * Turn a button of a controller into the input event of its bound action.
     *
     * A button in demo mode stops the world instead.
     *
     * @param pMsg The message of the button.
     * @return False, for every message.
     * @ghidraAddress NTSC-U/C: 0x001189d8
     * @ghidraAddress PAL: 0x0011a170
     */
    bool HandleButton(JoypadInputMsg *pMsg);

    /**
     * Act on a message sent to the router.
     *
     * @param pMsg The message.
     * @return The result of HandleButton() for a controller button, otherwise false.
     * @ghidraAddress NTSC-U/C: 0x00118d90
     * @ghidraAddress PAL: 0x0011a528
     */
    bool DispatchPriv(Message *pMsg) override;

private:
    World *mWorld;                            /*!< The world input goes to. */
    std::vector<ControllerData> mControllers; /*!< The state of each controller. */
    std::vector<int> mLocalPlayers;           /*!< The player of each controller. */
    float mRepeatInitialDelayMs;              /*!< The time before the first repeat. */
    float mRepeatDelayMs;                     /*!< The time between repeats. */
    int mDemo;                                /*!< Whether a demo plays. */
};
