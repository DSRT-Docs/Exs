// src/Core/Platform/internal/PowerInfoBase.h
#ifndef EXS_INTERNAL_POWER_INFO_BASE_H
#define EXS_INTERNAL_POWER_INFO_BASE_H

#include "../../../include/Exs/Core/Types/BasicTypes.h"
#include <string>
#include <vector>
#include <chrono>

namespace Exs {
namespace Internal {
namespace PowerInfo {

// Power source types
enum class Exs_PowerSource {
    Unknown = 0,
    AC = 1,          // Alternating Current (wall power)
    DC = 2,          // Direct Current (battery)
    USB = 3,         // USB power
    Wireless = 4,    // Wireless charging
    Solar = 5        // Solar power
};

// Battery chemistry types
enum class Exs_BatteryChemistry {
    Unknown = 0,
    LiIon = 1,       // Lithium Ion
    LiPoly = 2,      // Lithium Polymer
    NiMH = 3,        // Nickel Metal Hydride
    NiCd = 4,        // Nickel Cadmium
    LeadAcid = 5,    // Lead Acid
    Alkaline = 6     // Alkaline
};

// Power state
enum class Exs_PowerState {
    Unknown = 0,
    Charging = 1,
    Discharging = 2,
    FullyCharged = 3,
    NotCharging = 4,
    Critical = 5
};

// Power plan types
enum class Exs_PowerPlan {
    Unknown = 0,
    HighPerformance = 1,
    Balanced = 2,
    PowerSaver = 3,
    UltimatePerformance = 4,
    Custom = 5
};

// Battery information
struct Exs_BatteryInfo {
    std::string name;
    std::string manufacturer;
    std::string serialNumber;
    std::string deviceName;
    Exs_BatteryChemistry chemistry;
    
    // Capacity information
    uint32 designCapacity;      // mWh
    uint32 fullChargeCapacity;  // mWh
    uint32 currentCapacity;     // mWh
    uint32 wearLevel;           // percentage
    
    // Voltage information
    uint32 designVoltage;       // mV
    uint32 currentVoltage;      // mV
    uint32 minVoltage;          // mV
    uint32 maxVoltage;          // mV
    
    // Current information
    int32 current;              // mA (negative for discharging)
    uint32 maxChargeCurrent;    // mA
    uint32 maxDischargeCurrent; // mA
    
    // Temperature
    int32 temperature;          // Celsius
    
    // Cycle count
    uint32 cycleCount;
    uint32 maxCycleCount;
    
    // State
    Exs_PowerState state;
    uint32 chargeLevel;         // percentage
    uint32 timeRemaining;       // minutes
    uint32 timeToFullCharge;    // minutes
    
    // Health
    bool isHealthy;
    std::string healthStatus;
    uint32 healthPercentage;    // percentage
};

// Power supply information
struct Exs_PowerSupplyInfo {
    std::string name;
    std::string manufacturer;
    std::string model;
    std::string serialNumber;
    
    // Specifications
    uint32 wattage;             // Watts
    uint32 efficiency;          // percentage (80 Plus rating)
    std::string efficiencyRating; // Bronze, Silver, Gold, Platinum, Titanium
    
    // Voltage rails
    uint32 voltage12V;          // Amps
    uint32 voltage5V;           // Amps
    uint32 voltage3_3V;         // Amps
    
    // Connections
    uint32 pcieConnectors;
    uint32 sataConnectors;
    uint32 molexConnectors;
    uint32 cpuConnectors;
    
    // Features
    bool isModular;
    bool hasFan;
    bool isFanless;
    bool hasOCP;                // Over Current Protection
    bool hasOVP;                // Over Voltage Protection
    bool hasUVP;                // Under Voltage Protection
    bool hasOTP;                // Over Temperature Protection
    bool hasSCP;                // Short Circuit Protection
    
    // Temperature
    int32 temperature;          // Celsius
    
    // Load
    uint32 currentLoad;         // percentage
    uint32 currentWattage;      // Watts
};

// Power plan settings
struct Exs_PowerPlanSettings {
    std::string name;
    std::string guid;
    Exs_PowerPlan type;
    bool isActive;
    
    // Processor settings
    uint32 processorMinState;   // percentage
    uint32 processorMaxState;   // percentage
    bool processorBoostEnabled;
    
