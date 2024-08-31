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

TEST_F(Client2Fixture, BasicTests) {
    expectGlitchChanceUpdate();
    std::vector<uint16_t> portVector{ 1, 2, 4003, 4, 5 };
    std::unique_ptr<SocketDataPrinter> systemUnderTest = std::make_unique<SocketDataPrinter>(portVector, 100, *mControlChannelMock);
    std::unique_ptr<FakeSocketReader> socketReaderMock(std::make_unique<FakeSocketReader>(*systemUnderTest));

    // Verify control channel stays silent
    EXPECT_CALL(*mControlChannelMock, sendWriteCommand(_, _, _)).Times(0);
    socketReaderMock->inputFromSocket(4.5, 5);

    // Verify control channel stays silent
    EXPECT_CALL(*mControlChannelMock, sendWriteCommand(_, _, _)).Times(0);
    socketReaderMock->inputFromSocket(2.5, 4003);

    // Verify frequency changes to 1Hz & amplitude to 8000
    EXPECT_CALL(*mControlChannelMock, sendWriteCommand(1, 255, 1000)).Times(1);
    EXPECT_CALL(*mControlChannelMock, sendWriteCommand(1, 170, 8000)).Times(1);
    socketReaderMock->inputFromSocket(3.5, 4003);

    // Verify control channel stays silent because value is already over 3.0
    EXPECT_CALL(*mControlChannelMock, sendWriteCommand(_, _, _)).Times(0);
    EXPECT_CALL(*mControlChannelMock, sendWriteCommand(_, _, _)).Times(0);
    socketReaderMock->inputFromSocket(4.6, 4003);

    // Verify frequency changes to 2Hz & amplitude to 4000
    EXPECT_CALL(*mControlChannelMock, sendWriteCommand(1, 255, 2000)).Times(1);
    EXPECT_CALL(*mControlChannelMock, sendWriteCommand(1, 170, 4000)).Times(1);
    socketReaderMock->inputFromSocket(1.5, 4003);

    // Verify control channel stays silent because value is already below 3.0
    EXPECT_CALL(*mControlChannelMock, sendWriteCommand(_, _, _)).Times(0);
    EXPECT_CALL(*mControlChannelMock, sendWriteCommand(_, _, _)).Times(0);
    socketReaderMock->inputFromSocket(2.3, 4003);
}


