#include "Buffer/StaticBuffer.h"
#include "Cache/OpenRelTable.h"
#include "Disk_Class/Disk.h"
#include "FrontendInterface/FrontendInterface.h"
#include <iostream>
#include <string.h>

int main(int argc, char *argv[]) {
  Disk disk_run;

  unsigned char buffer[BLOCK_SIZE];
  Disk::readBlock(buffer, 7000);
  char message[] = "hello world";
  memcpy(buffer, message, sizeof(message));
  Disk::writeBlock(buffer, 7000);

  unsigned char buffer2[BLOCK_SIZE];
  char message2[12];
  std::string message3;

  Disk::readBlock(buffer2, 7000);
  
  memcpy(message2, buffer2, sizeof(message2));
  message3.assign(reinterpret_cast<const char*>(buffer2));

  std::cout << message2 << std::endl;
  std::cout << message3 << std::endl;

  disk_run.~Disk();

  return 0;
}