    // Display settings
    uint32 displayTimeout;      // minutes
    uint32 displayDimTimeout;   // minutes
    uint32 displayBrightness;   // percentage
    
    // Disk settings
    uint32 diskTimeout;         // minutes
    bool diskWriteCaching;
    
    // Sleep settings
    uint32 sleepTimeout;        // minutes
    uint32 hibernateTimeout;    // minutes
    bool allowWakeTimers;
    
    // USB settings
    bool usbSelectiveSuspend;
    
    // Wireless settings
    bool wirelessPowerSave;
    
    // Graphics settings
    uint32 graphicsPowerPlan;   // 0 = Maximum Performance, 1 = Balanced, 2 = Power Saving
};

// Power event information
struct Exs_PowerEvent {
    std::chrono::system_clock::time_point timestamp;
    std::string type;           // "Sleep", "Hibernate", "Shutdown", "Restart", "Wake"
    std::string reason;
    std::string initiator;
    uint32 duration;            // milliseconds
};

// Power statistics
struct Exs_PowerStatistics {
    // Battery statistics
    uint32 totalBatteryCycles;
    uint32 averageBatteryLife;  // minutes
    uint32 estimatedBatteryLife; // minutes
    
    // Power consumption
    uint32 averagePowerConsumption; // Watts
    uint32 peakPowerConsumption;    // Watts
    uint32 idlePowerConsumption;    // Watts
    
    // Runtime statistics
    uint32 totalRuntime;        // minutes
    uint32 totalSleepTime;      // minutes
    uint32 totalHibernateTime;  // minutes
    
    // Cost statistics
    double electricityCost;     // currency per kWh
    double totalCost;           // total cost
    double estimatedMonthlyCost;
    
    // Carbon footprint
    double carbonEmissions;     // kg CO2
    double estimatedAnnualEmissions;
};

// Base power info class
class Exs_PowerInfoBase {
public:
    virtual ~Exs_PowerInfoBase() = default;
    
    // Battery information
    virtual std::vector<Exs_BatteryInfo> getBatteries() const = 0;
    virtual Exs_BatteryInfo getPrimaryBattery() const = 0;
    virtual uint32 getBatteryCount() const = 0;
    virtual bool hasBattery() const = 0;
    virtual bool isOnBatteryPower() const = 0;
    
    // Battery state
    virtual uint32 getBatteryPercentage() const = 0;
    virtual uint32 getBatteryTimeRemaining() const = 0; // minutes
    virtual uint32 getBatteryTimeToFullCharge() const = 0; // minutes
    virtual Exs_PowerState getBatteryState() const = 0;
    virtual bool isBatteryCharging() const = 0;
    virtual bool isBatteryCritical() const = 0;
    
    // Battery health
    virtual uint32 getBatteryWearLevel() const = 0; // percentage
    virtual uint32 getBatteryCycleCount() const = 0;
    virtual uint32 getBatteryHealthPercentage() const = 0;
    virtual bool isBatteryHealthy() const = 0;
    
    // Power supply information
    virtual Exs_PowerSupplyInfo getPowerSupplyInfo() const = 0;
    virtual uint32 getPowerSupplyWattage() const = 0;
    virtual std::string getPowerSupplyEfficiencyRating() const = 0;
    virtual bool isPowerSupplyHealthy() const = 0;
    
    // Power plans
    virtual std::vector<Exs_PowerPlanSettings> getPowerPlans() const = 0;
    virtual Exs_PowerPlanSettings getActivePowerPlan() const = 0;
    virtual bool setActivePowerPlan(const std::string& guid) = 0;
    virtual bool createPowerPlan(const std::string& name, const Exs_PowerPlanSettings& settings) = 0;
    virtual bool deletePowerPlan(const std::string& guid) = 0;
    
    // Power plan settings
    virtual bool setProcessorPowerSettings(uint32 minState, uint32 maxState, bool boostEnabled) = 0;
    virtual bool setDisplayPowerSettings(uint32 timeout, uint32 dimTimeout, uint32 brightness) = 0;
    virtual bool setSleepSettings(uint32 sleepTimeout, uint32 hibernateTimeout) = 0;
    virtual bool setHardDiskPowerSettings(uint32 timeout, bool writeCaching) = 0;
    
