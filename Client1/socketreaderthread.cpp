#include "socketreaderthread.h"
#include "socketdataproviderif.h"

#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <string>


SocketReaderThread::SocketReaderThread(uint16_t port, SocketDataProviderIF& consumer) 
    : mTargetPort(port)
    , mDataConsumerIF(consumer) {
        mRunning.store(false);
}

SocketReaderThread::~SocketReaderThread() {
    stop();
    join();
}

void SocketReaderThread::stop() {
    mRunning.store(false);
}

void SocketReaderThread::join() {
    if(mSocketReaderThread.joinable()) {
        mSocketReaderThread.join();
    }
}

void SocketReaderThread::startThread() {
    int result{0};
    mClientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(mClientSocket) {
        sockaddr_in serverAddress;
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_port = htons(mTargetPort);
        serverAddress.sin_addr.s_addr = INADDR_ANY;

        int flags = fcntl(mClientSocket, F_GETFL, 0);
        flags = flags & (~O_NONBLOCK);
        fcntl(mClientSocket, F_SETFL, flags);
        result = connect(mClientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
    } else {
        // NOTE: I am assumning it is acceptable to notify user of error conditions through stdout
        // although the program should not put out anything else besides the JSON objects.
        std::cerr << "Could not create socket" << std::endl;
    }

    if(result != 0) {
        std::cerr << "Could not start socket reader, error: " << result << std::endl;
        close(mClientSocket);
    } else {
        mRunning.store(true);
        mSocketReaderThread = std::thread(&SocketReaderThread::runThread, this);
    }
}

void SocketReaderThread::stopThread() {
    mRunning.store(false);
}

void SocketReaderThread::runThread() {
    while(!mRunning.load()) {
       std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    while(mRunning.load()) {
        readDataFromSocket();
    }
}

void SocketReaderThread::readDataFromSocket() {
    std::string recvBuf;
    int bytes = read(mClientSocket, &recvBuf[0], recvBuf.capacity());
    if(bytes > 0 && mRunning.load()) {
        try {
            mDataConsumerIF.receiveData(std::stof(recvBuf.data()), mTargetPort);
        } catch(std::exception e) {
            std::cerr << "Error when calling data consumer: " << e.what() << std::endl;
        }
    }
}

