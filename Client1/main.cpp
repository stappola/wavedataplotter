#include "socketreaderthread.h"
#include "socketdataprinter.h"

#include <iostream>


int main(void) {
    const int64_t timeWindow(100);

    // Create objects
    std::vector<uint16_t> portVector{4001, 4002, 4003};
    SocketDataPrinter dataPrinter(portVector, timeWindow);
    SocketReaderThread thread1(portVector.at(0), dataPrinter);
    SocketReaderThread thread2(portVector.at(1), dataPrinter);
    SocketReaderThread thread3(portVector.at(2), dataPrinter);

    // Start threads
    dataPrinter.startPrinterThread();
    thread1.startThread();
    thread2.startThread();
    thread3.startThread();

    // Wait for user input
    std::cin.get();

    // Close down & cleanup
    dataPrinter.stopPrinterThread();
    thread1.stopThread();
    thread2.stopThread();
    thread3.stopThread();
    return 0;
}