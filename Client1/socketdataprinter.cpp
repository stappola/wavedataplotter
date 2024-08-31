#include "socketdataprinter.h"

#include <string>
#include <iostream>
#include <iomanip>
#include <algorithm>


SocketDataPrinter::SocketDataPrinter(const std::vector<uint16_t>& targetPorts, int64_t timeWindow)
    : mTimeWindow(timeWindow) {
    for(const auto& port : targetPorts) {
        mTargetPorts.push_back(SocketDataPrinter::ValueMap(port));
    }
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
        for(auto& port : mTargetPorts) {
            std::cout << ", \"out" << index + 1 << "\": " << "\"" << port.getValue() << "\"";
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
    mRunning.store(false);
}

void SocketDataPrinter::runThread() {
    while(mRunning.load()) {
        handlePrintingSchedule();
    }
}

void SocketDataPrinter::receiveData(float data, uint16_t port) {
    std::lock_guard<std::mutex> guard(mPrinterMutex);
    auto valueItem = std::find_if(mTargetPorts.begin(), mTargetPorts.end(), [&port](const ValueMap& entry) {
        return port == entry.getPort();
    });

    if(valueItem != mTargetPorts.end()) {
        valueItem->assignValue(data);
    }
}