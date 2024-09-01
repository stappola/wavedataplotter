#include "socketreaderthread.h"
#include "socketdataprinter.h"
#include "controlchannel.h"
#include "defaultwriter.h"
#include <iostream>


int main(void) {
    const int64_t timeWindow(20);
    DefaultWriter stdOutWriter;
    std::vector<uint16_t> portVector{4001, 4002, 4003};
    std::unique_ptr<ControlChannel> ctrlChannel = std::make_unique<ControlChannel>();
    SocketDataPrinter dataPrinter(portVector, timeWindow, *ctrlChannel, stdOutWriter);
    SocketReaderThread thread1(portVector.at(0), dataPrinter);
    SocketReaderThread thread2(portVector.at(1), dataPrinter);
    SocketReaderThread thread3(portVector.at(2), dataPrinter);
    dataPrinter.startPrinterThread();
    thread1.startThread();
    thread2.startThread();
    thread3.startThread();
    std::cin.get();
    dataPrinter.stopPrinterThread();
    thread1.stopThread();
    thread2.stopThread();
    thread3.stopThread();
    return 0;
}