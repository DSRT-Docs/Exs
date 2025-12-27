// src/Core/Platform/internal/AudioInfoBase.h
#ifndef EXS_INTERNAL_AUDIO_INFO_BASE_H
#define EXS_INTERNAL_AUDIO_INFO_BASE_H

#include "../../../include/Exs/Core/Types/BasicTypes.h"
#include <string>
#include <vector>
#include <cstdint>

namespace Exs {
namespace Internal {
namespace AudioInfo {

// Audio API types
enum class Exs_AudioAPI {
    Unknown = 0,
    WASAPI = 1,
    DirectSound = 2,
    ASIO = 3,
    ALSA = 4,
    PulseAudio = 5,
    CoreAudio = 6,
    OpenAL = 7,
    XAudio2 = 8,
    SDL_Audio = 9
};

// Audio device type
enum class Exs_AudioDeviceType {
    Unknown = 0,
    Output = 1,
    Input = 2,
    Duplex = 3
};

// Audio format
struct Exs_AudioFormat {
    uint32 sampleRate;      // Hz
    uint16 bitDepth;        // bits per sample
    uint16 channels;        // number of channels
    bool isFloat;           // floating point format
    bool isSigned;          // signed format
    uint32 bytesPerFrame;   // bytes per frame
};

// Audio device information
struct Exs_AudioDeviceInfo {
    std::string id;
    std::string name;
    std::string description;
    Exs_AudioDeviceType type;
    Exs_AudioAPI api;
    
    // Capabilities
    std::vector<Exs_AudioFormat> supportedFormats;
    uint32 defaultSampleRate;
    uint32 defaultChannels;
    uint32 defaultBitDepth;
    
    // Latency
    uint32 minLatency;      // milliseconds
    uint32 maxLatency;      // milliseconds
    uint32 defaultLatency;  // milliseconds
    
    // Volume and gain
    bool supportsVolumeControl;
    bool supportsMute;
    bool supportsBalance;
    
    // Spatial audio
    bool supports3DAudio;
    bool supportsSurroundSound;
    
    // Jack detection
    bool hasJackDetection;
    bool isPluggedIn;
    
    // Device state
    bool isDefault;
    bool isEnabled;
    bool isActive;
    
    // Driver information
    std::string driverName;
    std::string driverVersion;
    std::string manufacturer;
};

// Audio endpoint information
struct Exs_AudioEndpointInfo {
    std::string deviceId;
    std::string endpointId;
    std::string name;
    
    // Endpoint type
    std::string type; // Speakers, Headphones, Microphone, Line In, etc.
    
    // Position (for spatial audio)
    float x, y, z; // 3D position
    float yaw, pitch, roll; // Orientation
    
    // Physical properties
    float impedance; // Ohms
    float sensitivity; // dB
};

// Audio session information
struct Exs_AudioSessionInfo {
    uint32 processId;
    std::string processName;
    std::string sessionId;
    
    // Volume
    float volume; // 0.0 to 1.0
    bool isMuted;
    
    // Peak levels
    float peakLevelLeft;
    float peakLevelRight;
    
    // State
    bool isActive;
    bool isSystemSound;
    
    // Format
    Exs_AudioFormat format;
    
    // Latency
    uint32 latency; // milliseconds
};

// Audio statistics
struct Exs_AudioStatistics {
    // Device statistics
    uint32 deviceCount;
    uint32 activeDeviceCount;
    
    // Performance statistics
    uint64 samplesProcessed;
    uint64 bufferUnderruns;
    uint64 bufferOverruns;
    uint64 dropouts;
    
    // Latency statistics
    uint32 averageLatency;
    uint32 maximumLatency;
    uint32 minimumLatency;
    
    // CPU usage
    double cpuUsage; // percentage
    double memoryUsage; // MB
    
    // Quality metrics
    double signalToNoiseRatio; // dB
    double totalHarmonicDistortion; // percentage
};

// Audio configuration
struct Exs_AudioConfiguration {
    Exs_AudioFormat format;
    uint32 bufferSize;      // frames
    uint32 bufferCount;     // number of buffers
    uint32 sampleRateConversionQuality; // 0-100
    
    // Effects
    bool enableEqualizer;
    bool enableReverb;
    bool enableNoiseSuppression;
    bool enableEchoCancellation;
    
    // Spatial audio
    bool enable3DAudio;
    std::string spatialAudioFormat;
    
    // Volume normalization
    bool enableVolumeNormalization;
    float targetLoudness; // LUFS
};

// Base audio info class
class Exs_AudioInfoBase {
public:
    virtual ~Exs_AudioInfoBase() = default;
    
    // Device enumeration
    virtual std::vector<Exs_AudioDeviceInfo> getAudioDevices() const = 0;
    virtual std::vector<Exs_AudioDeviceInfo> getAudioDevicesByType(Exs_AudioDeviceType type) const = 0;
    virtual Exs_AudioDeviceInfo getDefaultOutputDevice() const = 0;
    virtual Exs_AudioDeviceInfo getDefaultInputDevice() const = 0;
    