    // Power events
    virtual std::vector<Exs_PowerEvent> getPowerEvents() const = 0;
    virtual bool putComputerToSleep() = 0;
    virtual bool hibernateComputer() = 0;
    virtual bool shutdownComputer() = 0;
    virtual bool restartComputer() = 0;
    virtual bool abortShutdown() = 0;
    
    // Wake timers
    virtual bool setWakeTimer(const std::chrono::system_clock::time_point& wakeTime) = 0;
    virtual bool cancelWakeTimer() = 0;
    virtual bool isWakeTimerSet() const = 0;
    
    // Power source information
    virtual Exs_PowerSource getPowerSource() const = 0;
    virtual bool isACPowerConnected() const = 0;
    virtual bool isDCPowerConnected() const = 0;
    virtual bool isUSBPowerConnected() const = 0;
    
    // Power consumption
    virtual uint32 getCurrentPowerConsumption() const = 0; // Watts
    virtual uint32 getAveragePowerConsumption() const = 0; // Watts
    virtual uint32 getPeakPowerConsumption() const = 0;    // Watts
    
    // Temperature monitoring
    virtual int32 getBatteryTemperature() const = 0;
    virtual int32 getPowerSupplyTemperature() const = 0;
    
    // Power statistics
    virtual Exs_PowerStatistics getPowerStatistics() const = 0;
    virtual void resetPowerStatistics() = 0;
    
    // Power alerts
    virtual bool setLowBatteryAlert(uint32 percentage) = 0;
    virtual bool setCriticalBatteryAlert(uint32 percentage) = 0;
    virtual uint32 getLowBatteryAlertLevel() const = 0;
    virtual uint32 getCriticalBatteryAlertLevel() const = 0;
    
    // Power saving features
    virtual bool enablePowerSaving(bool enable) = 0;
    virtual bool isPowerSavingEnabled() const = 0;
    virtual bool enableAdaptiveBrightness(bool enable) = 0;
    virtual bool isAdaptiveBrightnessEnabled() const = 0;
    
    // USB selective suspend
    virtual bool enableUSBSelectiveSuspend(bool enable) = 0;
    virtual bool isUSBSelectiveSuspendEnabled() const = 0;
    
    // Wireless adapter power saving
    virtual bool enableWirelessPowerSaving(bool enable) = 0;
    virtual bool isWirelessPowerSavingEnabled() const = 0;
    
    // Processor power management
    virtual bool enableProcessorIdleStates(bool enable) = 0;
    virtual bool areProcessorIdleStatesEnabled() const = 0;
    virtual bool enableProcessorPerformanceStates(bool enable) = 0;
    virtual bool areProcessorPerformanceStatesEnabled() const = 0;
    
    // Graphics power management
    virtual bool setGraphicsPowerPlan(uint32 plan) = 0; // 0 = Maximum Performance, 1 = Balanced, 2 = Power Saving
    virtual uint32 getGraphicsPowerPlan() const = 0;
    
    // Network power management
    virtual bool enableNetworkPowerSaving(bool enable) = 0;
    virtual bool isNetworkPowerSavingEnabled() const = 0;
    
    // Display power management
    virtual bool setDisplayBrightness(uint32 percentage) = 0;
    virtual uint32 getDisplayBrightness() const = 0;
    virtual bool setDisplayTimeout(uint32 minutes) = 0;
    virtual uint32 getDisplayTimeout() const = 0;
    
    // Disk power management
    virtual bool enableDiskSpinDown(bool enable) = 0;
    virtual bool isDiskSpinDownEnabled() const = 0;
    
    // System cooling policy
    virtual bool setCoolingPolicy(const std::string& policy) = 0; // "Active", "Passive"
    virtual std::string getCoolingPolicy() const = 0;
    
    // Power limit settings
    virtual bool setPowerLimit(uint32 watts) = 0;
    virtual uint32 getPowerLimit() const = 0;
    
    // Battery calibration
    virtual bool calibrateBattery() = 0;
    virtual bool isBatteryCalibrationNeeded() const = 0;
    
    // Power report generation
    virtual std::string generatePowerReport() const = 0;
    virtual bool savePowerReport(const std::string& filePath) const = 0;
};

// Factory function
Exs_PowerInfoBase* Exs_CreatePowerInfoInstance();

} // namespace PowerInfo
} // namespace Internal
} // namespace Exs

#endif // EXS_INTERNAL_POWER_INFO_BASE_H
