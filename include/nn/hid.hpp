/**
 * @file hid.hpp
 * @brief Functions that help process gamepad inputs.
 */

#pragma once

#include "time.h"
#include "types.h"
#include "util.h"

namespace nn {
namespace hid {
    /// HidControllerKeys
    typedef enum {
        KEY_A = BIT(0),              ///< A
        KEY_B = BIT(1),              ///< B
        KEY_X = BIT(2),              ///< X
        KEY_Y = BIT(3),              ///< Y
        KEY_LSTICK = BIT(4),         ///< Left Stick Button
        KEY_RSTICK = BIT(5),         ///< Right Stick Button
        KEY_L = BIT(6),              ///< L
        KEY_R = BIT(7),              ///< R
        KEY_ZL = BIT(8),             ///< ZL
        KEY_ZR = BIT(9),             ///< ZR
        KEY_PLUS = BIT(10),          ///< Plus
        KEY_MINUS = BIT(11),         ///< Minus
        KEY_DLEFT = BIT(12),         ///< D-Pad Left
        KEY_DUP = BIT(13),           ///< D-Pad Up
        KEY_DRIGHT = BIT(14),        ///< D-Pad Right
        KEY_DDOWN = BIT(15),         ///< D-Pad Down
        KEY_LSTICK_LEFT = BIT(16),   ///< Left Stick Left
        KEY_LSTICK_UP = BIT(17),     ///< Left Stick Up
        KEY_LSTICK_RIGHT = BIT(18),  ///< Left Stick Right
        KEY_LSTICK_DOWN = BIT(19),   ///< Left Stick Down
        KEY_RSTICK_LEFT = BIT(20),   ///< Right Stick Left
        KEY_RSTICK_UP = BIT(21),     ///< Right Stick Up
        KEY_RSTICK_RIGHT = BIT(22),  ///< Right Stick Right
        KEY_RSTICK_DOWN = BIT(23),   ///< Right Stick Down
        KEY_SL_LEFT = BIT(24),       ///< SL on Left Joy-Con
        KEY_SR_LEFT = BIT(25),       ///< SR on Left Joy-Con
        KEY_SL_RIGHT = BIT(26),      ///< SL on Right Joy-Con
        KEY_SR_RIGHT = BIT(27),      ///< SR on Right Joy-Con

        // Pseudo-key for at least one finger on the touch screen
        KEY_TOUCH = BIT(28),

        // Buttons by orientation (for single Joy-Con), also works with Joy-Con pairs, Pro Controller
        KEY_JOYCON_RIGHT = BIT(0),
        KEY_JOYCON_DOWN = BIT(1),
        KEY_JOYCON_UP = BIT(2),
        KEY_JOYCON_LEFT = BIT(3),

        // Generic catch-all directions, also works for single Joy-Con
        KEY_UP = KEY_DUP | KEY_LSTICK_UP | KEY_RSTICK_UP,              ///< D-Pad Up or Sticks Up
        KEY_DOWN = KEY_DDOWN | KEY_LSTICK_DOWN | KEY_RSTICK_DOWN,      ///< D-Pad Down or Sticks Down
        KEY_LEFT = KEY_DLEFT | KEY_LSTICK_LEFT | KEY_RSTICK_LEFT,      ///< D-Pad Left or Sticks Left
        KEY_RIGHT = KEY_DRIGHT | KEY_LSTICK_RIGHT | KEY_RSTICK_RIGHT,  ///< D-Pad Right or Sticks Right
        KEY_SL = KEY_SL_LEFT | KEY_SL_RIGHT,                           ///< SL on Left or Right Joy-Con
        KEY_SR = KEY_SR_LEFT | KEY_SR_RIGHT,                           ///< SR on Left or Right Joy-Con
    } HidControllerKeys;

    // NpadFlags
    typedef enum {
        NPAD_CONNECTED = BIT(0),
        NPAD_WIRED = BIT(1),
    } NpadFlags;

    /// NpadId
    typedef enum {
        CONTROLLER_PLAYER_1 = 0,
        CONTROLLER_PLAYER_2 = 1,
        CONTROLLER_PLAYER_3 = 2,
        CONTROLLER_PLAYER_4 = 3,
        CONTROLLER_PLAYER_5 = 4,
        CONTROLLER_PLAYER_6 = 5,
        CONTROLLER_PLAYER_7 = 6,
        CONTROLLER_PLAYER_8 = 7,
        CONTROLLER_HANDHELD = 0x20,
    } NpadId;

    struct NpadHandheldState {
        s64 updateCount;
        u64 Buttons;
        s32 LStickX;
        s32 LStickY;
        s32 RStickX;
        s32 RStickY;
        u32 Flags;
    };
    // Seems to be the same?
    struct NpadFullKeyState : NpadHandheldState {};
    struct NpadJoyDualState : NpadHandheldState {};

