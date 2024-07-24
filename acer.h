#include <Arduino.h>

class Acer {
public:
  Acer(HardwareSerial &port, uint8_t id = 0)
    : port_{ port }, id_{ id } {}

  void begin() {
  }

  void powerOn() {
    send("OKOKOKOKOK");
  }

  void powerOff() {
    send("* 0 IR 002");
  }

private:
  HardwareSerial &port_;
  uint8_t id_;

  void send(const char *cmd) {
    port_.write(cmd);
    port_.write("\r");
    port_.flush();
  }
};
