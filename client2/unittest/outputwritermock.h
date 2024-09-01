#include "dataoutputif.h"
#include <gmock/gmock.h>


class MockOutputWriter : public OutputWriterIF {
    public:
        MOCK_METHOD(void, writeToStream, (const std::string& data));
};


