// src/Core/Platform/internal/InputInfoBase.h
#ifndef EXS_INTERNAL_INPUT_INFO_BASE_H
#define EXS_INTERNAL_INPUT_INFO_BASE_H

#include "../../../include/Exs/Core/Types/BasicTypes.h"
#include <string>
#include <vector>
#include <cstdint>
#include <functional>

namespace Exs {
namespace Internal {
namespace InputInfo {

// Input device types
enum class Exs_InputDeviceType {
    Unknown = 0,
    Keyboard = 1,
    Mouse = 2,
    Gamepad = 3,
    Joystick = 4,
    TouchScreen = 5,
    TouchPad = 6,
    Pen = 7,
    VRController = 8,
    MotionSensor = 9,
    SteeringWheel = 10,
    FlightStick = 11,
    DancePad = 12,
    Guitar = 13,
    DrumKit = 14
};

// Input device connection type
enum class Exs_InputConnectionType {
    Unknown = 0,
    USB = 1,
    Bluetooth = 2,
    PS2 = 3,
    Serial = 4,
    Wireless = 5,
    Internal = 6
};

// Key state enumeration
enum class Exs_KeyState {
    Up = 0,
    Down = 1,
    Pressed = 2,
    Released = 3,
    Toggled = 4
};

// Mouse button enumeration
enum class Exs_MouseButton {
    None = 0,
    Left = 1,
    Right = 2,
    Middle = 3,
    X1 = 4,
    X2 = 5
};

// Gamepad button enumeration
enum class Exs_GamepadButton {
    None = 0,
    A = 1,
    B = 2,
    X = 3,
    Y = 4,
    LeftShoulder = 5,
    RightShoulder = 6,
    LeftTrigger = 7,
    RightTrigger = 8,
    LeftThumb = 9,
    RightThumb = 10,
    DPadUp = 11,
    DPadDown = 12,
    DPadLeft = 13,
    DPadRight = 14,
    Start = 15,
    Back = 16,
    Guide = 17
};

// Input device information
struct Exs_InputDeviceInfo {
    std::string id;
    std::string name;
    std::string manufacturer;
    std::string model;
    std::string serialNumber;
    
    Exs_InputDeviceType type;
    Exs_InputConnectionType connectionType;
    
    // Capabilities
    uint32 buttonCount;
    uint32 axisCount;
    uint32 hatCount;
    bool hasForceFeedback;
    bool hasTouchInput;
    bool hasMotionSensors;
    bool hasLEDs;
    bool hasRumble;
    
    // Physical properties
    uint32 vendorId;
    uint32 productId;
    uint32 version;
    
    // State
    bool isConnected;
    bool isEnabled;
    bool isDefault;
    
    // Power
    bool isWireless;
    uint32 batteryLevel; // percentage
    bool isCharging;
    
    // Driver information
    std::string driverName;
    std::string driverVersion;
};

// Keyboard information
struct Exs_KeyboardInfo {
    Exs_InputDeviceInfo baseInfo;
    
    // Layout information
    std::string layout;
    std::string language;
    uint32 keyCount;
    
    // Special keys
    bool hasNumpad;
    bool hasFunctionKeys;
    bool hasMediaKeys;
    bool hasMacroKeys;
    bool hasBacklight;
    
    // Key rollover
    uint32 keyRollover; // NKRO, 6KRO, etc.
    
    // Switch type
    std::string switchType; // Mechanical, Membrane, etc.
    
    // Backlight
    bool hasRGB;
    uint32 backlightBrightness;
    std::string backlightColor;
};

// Mouse information
struct Exs_MouseInfo {
    Exs_InputDeviceInfo baseInfo;
    
    // Button information
    uint32 buttonCount;
    bool hasScrollWheel;
    bool hasHorizontalScroll;
    bool hasTiltWheel;
    bool hasThumbButtons;
    
    // Sensor information
    uint32 dpi;
    uint32 maxDpi;
    uint32 pollingRate;
    std::string sensorType; // Optical, Laser, etc.
    
    // Movement
    uint32 acceleration;
    uint32 sensitivity;
    
    // Surface
    bool hasSurfaceCalibration;
    std::string surfaceType;
    
    // Weight and size
    uint32 weight; // grams
    uint32 width;
    uint32 height;
    uint32 depth;
};

// Gamepad information
struct Exs_GamepadInfo {
    Exs_InputDeviceInfo baseInfo;
    
