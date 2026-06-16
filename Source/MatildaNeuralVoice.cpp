#include "MatildaNeuralVoice.h"
#include "MatildaNeuralSound.h"
#include <cmath>

std::default_random_engine MatildaNeuralVoice::randomGenerator;
std::normal_distribution<float> MatildaNeuralVoice::phaseDistribution(0.0f, 1.5f);

NeuralInferenceScheduler::WorkerThread::WorkerThread(NeuralInferenceScheduler& ownerIn)
    : juce::Thread("MatildaNeuralInference"), owner(ownerIn)
{
}

void NeuralInferenceScheduler::WorkerThread::run()
{
    owner.runWorkerLoop();
}

NeuralInferenceScheduler::NeuralInferenceScheduler()
    : worker(*this)
{
}

NeuralInferenceScheduler::~NeuralInferenceScheduler()
{
    stop();
}

void NeuralInferenceScheduler::start()
{
    shouldStop.store(false, std::memory_order_release);
    if (!worker.isThreadRunning())
        worker.startThread(juce::Thread::Priority::normal);
}

void NeuralInferenceScheduler::stop()
{
    shouldStop.store(true, std::memory_order_release);
    queueCondition.notify_all();
    worker.stopThread(5000);

    const std::lock_guard<std::mutex> lock(queueMutex);
    jobQueue.clear();
    jobsEnqueuedThisBlock = 0;
}

bool NeuralInferenceScheduler::isRunning() const
{
    return worker.isThreadRunning();
}

void NeuralInferenceScheduler::beginAudioBlock(int incomingNoteOnEvents)
{
    juce::ignoreUnused(incomingNoteOnEvents);
    jobsEnqueuedThisBlock = 0;
    maxJobsThisBlock = 32;
}

bool NeuralInferenceScheduler::tryEnqueue(Job job)
{
    if (job.model == nullptr || job.voice == nullptr)
        return false;

    {
        const std::lock_guard<std::mutex> lock(queueMutex);
        if (jobsEnqueuedThisBlock >= maxJobsThisBlock || jobQueue.size() >= maxQueueSize)
            return false;

        ++jobsEnqueuedThisBlock;
        jobQueue.push_back(std::move(job));
    }

    queueCondition.notify_one();
    return true;
}

void NeuralInferenceScheduler::runWorkerLoop()
{
    while (!worker.threadShouldExit())
    {
        Job job;
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            queueCondition.wait(lock, [this]()
            {
                return shouldStop.load(std::memory_order_acquire) || !jobQueue.empty();
            });

            if (shouldStop.load(std::memory_order_acquire) && jobQueue.empty())
                break;

            if (jobQueue.empty())
                continue;

            job = std::move(jobQueue.front());
            jobQueue.pop_front();
        }

        if (shouldStop.load(std::memory_order_acquire))
            break;

        try
        {
            std::vector<float> output(static_cast<size_t>(job.model->outputShape[0]));
            job.model->eval(job.input, output);

            if (!shouldStop.load(std::memory_order_acquire) && job.voice != nullptr)
                job.voice->acceptInferenceResult(std::move(output), job.generation);
        }
        catch (const std::exception& e)
        {
            juce::Logger::writeToLog("Neural inference failed: " + juce::String(e.what()));
            if (job.voice != nullptr)
                job.voice->acceptInferenceResult({}, job.generation);
        }
        catch (...)
        {
            if (job.voice != nullptr)
                job.voice->acceptInferenceResult({}, job.generation);
        }
    }
}

NeuralModel::NeuralModel(const char* modelResourceName)
    : env(ORT_LOGGING_LEVEL_WARNING, "MatildaPiano")
{
    sessionOptions.SetIntraOpNumThreads(1);
    sessionOptions.SetInterOpNumThreads(1);

    int dataSizeInBytes;
    const void* data = BinaryData::getNamedResource(modelResourceName, dataSizeInBytes);
    
    if (!data)
    {
        juce::Logger::writeToLog("ERROR: Could not load neural model: " + juce::String(modelResourceName));
        throw std::runtime_error("Neural model not found");
    }
    
    session = Ort::Session(env, data, dataSizeInBytes, sessionOptions);
    
    inputShape = session.GetInputTypeInfo(0).GetTensorTypeAndShapeInfo().GetShape();
    outputShape = session.GetOutputTypeInfo(0).GetTensorTypeAndShapeInfo().GetShape();
    
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
    const std::lock_guard<std::mutex> lock(inferenceMutex);

    Ort::MemoryInfo memoryInfo = Ort::MemoryInfo::CreateCpu(
        OrtAllocatorType::OrtDeviceAllocator, 
        OrtMemType::OrtMemTypeDefault
    );
    
    auto inputTensor = Ort::Value::CreateTensor<float>(
        memoryInfo, input.data(), input.size(), inputShape.data(), inputShape.size());
    
    auto outputTensor = Ort::Value::CreateTensor<float>(
        memoryInfo, output.data(), output.size(), outputShape.data(), outputShape.size());
    
    std::vector<const char*> inputNames{inputName.data()};
    std::vector<const char*> outputNames{outputName.data()};
    
    session.Run(
        Ort::RunOptions{nullptr},
        inputNames.data(), &inputTensor, 1,
        outputNames.data(), &outputTensor, 1
    );
}

