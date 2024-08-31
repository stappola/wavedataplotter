#include <thread>
#include <atomic>
#include <vector>
#include <memory>


// Forward declaration
class SocketDataProviderIF;

class SocketReaderThread {
    public:  // Constructors
        SocketReaderThread(uint16_t port, SocketDataProviderIF& consumer);
        ~SocketReaderThread();
    public:  // API
        void startThread();
        void stopThread();
    private:  // Helpers
        void runThread();
        void stop();
        void join();
        void readDataFromSocket();
    private:  // Data
        std::atomic_bool mRunning;
        std::thread mSocketReaderThread;
        int mClientSocket{0};
        uint16_t mTargetPort;
        SocketDataProviderIF& mDataConsumerIF;
};