    // Button information
    uint32 buttonCount;
    uint32 analogButtonCount;
    
    // Axis information
    uint32 axisCount;
    bool hasTriggers;
    bool hasThumbsticks;
    bool hasDPad;
    
    // Vibration
    bool hasVibration;
    uint32 vibrationMotorCount;
    
    // Motion sensors
    bool hasAccelerometer;
    bool hasGyroscope;
    bool hasMagnetometer;
    
    // Touchpad
    bool hasTouchpad;
    uint32 touchpadWidth;
    uint32 touchpadHeight;
    
    // Audio
    bool hasSpeaker;
    bool hasMicrophone;
    bool hasHeadphoneJack;
    
    // Platform specific
    std::string platform; // Xbox, PlayStation, Switch, etc.
};

// Input state information
struct Exs_InputState {
    // Keyboard state
    std::vector<bool> keyStates;
    std::vector<uint32> keyPressCounts;
    
    // Mouse state
    int32 mouseX;
    int32 mouseY;
    int32 mouseDeltaX;
    int32 mouseDeltaY;
    int32 mouseWheel;
    int32 mouseWheelH;
    std::vector<bool> mouseButtonStates;
    
    // Gamepad state
    std::vector<float> gamepadAxisValues;
    std::vector<bool> gamepadButtonStates;
    std::vector<float> gamepadTriggerValues;
    
    // Touch state
    std::vector<std::tuple<int32, int32, float>> touchPoints; // x, y, pressure
    
    // Timestamp
    uint64 timestamp;
};

// Input event
struct Exs_InputEvent {
    enum class Type {
        KeyDown,
        KeyUp,
        KeyPress,
        MouseMove,
        MouseButtonDown,
        MouseButtonUp,
        MouseWheel,
        GamepadButtonDown,
        GamepadButtonUp,
        GamepadAxisMove,
        TouchDown,
        TouchUp,
        TouchMove
    };
    
    Type type;
    uint32 deviceId;
    uint32 keyCode;
    uint32 scanCode;
    uint32 modifiers;
    int32 x;
    int32 y;
    int32 deltaX;
    int32 deltaY;
    int32 wheelDelta;
    float axisValue;
    float pressure;
    uint64 timestamp;
};

// Input configuration
struct Exs_InputConfiguration {
    // Keyboard settings
    uint32 keyRepeatDelay;
    uint32 keyRepeatRate;
    bool stickyKeys;
    bool filterKeys;
    bool toggleKeys;
    
    // Mouse settings
    uint32 mouseSpeed;
    bool mouseAcceleration;
    bool mouseSwapButtons;
    bool mouseTrails;
    uint32 mouseTrailLength;
    
    // Gamepad settings
    float gamepadDeadzone;
    float gamepadSensitivity;
    bool gamepadVibration;
    uint32 gamepadVibrationStrength;
    
    // Touch settings
    float touchSensitivity;
    bool touchFeedback;
    uint32 touchFeedbackDuration;
    
    // Accessibility
    bool screenReader;
    bool magnifier;
    bool onScreenKeyboard;
    
    // Power settings
    bool wakeOnInput;
    uint32 idleTimeout;
};

// Input callback type
using Exs_InputCallback = std::function<void(const Exs_InputEvent&)>;

// Base input info class
class Exs_InputInfoBase {
public:
    virtual ~Exs_InputInfoBase() = default;
    
    // Device enumeration
    virtual std::vector<Exs_InputDeviceInfo> getInputDevices() const = 0;
    virtual std::vector<Exs_InputDeviceInfo> getInputDevicesByType(Exs_InputDeviceType type) const = 0;
    virtual Exs_InputDeviceInfo getInputDevice(const std::string& deviceId) const = 0;
    
    // Device-specific information
    virtual Exs_KeyboardInfo getKeyboardInfo() const = 0;
    virtual Exs_MouseInfo getMouseInfo() const = 0;
    virtual std::vector<Exs_GamepadInfo> getGamepadInfo() const = 0;
    virtual Exs_GamepadInfo getGamepadInfo(uint32 index) const = 0;
    
    // State querying
    virtual Exs_InputState getCurrentState() const = 0;
    virtual Exs_KeyState getKeyState(uint32 keyCode) const = 0;
    virtual bool isKeyDown(uint32 keyCode) const = 0;
    virtual bool isKeyUp(uint32 keyCode) const = 0;
    virtual bool isKeyPressed(uint32 keyCode) const = 0;
    
