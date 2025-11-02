#include <iostream>
#include "src/core/DataStruct/DataStruct.h"

int main() {
    std::cout << "Size of TemperatureSensor: " << sizeof(TemperatureSensor) << " bytes" << std::endl;
    std::cout << "Size of SmartDiskScore: " << sizeof(SmartDiskScore) << " bytes" << std::endl;
    std::cout << "Size of SharedMemoryBlock: " << sizeof(SharedMemoryBlock) << " bytes" << std::endl;
    return 0;
}