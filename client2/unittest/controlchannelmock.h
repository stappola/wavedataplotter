#include "controlchannelif.h"
#include <gmock/gmock.h>


class MockControlChannel : public ControlChannelIF {
    public:
        MOCK_METHOD(void, sendReadCommand, (uint16_t object, uint16_t property));
        MOCK_METHOD(void, sendWriteCommand, (uint16_t object, uint16_t property, uint16_t value));
};


