#include "socketdataprinter.h"
#include "dataoutputif.h"

#include <algorithm>


SocketDataPrinter::SocketDataPrinter(const std::vector<uint16_t>& targetPorts,
    int64_t timeWindow, OutputWriterIF& outputWriter)
        : mTimeWindow(timeWindow)
        , mOutputWriter(outputWriter) {
    for(const auto& port : targetPorts) {
        mTargetPorts.push_back(SocketDataPrinter::ValueMap(port));
    }
}

void SocketDataPrinter::installTimestampService(TimestampProviderIF* timestampService) {
    mTimestampService = timestampService == nullptr ? &mDefaultTimestamp : timestampService;
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
            mOutputStream << ", \"out" << index + 1 << "\": " << "\"" << port.getValue() << "\"";
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

void SocketDataPrinter::receiveData(float data, uint16_t port) {
    std::lock_guard<std::mutex> guard(mPrinterMutex);
    auto valueItem = std::find_if(mTargetPorts.begin(), mTargetPorts.end(), [&port](const ValueMap& entry) {
        return port == entry.getPort();
    });

    if(valueItem != mTargetPorts.end()) {
        valueItem->assignValue(data);
    }
}
