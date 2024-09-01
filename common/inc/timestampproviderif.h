#include <cstdint>


class TimestampProviderIF {
    public:   // Virtual destructor
        virtual ~TimestampProviderIF() {}
    public:   // API
        virtual uint64_t getCurrentTimeStamp() const = 0;
};

