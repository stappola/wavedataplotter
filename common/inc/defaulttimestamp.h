#include "timestampproviderif.h"


class DefaultTimestamp : public TimestampProviderIF {
    public:   // Virtual destructor
        DefaultTimestamp();
        ~DefaultTimestamp();
    public:   // API
        virtual uint64_t getCurrentTimeStamp() const override;
};

