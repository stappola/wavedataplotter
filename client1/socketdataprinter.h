#include "socketdataproviderif.h"
#include "defaulttimestamp.h"

#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <sstream>
#include <iomanip>


// Forward decratations
class OutputWriterIF;

class SocketDataPrinter : public SocketDataProviderIF {
    public:
        SocketDataPrinter(const std::vector<uint16_t>& targetPorts,
            int64_t timeWindow, OutputWriterIF& outputWriter);
        ~SocketDataPrinter();
    public:
        void startPrinterThread();
        void stopPrinterThread();
        void installTimestampService(TimestampProviderIF* timestampService);
    private:  // From SocketDataProviderIF
        virtual void receiveData(float data, uint16_t port) override;
    private:  // Helpers
        void runThread();
        void stop();
        void join();
        void handlePrintingSchedule();
        void setCurrentTimeStamp();
        uint64_t getCurrentTimeStamp() const;
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
                std::string getValue() {
                    return mValue == NOT_AVAILABLE ? "--" : formatValue();
                }
                uint16_t getPort() const { return mTargetPort; }
            private:
                std::string formatValue() {
                    mFormatStream.clear();
                    mFormatStream.str(std::string());
                    mFormatStream << std::fixed << std::setprecision(1) << mValue;
                    return mFormatStream.str();
                }
            private:  // Data
                float mValue;
                uint16_t mTargetPort;
                std::stringstream mFormatStream;
        };
    private:  // Data
        std::atomic_bool mRunning;
        std::mutex mPrinterMutex;
        std::vector<ValueMap> mTargetPorts;
        std::thread mTimestampThread;
        int64_t mTimeWindow{0};
        int64_t mMillisecondsSinceEpoch;
        OutputWriterIF& mOutputWriter;
        std::stringstream mOutputStream;
        DefaultTimestamp mDefaultTimestamp;
        TimestampProviderIF* mTimestampService{nullptr};
};