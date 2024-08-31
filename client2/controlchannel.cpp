#include "controlchannel.h"

#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <string>

namespace {
    // https://stackoverflow.com/questions/29242/off-the-shelf-c-hex-dump-code
    void hexdump(void* ptr, int buflen) {
        unsigned char* buf = (unsigned char*)ptr;
        int i, j;
        for(i = 0;i < buflen; i += 16) {
            printf("%06x: ", i);
            for(j=0; j<16; j++) {
                if (i+j < buflen) {
                    printf("%02x ", buf[i+j]);
                } else {
                    printf("   ");
                }
            }
            printf(" ");
            for(j = 0; j < 16; j++) {
                if (i + j < buflen) {
                    printf("%c", isprint(buf[i+j]) ? buf[i + j] : '.');
                }
            }
            printf("\n");
        }
    }
}

ControlChannel::ControlChannel() {
}

ControlChannel::~ControlChannel() {
}

void ControlChannel::encodeUint16(uint16_t twoBytes, uint8_t* data) {
    uint8_t* encodePtr = data;
    *encodePtr = (uint8_t)((twoBytes & 0xFF00) >> 8); encodePtr++;
    *encodePtr = (uint8_t)(twoBytes & 0x00FF); encodePtr; 
}

void ControlChannel::sendReadCommand(uint16_t object, uint16_t property) {
    uint8_t sendBuf[READ_MESSAGE_LENGTH] = {0};
    const std::string hostname{"127.0.0.1"};
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    sockaddr_in destination;
    destination.sin_family = AF_INET;
    destination.sin_port = htons(CONTROL_CHANNEL_PORT);
    destination.sin_addr.s_addr = inet_addr(hostname.c_str());

    encodeUint16(COMMAND_READ, &sendBuf[0]);
    encodeUint16(object, &sendBuf[2]);
    encodeUint16(property, &sendBuf[4]);

    hexdump(sendBuf, sizeof(sendBuf));
    int bytes = ::sendto(sock, sendBuf, sizeof(sendBuf), 0, reinterpret_cast<sockaddr*>(&destination), sizeof(destination));
    std::cout << bytes << " bytes sent" << std::endl;
    close(sock);
}

void ControlChannel::sendWriteCommand(uint16_t object, uint16_t property, uint16_t value) {
    uint8_t sendBuf[WRITE_MESSAGE_LENGTH] = {0};
    const std::string hostname{"127.0.0.1"};
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    sockaddr_in destination;
    destination.sin_family = AF_INET;
    destination.sin_port = htons(CONTROL_CHANNEL_PORT);
    destination.sin_addr.s_addr = inet_addr(hostname.c_str());

    encodeUint16(COMMAND_WRITE, &sendBuf[0]);
    encodeUint16(object, &sendBuf[2]);
    encodeUint16(property, &sendBuf[4]);
    encodeUint16(value, &sendBuf[6]);

    hexdump(sendBuf, sizeof(sendBuf));
    int bytes = ::sendto(sock, sendBuf, sizeof(sendBuf), 0, reinterpret_cast<sockaddr*>(&destination), sizeof(destination));
    close(sock);
}

