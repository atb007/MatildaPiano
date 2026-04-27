#include "MatildaNeuralVoice.h"
#include "MatildaNeuralSound.h"
#include <cmath>

// Static member initialization
std::default_random_engine MatildaNeuralVoice::randomGenerator;
std::normal_distribution<float> MatildaNeuralVoice::phaseDistribution(0.0f, 1.5f);

// ============================================================================
// NeuralModel Implementation
// ============================================================================

NeuralModel::NeuralModel(const char* modelResourceName)
{
    // Create ONNX Runtime environment
    Ort::Env env{ORT_LOGGING_LEVEL_WARNING, "MatildaPiano"};
    
    // Load model from BinaryData (embedded in plugin)
    int dataSizeInBytes;
    const void* data = BinaryData::getNamedResource(modelResourceName, dataSizeInBytes);
    
    if (!data)
    {
        juce::Logger::writeToLog("ERROR: Could not load neural model: " + juce::String(modelResourceName));
        throw std::runtime_error("Neural model not found");
    }
    
    // Create session from memory
    session = Ort::Session(env, data, dataSizeInBytes, Ort::SessionOptions{nullptr});
    
    // Get input/output shapes
    inputShape = session.GetInputTypeInfo(0).GetTensorTypeAndShapeInfo().GetShape();
    outputShape = session.GetOutputTypeInfo(0).GetTensorTypeAndShapeInfo().GetShape();
    
    // Get input/output names
    Ort::AllocatorWithDefaultOptions allocator;
    auto inputNamePtr = session.GetInputNameAllocated(0, allocator);
    auto outputNamePtr = session.GetOutputNameAllocated(0, allocator);
    inputName = inputNamePtr.get();
    outputName = outputNamePtr.get();
    
    juce::Logger::writeToLog("Neural model loaded: " + juce::String(modelResourceName) + 
                             " | Input: " + juce::String(inputShape[0]) +
                             " | Output: " + juce::String(outputShape[0]));
}

void NeuralModel::eval(std::vector<float>& input, std::vector<float>& output)
{
    // Create memory info for CPU tensors
    Ort::MemoryInfo memoryInfo = Ort::MemoryInfo::CreateCpu(
        OrtAllocatorType::OrtDeviceAllocator, 
        OrtMemType::OrtMemTypeDefault
    );
    
    // Create input tensor
    auto inputTensor = Ort::Value::CreateTensor<float>(
        memoryInfo, 
        input.data(), 
        input.size(), 
        inputShape.data(), 
        inputShape.size()
    );
    
    // Create output tensor
    auto outputTensor = Ort::Value::CreateTensor<float>(
        memoryInfo, 
        output.data(), 
        output.size(), 
        outputShape.data(), 
        outputShape.size()
    );
    
    // Run inference
    std::vector<const char*> inputNames{inputName.data()};
    std::vector<const char*> outputNames{outputName.data()};
    
    session.Run(
        Ort::RunOptions{nullptr},
        inputNames.data(),
        &inputTensor,
        1,
        outputNames.data(),
        &outputTensor,
        1
    );
}

// ============================================================================
// MatildaNeuralVoice Implementation
// ============================================================================

MatildaNeuralVoice::MatildaNeuralVoice(NeuralModel* neuralModel)
    : model(neuralModel)
{
    jassert(model != nullptr);
    
    // Allocate vectors based on model output size
    size_t outputSize = model->outputShape[0];
    targetAmplitudes.resize(outputSize, 0.0f);
    currentAmplitudes.resize(outputSize, 0.0f);
    phasesLeft.resize(outputSize, 0.0f);
    phasesRight.resize(outputSize, 0.0f);
    
    // Input vector: [pitch, velocity, periodCount]
    size_t inputSize = model->inputShape[0];
    inputVector.resize(inputSize, 0.0f);
    
    // Default ADSR
    adsrParams.attack = 0.01f;
    adsrParams.decay = 0.1f;
    adsrParams.sustain = 0.8f;
    adsrParams.release = 0.5f;
    adsr.setParameters(adsrParams);
}

bool MatildaNeuralVoice::canPlaySound(juce::SynthesiserSound* sound)
{
    return dynamic_cast<MatildaNeuralSound*>(sound) != nullptr;
}

void MatildaNeuralVoice::startNote(int midiNoteNumber, float velocity, 
                                   juce::SynthesiserSound*, int)
{
    isSounding = true;
    keyIsDown = true;
    currentMidiNote = midiNoteNumber;
    currentVelocity = velocity;
    
    // Normalize MIDI note to 0-1 (A0=21 to C8=108 → 0.0 to 1.0)
    currentPitch = (currentMidiNote - 21.0f) / 87.0f;
    
    // Reset synthesis state
    sampleCounter = 0;
    tailOff = 1.0f;
    tailOffRatio = DEFAULT_TAIL_OFF_RATIO;
    currentDecay = 1.0f;
    
    // Calculate frequency and period
    float frequency = 440.0f * std::pow(2.0f, (currentMidiNote - 69.0f) / 12.0f);
    period = sampleRate / frequency;
    deltaStep = juce::MathConstants<float>::twoPi * frequency / sampleRate;
    
    // Initialize random phases for stereo spread
    for (size_t i = 0; i < phasesLeft.size(); ++i)
    {
        phasesLeft[i] = phaseDistribution(randomGenerator);
        phasesRight[i] = phaseDistribution(randomGenerator);
    }
    
    // Prepare input for neural network: [pitch, velocity, periodCount=0]
    inputVector[0] = currentPitch;
    inputVector[1] = currentVelocity;
    inputVector[2] = 0.0f;  // Initial period count
    
    // Launch async inference
    inferenceTask = std::async(std::launch::async, &MatildaNeuralVoice::runNeuralInference, this);
    
    // Wait for initial inference to complete (with timeout)
    if (inferenceTask.wait_for(std::chrono::milliseconds(100)) == std::future_status::ready)
    {
        // Copy target to current for immediate playback
        currentAmplitudes = targetAmplitudes;
    }
    
    // Start ADSR envelope
    adsr.noteOn();
}

