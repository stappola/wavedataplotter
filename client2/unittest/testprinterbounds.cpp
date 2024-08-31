#include "socketdataprinter.h"
#include "controlchannelmock.h"
#include "fakesocketreader.h"

#include <gtest/gtest.h>
#include <fstream>
#include <iostream>

static const char* DEFAULT_OUTPUT_FILE_NAME = "output.txt";
using namespace ::testing;

/********************************************************************
 * Common test fixture for setup, teardown & helper operations.
 ********************************************************************/
class Client2Fixture : public testing::Test {
    public:
        Client2Fixture() {
            mControlChannelMock = std::make_unique<testing::StrictMock<MockControlChannel>>();
        }
        virtual ~Client2Fixture() {}
    public:
        bool verifyOutputExists() {
            try {
                std::ifstream infile(DEFAULT_OUTPUT_FILE_NAME, std::ios::in | std::ios::ate);
                return !infile.fail();
            }
            catch(const std::exception& e) {
                std::cerr << "Exception when reading input file" << std::endl;
            }
            return false;
        }
    protected:
        void expectGlitchChanceUpdate() {
            EXPECT_CALL(*mControlChannelMock, sendWriteCommand(1, 300, 0)).Times(1);
            EXPECT_CALL(*mControlChannelMock, sendWriteCommand(2, 300, 0)).Times(1);
        }
    protected:
        std::unique_ptr<testing::StrictMock<MockControlChannel>> mControlChannelMock;
};

TEST_F(Client2Fixture, BasicTest) {
    expectGlitchChanceUpdate();
    std::vector<uint16_t> portVector{ 1, 2, 4003, 4, 5 };
    std::unique_ptr<SocketDataPrinter> systemUnderTest = std::make_unique<SocketDataPrinter>(portVector, 100, *mControlChannelMock);
    std::unique_ptr<FakeSocketReader> socketReaderMock(std::make_unique<FakeSocketReader>(*systemUnderTest));

    // Verify control channel stays silent
    EXPECT_CALL(*mControlChannelMock, sendWriteCommand(_, _, _)).Times(0);
    socketReaderMock->inputFromSocket(4.5, 5);
}


