#include "socketdataprinter.h"
#include "controlchannelif.h"
#include "dataoutputif.h"

// #include <string>
#include <algorithm>


SocketDataPrinter::SocketDataPrinter(const std::vector<uint16_t>& targetPorts,
    int64_t timeWindow, ControlChannelIF& ctrlChannelIF, OutputWriterIF& outputWriter)
        : mTimeWindow(timeWindow)
        , mCtrlChannel(ctrlChannelIF)
        , mOutputWriter(outputWriter) {
    for(const auto& port : targetPorts) {
        mTargetPorts.push_back(SocketDataPrinter::ValueMap(port));
    }

    // Not entirely sure what "glitch chance" means, but I'm assuming the value ought to be low.
    mCtrlChannel.sendWriteCommand(OBJECT1, GLITCH_CHANCE, 0);
    mCtrlChannel.sendWriteCommand(OBJECT2, GLITCH_CHANCE, 0);
    setCurrentTimeStamp();
}

SocketDataPrinter::~SocketDataPrinter() {
    stop();
    join();
}

void SocketDataPrinter::stop() {
    mRunning.store(false);
}

void SocketDataPrinter::join() {
    if(mTimestampThread.joinable()) {
        mTimestampThread.join();
    }
}

void SocketDataPrinter::installTimestampService(TimestampProviderIF* timestampService) {
    mTimestampService = timestampService == nullptr ? &mDefaultTimestamp : timestampService;
    setCurrentTimeStamp();
}

void SocketDataPrinter::setCurrentTimeStamp() {
    mMillisecondsSinceEpoch = mTimestampService == nullptr ?
        mDefaultTimestamp.getCurrentTimeStamp() : mTimestampService->getCurrentTimeStamp();
}

uint64_t SocketDataPrinter::getCurrentTimeStamp() const {
    return mTimestampService == nullptr ?
        mDefaultTimestamp.getCurrentTimeStamp() : mTimestampService->getCurrentTimeStamp();
}

void SocketDataPrinter::handlePrintingSchedule() {
    int64_t currentTime(getCurrentTimeStamp());
    if((currentTime - mMillisecondsSinceEpoch) >= mTimeWindow) {
        uint16_t index{0};
        mOutputStream.clear();
        mOutputStream.str(std::string());
        mOutputStream << "{\"timestamp\": " << currentTime;
        std::lock_guard<std::mutex> guard(mPrinterMutex);
        for(auto& port : mTargetPorts) {
            mOutputStream << ", \"out" << index + 1 << "\": " << "\"" << port.getStrValue() << "\"";
            port.resetValue();
            index++;
        }
        mOutputStream << "}" << std::endl;
        mOutputWriter.writeToStream(mOutputStream.str());
        mMillisecondsSinceEpoch = currentTime;
    }
}

void SocketDataPrinter::startPrinterThread() {
    mRunning.store(true);
    mTimestampThread = std::thread(&SocketDataPrinter::runThread, this);
}

void SocketDataPrinter::stopPrinterThread() {
    mRunning.store(false);
}

void SocketDataPrinter::runThread() {
    while(mRunning.load()) {
        handlePrintingSchedule();
    }
}

// Set the server output to known values according to the requirements.
void SocketDataPrinter::setInitialProperties(ValueMap& value) {
    if(value.getFltValue() >= TRIGGER_BOUND) {
        mCtrlChannel.sendWriteCommand(OBJECT1, FREQUENCY, 1000);
        mCtrlChannel.sendWriteCommand(OBJECT1, AMPLITUDE, 8000);
    } else if(value.getFltValue() < TRIGGER_BOUND) {
        mCtrlChannel.sendWriteCommand(OBJECT1, FREQUENCY, 2000);
        mCtrlChannel.sendWriteCommand(OBJECT1, AMPLITUDE, 4000);
    } // no else

    mCurrentControlValue = value.getFltValue();
}

// NOTE: The implementation assumes that the port numbers are not significant.
// Therefore the frequency is changed on output 1 even when the port number is not 4001.
void SocketDataPrinter::checkLimits(ValueMap& value) {
    if(mCurrentControlValue == NOT_AVAILABLE) {
        setInitialProperties(value);
        return;
    }

    /*
    * When the value on the output 3 of the server becomes greater than or equal to 3.0:
    *    - Set the frequency of server output 1 to 1Hz.
    *    - Set the amplitude of server output 1 to 8000.
    */
    if(value.getFltValue() >= TRIGGER_BOUND && mCurrentControlValue < TRIGGER_BOUND) {
        mCtrlChannel.sendWriteCommand(OBJECT1, FREQUENCY, 1000);
        mCtrlChannel.sendWriteCommand(OBJECT1, AMPLITUDE, 8000);
    /*
    * When the value on the output 3 becomes lower than 3.0:
    *    - Set the frequency of server output 1 to 2Hz.
    *    - Set the amplitude of server output 1 to 4000.
    */
    } else if(mCurrentControlValue >= TRIGGER_BOUND && value.getFltValue() < TRIGGER_BOUND) {
        mCtrlChannel.sendWriteCommand(OBJECT1, FREQUENCY, 2000);
        mCtrlChannel.sendWriteCommand(OBJECT1, AMPLITUDE, 4000);
    } // no else

    mCurrentControlValue = value.getFltValue();
}

void SocketDataPrinter::receiveData(float data, uint16_t port) {
    std::lock_guard<std::mutex> guard(mPrinterMutex);
    auto valueItem = std::find_if(mTargetPorts.begin(), mTargetPorts.end(), [&port](const ValueMap& entry) {
        return port == entry.getPort();
    });

    if(valueItem != mTargetPorts.end()) {
        valueItem->assignValue(data);
        // Check the server output number 3, assuming port number itself is not significant
        if(valueItem - mTargetPorts.begin() == 2 && valueItem->isAvailable()) {
            checkLimits(*valueItem);
        }
    }
}