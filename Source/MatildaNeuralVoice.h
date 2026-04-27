#pragma once
#include <JuceHeader.h>
#include <future>
#include <random>
#include <vector>
#include <onnxruntime_cxx_api.h>

// Forward declarations
class NeuralModel;

/// MatildaNeuralVoice - JUCE SynthesiserVoice using ONNX neural network
/// 
/// Based on PianoForte's architecture (MIT License, Carlos Tarjano)
/// Adapted for Matilda Piano v3 with simplified pure-neural approach
/// 
/// Architecture:
/// - Neural network inference via ONNX Runtime
/// - Async inference with std::future (non-blocking audio thread)
/// - Outputs frequency-domain amplitudes → synthesized to time-domain audio
/// - Velocity-sensitive, pitch-wheel responsive
/// - Natural attack/decay/release envelopes
class MatildaNeuralVoice : public juce::SynthesiserVoice
{
public:
    MatildaNeuralVoice(NeuralModel* neuralModel);
    ~MatildaNeuralVoice() override = default;
    
    // SynthesiserVoice interface
    bool canPlaySound(juce::SynthesiserSound* sound) override;
    void startNote(int midiNoteNumber, float velocity, 
                   juce::SynthesiserSound* sound, 
                   int currentPitchWheelPosition) override;
    void stopNote(float velocity, bool allowTailOff) override;
    void pitchWheelMoved(int newPitchWheelValue) override;
    void controllerMoved(int controllerNumber, int newControllerValue) override;
    void renderNextBlock(juce::AudioBuffer<float>& outputBuffer, 
                         int startSample, int numSamples) override;
    bool isVoiceActive() const override;
    
    // ADSR control (called from PluginProcessor)
    void setADSRParameters(float attack, float decay, float sustain, float release);
    
private:
    // Neural synthesis
    void computeNextSample();
    void runNeuralInference();
    
    // Neural model reference
    NeuralModel* model = nullptr;
    
    // Voice state
    bool isSounding = false;
    bool keyIsDown = false;
    int currentMidiNote = 0;
    float currentVelocity = 0.0f;
    float currentPitch = 0.0f;  // Normalized 0-1
    
    // ADSR envelope
    juce::ADSR adsr;
    juce::ADSR::Parameters adsrParams;
    
    // Tail-off for release
    float tailOff = 1.0f;
    float tailOffRatio = 0.9997f;
    
    // Neural inference (async)
    std::future<void> inferenceTask;
    std::vector<float> inputVector;      // [pitch, velocity, periodCount]
    std::vector<float> targetAmplitudes; // Output from neural network
    std::vector<float> currentAmplitudes; // Smoothly interpolated amplitudes
    
    // Synthesis state
    float sampleRate = 44100.0f;
    long sampleCounter = 0;
    float period = 0.0f;
    float deltaStep = 0.0f;
    float currentDecay = 1.0f;
    
    // Stereo phase randomization
    std::vector<float> phasesLeft;
    std::vector<float> phasesRight;
    static std::default_random_engine randomGenerator;
    static std::normal_distribution<float> phaseDistribution;
    
    // Output buffer
    std::array<float, 2> outputSample = {0.0f, 0.0f};
    
    // Constants
    static constexpr float MAX_PERIOD_COUNT = 4511.0f;
    static constexpr float DEFAULT_TAIL_OFF_RATIO = 0.9997f;
};

/// NeuralModel - ONNX Runtime wrapper for piano synthesis
/// 
/// Manages ONNX session and performs neural network inference
/// Models are embedded in plugin binary via JUCE BinaryData
class NeuralModel
{
public:
    NeuralModel(const char* modelResourceName = "engineMain");
    
    /// Run inference: I (input) → O (output)
    void eval(std::vector<float>& input, std::vector<float>& output);
    
    // Model shape info
    std::vector<int64_t> inputShape;
    std::vector<int64_t> outputShape;
    
private:
    Ort::Session session = Ort::Session{nullptr};
    std::string inputName;
    std::string outputName;
};
