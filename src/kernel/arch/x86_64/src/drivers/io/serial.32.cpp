#include <drivers/io/serial.hpp>

#include <drivers/io/ports.hpp>
#include <common/types.hpp>

namespace drivers::io {

#define SERIAL_DATA_PORT(base) (base)
#define SERIAL_FIFO_COMMAND_PORT(base) (base + 2)
#define SERIAL_LINE_COMMAND_PORT(base) (base + 3)
#define SERIAL_MODEM_COMMAND_PORT(base) (base + 4)
#define SERIAL_LINE_STATUS_PORT(base) (base + 5)

// Tells the port to expect the highest 8 bits then the lowest 8 bits on the data port
#define SERIAL_LINE_ENABLE_DLAB 0x80

void serialConfigureBaudRate(uint16_t com, uint16_t divisor) {
  outB(SERIAL_LINE_COMMAND_PORT(com), SERIAL_LINE_ENABLE_DLAB);
  outB(SERIAL_DATA_PORT(com), (divisor >> 8) & 0x00FF);
  outB(SERIAL_DATA_PORT(com), divisor & 0x00FF);
}

void serialConfigureLine(uint16_t com) {
  // dlab | break ctrl | parity bits | stop bits | data-length
  // 0    | 0          | 0 0 0       | 0         | 1 1 = 0x3
  outB(SERIAL_LINE_COMMAND_PORT(com), 0x3);
}

void serialConfigure(uint16_t com, uint16_t divisor) {
  serialConfigureBaudRate(com, divisor);
  serialConfigureLine(com);
}

bool serialIsTransmitFifoEmpty(uint16_t com) {
  return (inB(SERIAL_LINE_STATUS_PORT(com)) & 0x20) != 0;
}

void serialWrite(const char* data, uint16_t com) {
  while (!serialIsTransmitFifoEmpty(com)) {};
  for (uint32_t i = 0; data[i]; i++) {
    outB(SERIAL_DATA_PORT(com), data[i]);
  }
}
}
