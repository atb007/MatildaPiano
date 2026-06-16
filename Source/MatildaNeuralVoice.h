#pragma once
#include <JuceHeader.h>
#include <atomic>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <random>
#include <vector>
#include <onnxruntime_cxx_api.h>
#include "SchuckYoungPartials.h"

class NeuralModel;
class MatildaNeuralVoice;

class NeuralInferenceScheduler
{
public:
    struct Job
    {
        NeuralModel* model = nullptr;
        MatildaNeuralVoice* voice = nullptr;
        std::vector<float> input;
        uint32_t generation = 0;
    };

    NeuralInferenceScheduler();
    ~NeuralInferenceScheduler();

    void start();
    void stop();
    bool isRunning() const;
    void beginAudioBlock(int incomingNoteOnEvents);
    bool tryEnqueue(Job job);

private:
    class WorkerThread : public juce::Thread
    {
    public:
        explicit WorkerThread(NeuralInferenceScheduler& owner);
        void run() override;

    private:
        NeuralInferenceScheduler& owner;
    };

    void runWorkerLoop();

    WorkerThread worker;
    std::mutex queueMutex;
    std::condition_variable queueCondition;
    std::deque<Job> jobQueue;
    std::atomic<bool> shouldStop { false };
    int jobsEnqueuedThisBlock = 0;
    int maxJobsThisBlock = 32;
    static constexpr size_t maxQueueSize = 64;
};

class MatildaNeuralVoice : public juce::SynthesiserVoice
{
public:
    MatildaNeuralVoice(NeuralModel* neuralModel, NeuralInferenceScheduler* scheduler);
    ~MatildaNeuralVoice() override;
    
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
    
    void setADSRParameters(float attack, float decay, float sustain, float release);
    void setSampleRate(double sampleRate);
    void setInharmonicity(float normalized01);
    void setPerformanceMorph(float hardness01, float cabinetResonance01);
    void setSustainPedalDown(bool isDown);

    int getCurrentMidiNote() const { return currentMidiNote; }
    void acceptInferenceResult(std::vector<float>&& output, uint32_t generation);
    
private:
    friend class NeuralInferenceScheduler;

    void computeNextSample();
    void harvestInferenceResults();
    void launchInference(uint32_t generation, bool force);
    void updateInferenceInput(float periodCount);
    void seedDefaultAmplitudes();
    void recomputePartialPhaseIncrements(bool force);
    float computePhysicalBlendSample(int channelIndex);
    static void clampAmplitudes(std::vector<float>& amplitudes);
    static void normalizeAmplitudeEnergy(std::vector<float>& amplitudes, float maxAbsSum);
    static float amplitudeAbsSum(const std::vector<float>& amplitudes, size_t count);
    
    NeuralModel* model = nullptr;
    NeuralInferenceScheduler* scheduler = nullptr;
    
    bool isSounding = false;
    bool keyIsDown = false;
    int currentMidiNote = 0;
    float currentVelocity = 0.0f;
    float currentPitch = 0.0f;
    
    juce::ADSR adsr;
    juce::ADSR::Parameters adsrParams;
    
    float tailOff = 1.0f;
    float tailOffRatio = 0.9997f;
    
    std::atomic<bool> inferenceRunning { false };
    std::atomic<uint32_t> inferenceGeneration { 0 };
    int samplesUntilNextInference = 0;
    
    std::vector<float> inputVector;
    std::vector<float> targetAmplitudes;
    std::vector<float> currentAmplitudes;
    
    std::mutex pendingMutex;
    std::vector<float> pendingAmplitudes;
    uint32_t pendingGeneration = 0;
    bool pendingReady = false;
    
    float sampleRate = 44100.0f;
    long sampleCounter = 0;
    long samplesSinceNoteOn = 0;
    float period = 1.0f;
    float deltaStep = 0.0f;
    float currentDecay = 1.0f;
    float inharmonicityBeta = SchuckYoung::kReferenceBeta;
    float targetInharmonicityBeta = SchuckYoung::kReferenceBeta;
    float hardnessMorph = 0.5f;
    float cabinetResonance = 0.5f;
    bool sustainPedalDown = false;
    int activePartialCount = 30;
    std::vector<float> partialPhaseIncrements;
    float physicalPhaseScale = 0.0f;
    size_t physicalTimeStep = 0;
    std::vector<float> physicalHarmonics;
    std::vector<float> physicalAmplitudes;
    std::vector<float> physicalPhasesLeft;
    std::vector<float> physicalPhasesRight;
    
    std::vector<float> phasesLeft;
    std::vector<float> phasesRight;
    static std::default_random_engine randomGenerator;
    static std::normal_distribution<float> phaseDistribution;
    
    std::array<float, 2> outputSample = {0.0f, 0.0f};
    
    static constexpr float MAX_PERIOD_COUNT = 4511.0f;
    static constexpr float DEFAULT_TAIL_OFF_RATIO = 0.9997f;
    static constexpr int INFERENCE_RELAUNCH_SAMPLES = 4096;
    static constexpr int INFERENCE_HANDOFF_DELAY_SAMPLES = 2048;
    static constexpr int INFERENCE_BLEND_RAMP_SAMPLES = 8192;
};

class NeuralModel
{
public:
    NeuralModel(const char* modelResourceName = "engineMain");
    void eval(std::vector<float>& input, std::vector<float>& output);
    
    std::vector<int64_t> inputShape;
    std::vector<int64_t> outputShape;
    
private:
    Ort::Env env{ORT_LOGGING_LEVEL_WARNING, "MatildaPiano"};
    Ort::SessionOptions sessionOptions;
    Ort::Session session{nullptr};
    std::mutex inferenceMutex;
    std::string inputName;
    std::string outputName;
};