void MatildaNeuralVoice::stopNote(float, bool allowTailOff)
{
    keyIsDown = false;
    
    if (allowTailOff)
    {
        adsr.noteOff();
    }
    else
    {
        // Immediate stop
        tailOff = 0.0f;
        adsr.reset();
        clearCurrentNote();
        isSounding = false;
    }
}

void MatildaNeuralVoice::pitchWheelMoved(int)
{
    // Pitch wheel support can be added here if needed
}

void MatildaNeuralVoice::controllerMoved(int, int)
{
    // MIDI CC support (sustain pedal, etc.) can be added here
}

void MatildaNeuralVoice::renderNextBlock(juce::AudioBuffer<float>& outputBuffer, 
                                         int startSample, int numSamples)
{
    if (!isSounding)
        return;
    
    int numChannels = outputBuffer.getNumChannels();
    
    while (--numSamples >= 0)
    {
        computeNextSample();
        
        // Get ADSR envelope value
        float adsrLevel = adsr.getNextSample();
        
        // Apply envelope and tail-off
        float finalLevel = adsrLevel * tailOff;
        
        // Add to output buffer (stereo or mono)
        if (numChannels >= 2)
        {
            outputBuffer.addSample(0, startSample, outputSample[0] * finalLevel);
            outputBuffer.addSample(1, startSample, outputSample[1] * finalLevel);
        }
        else if (numChannels == 1)
        {
            // Mono: average left and right
            float mono = (outputSample[0] + outputSample[1]) * 0.5f;
            outputBuffer.addSample(0, startSample, mono * finalLevel);
        }
        
        ++startSample;
        
        // Apply tail-off if key is released
        if (!keyIsDown && !adsr.isActive())
        {
            tailOff *= tailOffRatio;
            
            // Stop voice when tail-off is complete
            if (tailOff < 0.01f)
            {
                tailOff = 0.0f;
                clearCurrentNote();
                isSounding = false;
                break;
            }
        }
    }
}

void MatildaNeuralVoice::computeNextSample()
{
    // Update period count for neural network
    float currentPeriod = static_cast<float>(sampleCounter) / period;
    float periodCount = std::tanh(currentPeriod / MAX_PERIOD_COUNT);
    
    // Launch new inference if previous one is ready
    if (inferenceTask.valid() && 
        inferenceTask.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
    {
        // Update input for next inference
        inputVector[0] = currentPitch;
        inputVector[1] = currentVelocity;
        inputVector[2] = periodCount;
        
        // Launch new async inference
        inferenceTask = std::async(std::launch::async, &MatildaNeuralVoice::runNeuralInference, this);
    }
    
    // Smoothly interpolate current amplitudes toward target
    // This creates smooth transitions between neural network predictions
    constexpr float smoothingRate = 2000.0f;
    float smoothingFactor = smoothingRate / sampleRate;
    
    for (size_t i = 0; i < currentAmplitudes.size(); ++i)
    {
        currentAmplitudes[i] += (targetAmplitudes[i] - currentAmplitudes[i]) * smoothingFactor;
    }
    
    // Compute envelope shapes
    float attackEnvelope = std::min(6.0f * currentPeriod, 1.0f);
    float decayPart = -0.003f * currentPeriod;
    currentDecay = 1.0f / (1.0f + decayPart * decayPart);
    
    float envelope = std::min(attackEnvelope, currentDecay) * currentVelocity;
    
    // Synthesize audio from neural network amplitudes
    outputSample[0] = 0.0f;
    outputSample[1] = 0.0f;
    
    float step = static_cast<float>(sampleCounter) * deltaStep;
    
    for (size_t i = 0; i < currentAmplitudes.size(); ++i)
    {
        float partialStep = static_cast<float>(i + 1) * step;
        outputSample[0] += currentAmplitudes[i] * std::sin(phasesLeft[i] + partialStep);
        outputSample[1] += currentAmplitudes[i] * std::sin(phasesRight[i] + partialStep);
    }
    
    // Apply envelope and scaling
    outputSample[0] *= envelope * 0.7f;
    outputSample[1] *= envelope * 0.7f;
    
    ++sampleCounter;
}

void MatildaNeuralVoice::runNeuralInference()
{
    model->eval(inputVector, targetAmplitudes);
}

bool MatildaNeuralVoice::isVoiceActive() const
{
    return isSounding;
}

void MatildaNeuralVoice::setADSRParameters(float attack, float decay, float sustain, float release)
{
    adsrParams.attack = attack;
    adsrParams.decay = decay;
    adsrParams.sustain = sustain;
    adsrParams.release = release;
    adsr.setParameters(adsrParams);
}
