#include <cstdint>

// Forward declaration
class SocketDataProviderIF;

class FakeSocketReader {
    public:  // Constructors
        FakeSocketReader(SocketDataProviderIF& consumer);
        ~FakeSocketReader();
    public:  // API
        void inputFromSocket(float data, uint16_t port);
    private:  // Data
        SocketDataProviderIF& mDataConsumer;
};
