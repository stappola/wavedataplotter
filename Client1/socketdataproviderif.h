#include <cstdint>


class SocketDataProviderIF {
    public:
        virtual void receiveData(float data, uint16_t port) = 0;
};