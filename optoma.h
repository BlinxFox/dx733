#include <Arduino.h>

class Optoma {
public:
  Optoma(HardwareSerial &port, uint8_t id = 0)
    : port_{ port }, id_{ id } {}

  void begin() {
    // nothing to do
  }

  void powerOn() {
    send("IR101");
  }

  void powerOff() {
    send("IR100");
  }

private:
  HardwareSerial &port_;
  uint8_t id_;

  void send(const char *cmd) {
    char buf[16];
    snprintf(buf, sizeof(buf), "ID%02d%s", id_, cmd);
    port_.println(buf);
  }
};
