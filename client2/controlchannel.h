#include "controlchannelif.h"

#include <string>


class ControlChannel : public ControlChannelIF {
    public:  // Constructor & destructor
        ControlChannel();
        ~ControlChannel();
    public:   // API from ControlChannelIF
        virtual void sendReadCommand(uint16_t object, uint16_t property) override;
        virtual void sendWriteCommand(uint16_t object, uint16_t property, uint16_t value) override;
    private:  // Helpers
        void encodeUint16(uint16_t twoBytes, uint8_t* data);
};

