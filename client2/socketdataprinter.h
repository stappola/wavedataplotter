#include "socketdataproviderif.h"
#include "defaulttimestamp.h"

#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <sstream>
#include <iomanip>

static const float TRIGGER_BOUND = 3.0;

// Forward decratations
class OutputWriterIF;
class ControlChannelIF;

class SocketDataPrinter : public SocketDataProviderIF {
    public:
        SocketDataPrinter(const std::vector<uint16_t>& targetPorts,
            int64_t timeWindow, ControlChannelIF& ctrlChannelIF, OutputWriterIF& outputWriter);
        ~SocketDataPrinter();
    public:
        void startPrinterThread();
        void stopPrinterThread();
        void installTimestampService(TimestampProviderIF* timestampService);
        // A small concession to make unittesting easier; should be private, really.
        void handlePrintingSchedule();
    private:  // From SocketDataProviderIF
        virtual void receiveData(float data, uint16_t port) override;
    private:  // Helpers
        void runThread();
        void stop();
        void join();
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
                float getFltValue() const { return mValue; }
                std::string getStrValue() {
                    return mValue == NOT_AVAILABLE ? "--" : formatValue();
                }
                uint16_t getPort() const { return mTargetPort; }
                bool isAvailable() const { return mValue != NOT_AVAILABLE; }
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
        void checkLimits(ValueMap& value);
        void setInitialProperties(ValueMap& value);
    private:  // Data
        std::atomic_bool mRunning;
        std::mutex mPrinterMutex;
        std::vector<ValueMap> mTargetPorts;
        std::thread mTimestampThread;
        int64_t mTimeWindow{0};
        int64_t mMillisecondsSinceEpoch;
        float mCurrentControlValue{NOT_AVAILABLE};
        ControlChannelIF& mCtrlChannel;
        OutputWriterIF& mOutputWriter;
        std::stringstream mOutputStream;
        DefaultTimestamp mDefaultTimestamp;
        TimestampProviderIF* mTimestampService{nullptr};
};