MatildaNeuralVoice::MatildaNeuralVoice(NeuralModel* neuralModel, NeuralInferenceScheduler* inferenceScheduler)
    : model(neuralModel), scheduler(inferenceScheduler)
{
    jassert(model != nullptr);
    
    const size_t outputSize = static_cast<size_t>(model->outputShape[0]);
    targetAmplitudes.assign(outputSize, 0.0f);
    currentAmplitudes.assign(outputSize, 0.0f);
    pendingAmplitudes.assign(outputSize, 0.0f);
    phasesLeft.assign(outputSize, 0.0f);
    phasesRight.assign(outputSize, 0.0f);
    
    const size_t inputSize = static_cast<size_t>(model->inputShape[0]);
    inputVector.assign(inputSize, 0.0f);
    
    adsrParams.attack = 0.01f;
    adsrParams.decay = 0.1f;
    adsrParams.sustain = 0.8f;
    adsrParams.release = 0.5f;
    adsr.setParameters(adsrParams);
}

MatildaNeuralVoice::~MatildaNeuralVoice() = default;

void MatildaNeuralVoice::clampAmplitudes(std::vector<float>& amplitudes)
{
    for (auto& value : amplitudes)
    {
        if (!std::isfinite(value))
            value = 0.0f;
        else
            value = juce::jlimit(-0.4f, 0.4f, value);
    }
}

void MatildaNeuralVoice::normalizeAmplitudeEnergy(std::vector<float>& amplitudes, float maxAbsSum)
{
    float absSum = 0.0f;
    for (const float value : amplitudes)
        absSum += std::abs(value);

    if (absSum > maxAbsSum && absSum > 0.0f)
    {
        const float scale = maxAbsSum / absSum;
        for (float& value : amplitudes)
            value *= scale;
    }
}

void MatildaNeuralVoice::seedDefaultAmplitudes()
{
    for (size_t i = 0; i < targetAmplitudes.size(); ++i)
    {
        const float amplitude = std::exp(-0.02f * static_cast<float>(i)) * 0.12f * currentVelocity;
        targetAmplitudes[i] = amplitude;
        currentAmplitudes[i] = amplitude;
    }
}

void MatildaNeuralVoice::updateInferenceInput(float periodCount)
{
    inputVector[0] = currentPitch;
    inputVector[1] = currentVelocity;
    inputVector[2] = periodCount;
}

void MatildaNeuralVoice::launchInference(uint32_t generation, bool force)
{
    juce::ignoreUnused(force);

    if (scheduler == nullptr || inferenceRunning.load(std::memory_order_acquire))
        return;

    NeuralInferenceScheduler::Job job;
    job.model = model;
    job.voice = this;
    job.input = inputVector;
    job.generation = generation;

    if (!scheduler->tryEnqueue(std::move(job)))
        return;

    inferenceRunning.store(true, std::memory_order_release);
}

void MatildaNeuralVoice::acceptInferenceResult(std::vector<float>&& output, uint32_t generation)
{
    if (!output.empty())
    {
        clampAmplitudes(output);
        normalizeAmplitudeEnergy(output, 1.2f);
    }

    const std::lock_guard<std::mutex> lock(pendingMutex);
    if (generation == inferenceGeneration.load(std::memory_order_acquire) && !output.empty())
    {
        pendingAmplitudes = std::move(output);
        pendingGeneration = generation;
        pendingReady = true;
    }

    inferenceRunning.store(false, std::memory_order_release);
}

void MatildaNeuralVoice::harvestInferenceResults()
{
    std::lock_guard<std::mutex> lock(pendingMutex);
    if (!pendingReady || pendingGeneration != inferenceGeneration.load(std::memory_order_acquire))
        return;

    normalizeAmplitudeEnergy(pendingAmplitudes, 1.2f);

    for (size_t i = 0; i < targetAmplitudes.size(); ++i)
    {
        targetAmplitudes[i] = currentAmplitudes[i]
            + (pendingAmplitudes[i] - currentAmplitudes[i]) * 0.3f;
    }

    pendingReady = false;
}