    // Device information
    virtual Exs_AudioDeviceInfo getAudioDevice(const std::string& deviceId) const = 0;
    virtual bool isAudioDeviceAvailable(const std::string& deviceId) const = 0;
    
    // Endpoint information
    virtual std::vector<Exs_AudioEndpointInfo> getAudioEndpoints() const = 0;
    virtual Exs_AudioEndpointInfo getAudioEndpoint(const std::string& endpointId) const = 0;
    
    // Session information
    virtual std::vector<Exs_AudioSessionInfo> getAudioSessions() const = 0;
    virtual Exs_AudioSessionInfo getAudioSession(uint32 processId) const = 0;
    
    // Audio statistics
    virtual Exs_AudioStatistics getAudioStatistics() const = 0;
    virtual Exs_AudioStatistics getAudioStatisticsForDevice(const std::string& deviceId) const = 0;
    
    // Volume control
    virtual float getMasterVolume() const = 0;
    virtual bool setMasterVolume(float volume) = 0;
    virtual bool isMasterMuted() const = 0;
    virtual bool setMasterMute(bool muted) = 0;
    
    // Device volume control
    virtual float getDeviceVolume(const std::string& deviceId) const = 0;
    virtual bool setDeviceVolume(const std::string& deviceId, float volume) = 0;
    virtual bool isDeviceMuted(const std::string& deviceId) const = 0;
    virtual bool setDeviceMute(const std::string& deviceId, bool muted) = 0;
    
    // Session volume control
    virtual float getSessionVolume(uint32 processId) const = 0;
    virtual bool setSessionVolume(uint32 processId, float volume) = 0;
    virtual bool isSessionMuted(uint32 processId) const = 0;
    virtual bool setSessionMute(uint32 processId, bool muted) = 0;
    
    // Format information
    virtual Exs_AudioFormat getCurrentFormat() const = 0;
    virtual Exs_AudioFormat getDeviceFormat(const std::string& deviceId) const = 0;
    virtual bool setDeviceFormat(const std::string& deviceId, const Exs_AudioFormat& format) = 0;
    
    // Latency information
    virtual uint32 getCurrentLatency() const = 0;
    virtual uint32 getDeviceLatency(const std::string& deviceId) const = 0;
    virtual bool setDeviceLatency(const std::string& deviceId, uint32 latency) = 0;
    
    // Audio API information
    virtual std::vector<Exs_AudioAPI> getSupportedAPIs() const = 0;
    virtual Exs_AudioAPI getCurrentAPI() const = 0;
    virtual bool setCurrentAPI(Exs_AudioAPI api) = 0;
    
    // Configuration
    virtual Exs_AudioConfiguration getCurrentConfiguration() const = 0;
    virtual bool setConfiguration(const Exs_AudioConfiguration& config) = 0;
    
    // Peak level monitoring
    virtual float getPeakLevelLeft() const = 0;
    virtual float getPeakLevelRight() const = 0;
    virtual std::pair<float, float> getPeakLevelsForDevice(const std::string& deviceId) const = 0;
    
    // Audio effects
    virtual bool enableEqualizer(bool enable) = 0;
    virtual bool enableReverb(bool enable) = 0;
    virtual bool enableNoiseSuppression(bool enable) = 0;
    virtual bool enableEchoCancellation(bool enable) = 0;
    
    // Spatial audio
    virtual bool isSpatialAudioSupported() const = 0;
    virtual bool isSpatialAudioEnabled() const = 0;
    virtual bool enableSpatialAudio(bool enable) = 0;
    
    // Jack detection
    virtual bool isHeadphoneJackDetected() const = 0;
    virtual bool isMicrophoneJackDetected() const = 0;
    
    // Audio quality metrics
    virtual double getSignalToNoiseRatio() const = 0;
    virtual double getTotalHarmonicDistortion() const = 0;
    virtual double getFrequencyResponse() const = 0;
    
    // Audio routing
    virtual bool routeAudioToDevice(const std::string& deviceId) = 0;
    virtual std::string getCurrentRoute() const = 0;
    
    // Audio capture
    virtual bool startAudioCapture(const std::string& filePath) = 0;
    virtual bool stopAudioCapture() = 0;
    virtual bool isAudioCapturing() const = 0;
    
    // Audio playback
    virtual bool playAudioFile(const std::string& filePath) = 0;
    virtual bool stopAudioPlayback() = 0;
    virtual bool isAudioPlaying() const = 0;
};

// Factory function
Exs_AudioInfoBase* Exs_CreateAudioInfoInstance();

} // namespace AudioInfo
} // namespace Internal
} // namespace Exs

#endif // EXS_INTERNAL_AUDIO_INFO_BASE_H
