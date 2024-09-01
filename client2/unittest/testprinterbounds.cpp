#include "socketdataprinter.h"
#include "controlchannelmock.h"
#include "outputwritermock.h"
#include "fakesocketreader.h"

#include <gtest/gtest.h>
#include <fstream>
#include <iostream>

static const char* DEFAULT_OUTPUT_FILE_NAME = "output.txt";
using namespace ::testing;

class MockTimestampProvider : public TimestampProviderIF {
    public:
        MOCK_METHOD(uint64_t, getCurrentTimeStamp, (), (const));
};

/********************************************************************
 * Common test fixture for setup, teardown & helper operations.
 ********************************************************************/
class Client2Fixture : public testing::Test {
    public:
        Client2Fixture() {
            mOutputWriterMock = std::make_unique<testing::StrictMock<MockOutputWriter>>();
            mControlChannelMock = std::make_unique<testing::StrictMock<MockControlChannel>>();
            mTimestampServiceMock = std::make_unique<testing::StrictMock<MockTimestampProvider>>();
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
        std::unique_ptr<testing::StrictMock<MockOutputWriter>> mOutputWriterMock;
        std::unique_ptr<testing::StrictMock<MockControlChannel>> mControlChannelMock;
        std::unique_ptr<testing::StrictMock<MockTimestampProvider>> mTimestampServiceMock;
};

TEST_F(Client2Fixture, ControlChannelTests) {
    expectGlitchChanceUpdate();
    std::vector<uint16_t> portVector{ 1, 2, 4003, 4, 5 };
    std::unique_ptr<SocketDataPrinter> systemUnderTest = 
        std::make_unique<SocketDataPrinter>(portVector, 100, *mControlChannelMock, *mOutputWriterMock);
    std::unique_ptr<FakeSocketReader> socketReaderMock(std::make_unique<FakeSocketReader>(*systemUnderTest));

    // Verify control channel stays silent, output 5 is not interesting
    EXPECT_CALL(*mControlChannelMock, sendWriteCommand(_, _, _)).Times(0);
    socketReaderMock->inputFromSocket(4.5, portVector.at(4));

    // Verify control channel sets the initial values
    EXPECT_CALL(*mControlChannelMock, sendWriteCommand(1, 255, 2000)).Times(1);
    EXPECT_CALL(*mControlChannelMock, sendWriteCommand(1, 170, 4000)).Times(1);
    socketReaderMock->inputFromSocket(2.5, portVector.at(2));

    // Verify frequency changes to 1Hz & amplitude to 8000
    EXPECT_CALL(*mControlChannelMock, sendWriteCommand(1, 255, 1000)).Times(1);
    EXPECT_CALL(*mControlChannelMock, sendWriteCommand(1, 170, 8000)).Times(1);
    socketReaderMock->inputFromSocket(3.0, portVector.at(2));

    // Verify control channel stays silent because value is already over 3.0
    EXPECT_CALL(*mControlChannelMock, sendWriteCommand(_, _, _)).Times(0);
    EXPECT_CALL(*mControlChannelMock, sendWriteCommand(_, _, _)).Times(0);
    socketReaderMock->inputFromSocket(4.6, portVector.at(2));

    // Verify frequency changes to 2Hz & amplitude to 4000
    EXPECT_CALL(*mControlChannelMock, sendWriteCommand(1, 255, 2000)).Times(1);
    EXPECT_CALL(*mControlChannelMock, sendWriteCommand(1, 170, 4000)).Times(1);
    socketReaderMock->inputFromSocket(1.5, portVector.at(2));

    // Verify control channel stays silent because value is already below 3.0
    EXPECT_CALL(*mControlChannelMock, sendWriteCommand(_, _, _)).Times(0);
    EXPECT_CALL(*mControlChannelMock, sendWriteCommand(_, _, _)).Times(0);
    socketReaderMock->inputFromSocket(2.3, portVector.at(2));
}

TEST_F(Client2Fixture, PrintingTests) {
    expectGlitchChanceUpdate();
    std::vector<uint16_t> portVector{ 1, 2, 4003, 4, 5 };
    std::unique_ptr<SocketDataPrinter> systemUnderTest = 
        std::make_unique<SocketDataPrinter>(portVector, 20, *mControlChannelMock, *mOutputWriterMock);
    std::unique_ptr<FakeSocketReader> socketReaderMock(std::make_unique<FakeSocketReader>(*systemUnderTest));

    EXPECT_CALL(*mTimestampServiceMock, getCurrentTimeStamp()).Times(1).WillOnce(Return(20));
    systemUnderTest->installTimestampService(mTimestampServiceMock.get());

    // Time window is set to 20, no writing should occur yet
    EXPECT_CALL(*mOutputWriterMock, writeToStream(_)).Times(0);
    EXPECT_CALL(*mTimestampServiceMock, getCurrentTimeStamp()).Times(1).WillOnce(Return(30));
    systemUnderTest->handlePrintingSchedule();

    EXPECT_CALL(*mOutputWriterMock, writeToStream(_)).Times(1);
    EXPECT_CALL(*mTimestampServiceMock, getCurrentTimeStamp()).Times(1).WillOnce(Return(42));
    systemUnderTest->handlePrintingSchedule();

    
}