    // Mouse state
    virtual std::pair<int32, int32> getMousePosition() const = 0;
    virtual std::pair<int32, int32> getMouseDelta() const = 0;
    virtual bool isMouseButtonDown(Exs_MouseButton button) const = 0;
    virtual bool isMouseButtonUp(Exs_MouseButton button) const = 0;
    virtual int32 getMouseWheelDelta() const = 0;
    
    // Gamepad state
    virtual bool isGamepadConnected(uint32 index) const = 0;
    virtual bool isGamepadButtonDown(uint32 index, Exs_GamepadButton button) const = 0;
    virtual float getGamepadAxis(uint32 index, uint32 axis) const = 0;
    virtual float getGamepadTrigger(uint32 index, uint32 trigger) const = 0;
    
    // Touch state
    virtual uint32 getTouchPointCount() const = 0;
    virtual std::tuple<int32, int32, float> getTouchPoint(uint32 index) const = 0;
    
    // Event handling
    virtual void registerCallback(const Exs_InputCallback& callback) = 0;
    virtual void unregisterCallback() = 0;
    virtual bool pollEvents() = 0;
    virtual std::vector<Exs_InputEvent> getPendingEvents() = 0;
    
    // Configuration
    virtual Exs_InputConfiguration getConfiguration() const = 0;
    virtual bool setConfiguration(const Exs_InputConfiguration& config) = 0;
    
    // Device control
    virtual bool setLEDColor(const std::string& deviceId, uint32 red, uint32 green, uint32 blue) = 0;
    virtual bool setVibration(const std::string& deviceId, float leftMotor, float rightMotor) = 0;
    virtual bool setBacklightBrightness(const std::string& deviceId, uint32 brightness) = 0;
    
    // Battery information
    virtual uint32 getBatteryLevel(const std::string& deviceId) const = 0;
    virtual bool isCharging(const std::string& deviceId) const = 0;
    
    // Calibration
    virtual bool calibrateDevice(const std::string& deviceId) = 0;
    virtual bool resetCalibration(const std::string& deviceId) = 0;
    
    // Macro and binding
    virtual bool createMacro(const std::string& name, const std::vector<Exs_InputEvent>& events) = 0;
    virtual bool bindMacro(const std::string& deviceId, uint32 button, const std::string& macroName) = 0;
    virtual bool unbindMacro(const std::string& deviceId, uint32 button) = 0;
    
    // Sensitivity adjustment
    virtual bool setMouseSensitivity(uint32 sensitivity) = 0;
    virtual bool setGamepadSensitivity(uint32 index, uint32 sensitivity) = 0;
    
    // Deadzone adjustment
    virtual bool setGamepadDeadzone(uint32 index, float deadzone) = 0;
    
    // Polling rate
    virtual bool setPollingRate(const std::string& deviceId, uint32 pollingRate) = 0;
    virtual uint32 getPollingRate(const std::string& deviceId) const = 0;
    
    // Input capture
    virtual bool startInputCapture() = 0;
    virtual bool stopInputCapture() = 0;
    virtual std::vector<Exs_InputEvent> getCapturedInput() = 0;
    
    // Input blocking
    virtual bool blockInput(bool block) = 0;
    virtual bool isInputBlocked() const = 0;
    
    // Input simulation
    virtual bool simulateKeyPress(uint32 keyCode) = 0;
    virtual bool simulateKeyRelease(uint32 keyCode) = 0;
    virtual bool simulateMouseMove(int32 x, int32 y) = 0;
    virtual bool simulateMouseClick(Exs_MouseButton button) = 0;
    virtual bool simulateGamepadButtonPress(uint32 index, Exs_GamepadButton button) = 0;
    virtual bool simulateGamepadButtonRelease(uint32 index, Exs_GamepadButton button) = 0;
    
    // Hotkey management
    virtual bool registerHotkey(uint32 modifiers, uint32 keyCode, const std::function<void()>& callback) = 0;
    virtual bool unregisterHotkey(uint32 modifiers, uint32 keyCode) = 0;
    virtual bool unregisterAllHotkeys() = 0;
};

// Factory function
Exs_InputInfoBase* Exs_CreateInputInfoInstance();

} // namespace InputInfo
} // namespace Internal
} // namespace Exs

#endif // EXS_INTERNAL_INPUT_INFO_BASE_H
