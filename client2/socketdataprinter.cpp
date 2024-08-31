#include "socketdataprinter.h"
#include "controlchannelif.h"

#include <string>
#include <iostream>
#include <iomanip>
#include <algorithm>


SocketDataPrinter::SocketDataPrinter(const std::vector<uint16_t>& targetPorts,
    int64_t timeWindow, ControlChannelIF& ctrlChannelIF)
    : mTimeWindow(timeWindow)
    , mCtrlChannel(ctrlChannelIF) {
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

void SocketDataPrinter::setCurrentTimeStamp() {
    const auto now = std::chrono::high_resolution_clock::now();
    auto duration = now.time_since_epoch();
    mMillisecondsSinceEpoch = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    std::cout << "Current time in milliseconds is: " << mMillisecondsSinceEpoch << std::endl;
}

uint64_t SocketDataPrinter::getCurrentTimeStamp() const {
    const auto now = std::chrono::high_resolution_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}

void SocketDataPrinter::handlePrintingSchedule() {
    int64_t currentTime(getCurrentTimeStamp());
    if((currentTime - mMillisecondsSinceEpoch) >= mTimeWindow) {
        uint16_t index{0};
        std::cout << "{\"timestamp\": " << currentTime;
        std::lock_guard<std::mutex> guard(mPrinterMutex);
        for(auto& port : mTargetPorts) {
            std::cout << ", \"out" << index + 1 << "\": " << "\"" << port.getStrValue() << "\"";
            port.resetValue();
            index++;
        }
        std::cout << "}" << std::endl;
        mMillisecondsSinceEpoch = currentTime;
    }
}

void SocketDataPrinter::startPrinterThread() {
    mRunning.store(true);
    mTimestampThread = std::thread(&SocketDataPrinter::runThread, this);
}

void SocketDataPrinter::stopPrinterThread() {
    std::cout << "SocketDataPrinter::stopThread()" << std::endl;
    mRunning.store(false);
}

void SocketDataPrinter::runThread() {
    while(mRunning.load()) {
        handlePrintingSchedule();
    }
}

// NOTE: The implementation assumes the port numbers are not
// significant. Therefore the frequency is changed on output
// one event when the port number is not 4001.
void SocketDataPrinter::checkLimits(const ValueMap& value) {
    /*
    * When the value on the output 3 of the server becomes greater than or equal to 3.0:
    *    - Set the frequency of server output 1 to 1Hz.
    *    - Set the amplitude of server output 1 to 8000.
    */
    if(value.getFltValue() >= TRIGGER_BOUND && mCurrentControlValue < TRIGGER_BOUND) {
        std::cout << "SocketDataPrinter::checkLimits(), value is "
            << value.getStrValue() << ", current control value is "
            << mCurrentControlValue << std::endl;
        mCtrlChannel.sendWriteCommand(OBJECT1, FREQUENCY, 1000);
        mCtrlChannel.sendWriteCommand(OBJECT1, AMPLITUDE, 8000);
    /*
    * When the value on the output 3 becomes lower than 3.0:
    *    - Set the frequency of server output 1 to 2Hz.
    *    - Set the amplitude of server output 1 to 4000.
    */
    } else if(mCurrentControlValue >= TRIGGER_BOUND && value.getFltValue() < TRIGGER_BOUND) {
        std::cout << "SocketDataPrinter::checkLimits(), value is "
            << value.getStrValue() << ", current control value is "
            << mCurrentControlValue << std::endl;
        mCtrlChannel.sendWriteCommand(OBJECT1, FREQUENCY, 2000);
        mCtrlChannel.sendWriteCommand(OBJECT1, AMPLITUDE, 4000);
    } else {
        std::cout << "SocketDataPrinter::checkLimits(), value is "
            << value.getStrValue() << ", current control value is "
            << mCurrentControlValue << std::endl;
    }
    mCurrentControlValue = value.getFltValue();
}

void SocketDataPrinter::receiveData(float data, uint16_t port) {
    std::cout << "SocketDataPrinter::receiveData() - Data: " << data << ", Port: " << port << std::endl;
    std::lock_guard<std::mutex> guard(mPrinterMutex);
    auto valueItem = std::find_if(mTargetPorts.begin(), mTargetPorts.end(), [&port](const ValueMap& entry) {
        return port == entry.getPort();
    });

    if(valueItem != mTargetPorts.end()) {
        valueItem->assignValue(data);
        if(valueItem->getPort() == 4003 && valueItem->isAvailable()) {
            checkLimits(*valueItem);
        }
    }
}