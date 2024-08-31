#include "fakesocketreader.h"
#include "socketdataproviderif.h"


FakeSocketReader::FakeSocketReader(SocketDataProviderIF& dataConsumerIF)
    : mDataConsumer(dataConsumerIF) {

}

FakeSocketReader::~FakeSocketReader() {

}

void FakeSocketReader::inputFromSocket(float data, uint16_t port) {
    mDataConsumer.receiveData(data, port);
}