    struct NpadStyleTag;
    struct NpadStyleSet {
        u32 flags;
    };
    constexpr nn::hid::NpadStyleSet NpadStyleFullKey = {BIT(0)};
    constexpr nn::hid::NpadStyleSet NpadStyleHandheld = {BIT(1)};
    constexpr nn::hid::NpadStyleSet NpadStyleJoyDual = {BIT(2)};
    constexpr nn::hid::NpadStyleSet NpadStyleJoyLeft = {BIT(3)};
    constexpr nn::hid::NpadStyleSet NpadStyleJoyRight = {BIT(4)};

    struct ControllerSupportArg {
        u8 mMinPlayerCount;
        u8 mMaxPlayerCount;
        u8 mTakeOverConnection;
        bool mLeftJustify;
        bool mPermitJoyconDual;
        bool mSingleMode;
        bool mUseColors;
        nn::util::Color4u8 mColors[4];
        u8 mUsingControllerNames;
        char mControllerNames[4][0x81];
    };

    struct ControllerSupportResultInfo {
        int mPlayerCount;
        int mSelectedId;
    };

    struct GesturePoint {
        s32 x;
        s32 y;
    };

    enum GestureDirection {
        GESTUREDIRECTION_NONE,
        GESTUREDIRECTION_LEFT,
        GESTUREDIRECTION_UP,
        GESTUREDIRECTION_RIGHT,
        GESTUREDIRECTION_DOWN,
    };

    enum GestureType {
        GESTURETYPE_IDLE,
        GESTURETYPE_COMPLETE,
        GESTURETYPE_CANCEL,
        GESTURETYPE_TOUCH,
        GESTURETYPE_PRESS,
        GESTURETYPE_TAP,
        GESTURETYPE_PAN,
        GESTURETYPE_SWIPE,
        GESTURETYPE_PINCH,
        GESTURETYPE_ROTATE,
    };

    enum GyroscopeZeroDriftMode {
        GyroscopeZeroDriftMode_Loose,
        GyroscopeZeroDriftMode_Standard,
        GyroscopeZeroDriftMode_Tight,
    };

    struct SixAxisSensorHandle {
        u32 handle;
    };

    struct GestureState {
        s64 updateNum;
        s64 detectionNum;
        s32 type;
        s32 direction;
        s32 x;
        s32 y;
        s32 deltaX;
        s32 deltaY;
        ::nn::util::Float2 velocity;
        u32 attributes;
        float scale;
        float rotationAngle;
        s32 pointCount;
        GesturePoint points[4];
    };

    struct DirectionState {
        ::nn::util::Float3 x;
        ::nn::util::Float3 y;
        ::nn::util::Float3 z;
    };

    struct SixAxisSensorState {
        ::nn::TimeSpan deltaUpdateTime;
        s64 updateNum;
        ::nn::util::Float3 acceleration;
        ::nn::util::Float3 angularVelocity;
        ::nn::util::Float3 angle;
        DirectionState direction;
        u32 attributes;
    };

    void InitializeNpad();
    void SetSupportedNpadIdType(u32 const*, u64);
    void SetSupportedNpadStyleSet(nn::util::BitFlagSet<32, nn::hid::NpadStyleTag>);
    NpadStyleSet GetNpadStyleSet(u32 const&);
    // returns the number of states put into the array
    int GetNpadStates(nn::hid::NpadHandheldState* outArray, s32 count, u32 const& NpadId);
    int GetNpadStates(nn::hid::NpadFullKeyState* outArray, s32 count, u32 const& NpadId);
    int GetNpadStates(nn::hid::NpadJoyDualState* outArray, s32 count, u32 const& NpadId);
    void GetNpadState(nn::hid::NpadHandheldState* out, u32 const& NpadId);
    void GetNpadState(nn::hid::NpadFullKeyState* out, u32 const& NpadId);
    void GetNpadState(nn::hid::NpadJoyDualState* out, u32 const& NpadId);
    int GetGestureStates(GestureState* outArray, int count);
    int GetSixAxisSensorHandles(SixAxisSensorHandle* pOutValues, int count, u32 const& NpadId,
                                NpadStyleSet style);
    void StartSixAxisSensor(const SixAxisSensorHandle& handle);
    void StopSixAxisSensor(const SixAxisSensorHandle& handle);
    bool IsSixAxisSensorAtRest(const SixAxisSensorHandle& handle);
    void GetSixAxisSensorState(SixAxisSensorState* outValue, const SixAxisSensorHandle& handle);
    int GetSixAxisSensorStates(SixAxisSensorState* outStates, int count, const SixAxisSensorHandle& handle);
    bool IsSixAxisSensorFusionEnabled(const SixAxisSensorHandle& handle);
    void EnableSixAxisSensorFusion(const SixAxisSensorHandle& handle, bool enable);
    void SetGyroscopeZeroDriftMode(const SixAxisSensorHandle& handle, const GyroscopeZeroDriftMode& mode);
    GyroscopeZeroDriftMode GetGyroscopeZeroDriftMode(const SixAxisSensorHandle& handle);
    int ShowControllerSupport(nn::hid::ControllerSupportResultInfo*, ControllerSupportArg const&);

};  // namespace hid
};  // namespace nn