void MatildaNeuralVoice::setSampleRate(double sr)
{
    sampleRate = juce::jmax(1.0f, static_cast<float>(sr));
    adsr.setSampleRate(sr);
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
    currentVelocity = juce::jmax(0.05f, juce::jlimit(0.0f, 1.0f, velocity));
    
    currentPitch = juce::jlimit(0.0f, 1.0f, (currentMidiNote - 21.0f) / 87.0f);
    
    sampleCounter = 0;
    tailOff =  1.0f;
    tailOffRatio = DEFAULT_TAIL_OFF_RATIO;
    currentDecay = 1.0f;
    
    const float frequency = juce::jmax(20.0f,
        440.0f * std::pow(2.0f, (currentMidiNote - 69.0f) / 12.0f));
    period = juce::jmax(1.0f, sampleRate / frequency);
    deltaStep = juce::MathConstants<float>::twoPi * frequency / sampleRate;
    
    for (size_t i = 0; i < phasesLeft.size(); ++i)
    {
        phasesLeft[i] = phaseDistribution(randomGenerator);
        phasesRight[i] = phaseDistribution(randomGenerator);
    }

    {
        const std::lock_guard<std::mutex> lock(pendingMutex);
        pendingReady = false;
    }

    inferenceRunning.store(false, std::memory_order_release);
    seedDefaultAmplitudes();

    updateInferenceInput(0.0f);
    inferenceGeneration.fetch_add(1, std::memory_order_acq_rel);
    samplesUntilNextInference = INFERENCE_RELAUNCH_SAMPLES;
    launchInference(inferenceGeneration.load(std::memory_order_acquire), true);

    adsr.noteOn();
}

void MatildaNeuralVoice::stopNote(float, bool allowTailOff)
{
    keyIsDown = false;
    inferenceGeneration.fetch_add(1, std::memory_order_acq_rel);

    {
        const std::lock_guard<std::mutex> lock(pendingMutex);
        pendingReady = false;
    }
    
    if (allowTailOff)
        adsr.noteOff();
    else
    {
        tailOff = 0.0f;
        adsr.reset();
        clearCurrentNote();
        isSounding = false;
    }
}

void MatildaNeuralVoice::pitchWheelMoved(int) {}
void MatildaNeuralVoice::controllerMoved(int, int) {}

void MatildaNeuralVoice::renderNextBlock(juce::AudioBuffer<float>& outputBuffer, 
                                         int startSample, int numSamples)
{
    if (!isSounding)
        return;

    harvestInferenceResults();

    if (keyIsDown)
    {
        samplesUntilNextInference -= numSamples;
        if (samplesUntilNextInference <= 0)
        {
            const float currentPeriod = static_cast<float>(sampleCounter) / period;
            const float periodCount = std::tanh(currentPeriod / MAX_PERIOD_COUNT);
            updateInferenceInput(periodCount);
            launchInference(inferenceGeneration.load(std::memory_order_acquire), false);
            samplesUntilNextInference = INFERENCE_RELAUNCH_SAMPLES;
        }
    }
    
    const int numChannels = outputBuffer.getNumChannels();
    
    while (--numSamples >= 0)
    {
        computeNextSample();
        
        const float adsrLevel = adsr.getNextSample();
        const float finalLevel = adsrLevel * tailOff;
        
        if (numChannels >= 2)
        {
            outputBuffer.addSample(0, startSample, outputSample[0] * finalLevel);
            outputBuffer.addSample(1, startSample, outputSample[1] * finalLevel);
        }
        else if (numChannels == 1)
        {
            const float mono = (outputSample[0] + outputSample[1]) * 0.5f;
            outputBuffer.addSample(0, startSample, mono * finalLevel);
        }
        
        ++startSample;
        
        if (!keyIsDown && !adsr.isActive())
        {
            tailOff *= tailOffRatio;
            
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
    constexpr float smoothingRate = 120.0f;
    const float smoothingFactor = juce::jmin(1.0f, smoothingRate / sampleRate);
    
    for (size_t i = 0; i < currentAmplitudes.size(); ++i)
        currentAmplitudes[i] += (targetAmplitudes[i] - currentAmplitudes[i]) * smoothingFactor;
    
    const float currentPeriod = static_cast<float>(sampleCounter) / period;
    const float attackEnvelope = std::min(6.0f * currentPeriod, 1.0f);
    const float decayPart = -0.003f * currentPeriod;
    currentDecay = 1.0f / (1.0f + decayPart * decayPart);
    
    const float envelope = std::min(attackEnvelope, currentDecay) * currentVelocity;
    
    outputSample[0] = 0.0f;
    outputSample[1] = 0.0f;
    
    const float step = static_cast<float>(sampleCounter) * deltaStep;
    
    for (size_t i = 0; i < currentAmplitudes.size(); ++i)
    {
        const float partialStep = static_cast<float>(i + 1) * step;
        outputSample[0] += currentAmplitudes[i] * std::sin(phasesLeft[i] + partialStep);
        outputSample[1] += currentAmplitudes[i] * std::sin(phasesRight[i] + partialStep);
    }
    
    outputSample[0] *= envelope * 0.5f;
    outputSample[1] *= envelope * 0.5f;

    outputSample[0] = std::tanh(outputSample[0]);
    outputSample[1] = std::tanh(outputSample[1]);

    if (!std::isfinite(outputSample[0])) outputSample[0] = 0.0f;
    if (!std::isfinite(outputSample[1])) outputSample[1] = 0.0f;
    
    ++sampleCounter;
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
