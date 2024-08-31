#include <string>
#include <mutex>


// Objects
static const uint16_t OBJECT1              = 1;
static const uint16_t OBJECT2              = 2;
static const uint16_t OBJECT3              = 3;

// Property values
static const uint16_t ENABLED              = 14;
static const uint16_t AMPLITUDE            = 170;
static const uint16_t FREQUENCY            = 255;
static const uint16_t GLITCH_CHANCE        = 300;

// Other constants
static const uint16_t COMMAND_READ         = 1;
static const uint16_t COMMAND_WRITE        = 2;
static const uint16_t CONTROL_CHANNEL_PORT = 4000;
static const uint16_t READ_MESSAGE_LENGTH  = 6;
static const uint16_t WRITE_MESSAGE_LENGTH = 8;


class ControlChannelIF {
    public:   // Virtual destructor
        virtual ~ControlChannelIF() {}
    public:   // API
        virtual void sendReadCommand(uint16_t object, uint16_t property) = 0;
        virtual void sendWriteCommand(uint16_t object, uint16_t property, uint16_t value) = 0;
};

