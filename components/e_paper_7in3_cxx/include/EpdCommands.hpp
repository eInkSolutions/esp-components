#pragma once

#include <stddef.h>
#include <stdint.h>

namespace Epd {

/// Command definitions for the e-paper display controller.
///
/// The controller documentation describes each command as one or more
/// interface transfers using the following fields:
///
///   W/R : 0 = write, 1 = read
///   C/D : 0 = command, 1 = data
///   D7~D0 : 8-bit value transferred during the write/read cycle
///
/// The "Setting" column in the documentation is the hexadecimal
/// representation of D7~D0. For example:
///
///   D7~D0 = 0000 0111 -> Setting = 07h
///   D7~D0 = 1010 0101 -> Setting = A5h
///
/// A command therefore consists of a command byte (C/D = 0), optionally
/// followed by one or more data bytes (C/D = 1).
///
/// For example, the Deep Sleep command is documented as:
///
///   C/D = 0, D7~D0 = 0000 0111 -> 0x07 (command)
///   C/D = 1, D7~D0 = 1010 0101 -> 0xA5 (data)
///
/// This is represented by EpdCommand::DeepSleep as:
///
///   opcode = 0x07
///   data   = { 0xA5 }
///
/// Commands such as DataStart (0x10) are followed by a variable-length
/// data stream rather than fixed command parameters. The variable data is
/// therefore written separately by the display driver.
class EpdCommands {
public:
  static constexpr uint8_t PowerOffOpcode = 0x02;
  static constexpr uint8_t PowerOnOpcode = 0x04;
  static constexpr uint8_t DeepSleepOpcode = 0x07;
  static constexpr uint8_t DataStartOpcode = 0x10;
  static constexpr uint8_t DataRefreshOpcode = 0x12;

  struct Command {
    uint8_t opcode;
    const uint8_t *data;
    size_t dataSize;
  };

  static constexpr uint8_t PowerOffData[] = {0x00};
  static constexpr uint8_t PowerOnData[] = {0x00};
  static constexpr uint8_t DeepSleepData[] = {0xA5};
  static constexpr uint8_t DataRefreshData[] = {0x01};

  static constexpr Command PowerOff{PowerOffOpcode, PowerOffData,
                                    sizeof(PowerOffData)};

  static constexpr Command PowerOn{PowerOnOpcode, PowerOnData,
                                   sizeof(PowerOnData)};

  static constexpr Command DeepSleep{DeepSleepOpcode, DeepSleepData,
                                     sizeof(DeepSleepData)};

  static constexpr Command DataStart{DataStartOpcode, nullptr, 0};

  static constexpr Command DataRefresh{DataRefreshOpcode, DataRefreshData,
                                       sizeof(DataRefreshData)};

  static constexpr Command create(uint8_t opcode, const uint8_t *data = nullptr,
                                  size_t dataSize = 0) {
    return Command{opcode, data, dataSize};
  }
};

} // namespace Epd
