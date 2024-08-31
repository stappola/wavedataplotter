#include "socketdataproviderif.h"

#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <sstream>
#include <iomanip>


class SocketDataPrinter : public SocketDataProviderIF {
    public:
        SocketDataPrinter(const std::vector<uint16_t>& targetPorts, int64_t timeWindow);
        ~SocketDataPrinter();
    public:
        void startPrinterThread();
        void stopPrinterThread();
    private:  // From SocketDataProviderIF
        virtual void receiveData(float data, uint16_t port) override;
    private:  // Helpers
        void runThread();
        void stop();
        void join();
        void setCurrentTimeStamp();
        uint64_t getCurrentTimeStamp() const;
        void handlePrintingSchedule();
    private:
        static constexpr float NOT_AVAILABLE = 3.402823E+38;
        class ValueMap {
            public:
                ValueMap(uint16_t port)
                    : mValue(NOT_AVAILABLE)
                    , mTargetPort(port) {}
            public:
                void resetValue() { mValue = NOT_AVAILABLE; }
                void assignValue(float value) { mValue = value; }
                std::string getValue() const {
                    return mValue == NOT_AVAILABLE ? "--" : formatValue();
                }
                uint16_t getPort() const { return mTargetPort; }
            private:
                std::string formatValue() const {
                    std::stringstream stream;
                    stream << std::fixed << std::setprecision(1) << mValue;
                    return stream.str();
                }
            private:  // Data
                float mValue;
                uint16_t mTargetPort;
        };
    private:  // Data
        std::atomic_bool mRunning;
        std::mutex mPrinterMutex;
        std::vector<ValueMap> mTargetPorts;
        std::thread mTimestampThread;
        int64_t mTimeWindow{0};
        int64_t mMillisecondsSinceEpoch;
};