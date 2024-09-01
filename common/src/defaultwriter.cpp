#include "defaultwriter.h"

#include <iostream>


DefaultWriter::DefaultWriter() {}

DefaultWriter::~DefaultWriter() {}
    
void DefaultWriter::writeToStream(const std::string& data) {
    std::cout << data;
}

