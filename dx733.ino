/*
 *  Little project to turn on/off a Optoma DX733 projector via serial port
 *  Didn't get the original remote when I bought it. Now using a spare
 *  button on the remote of my X5 Xnano Android TV box
 *  
 *  Compile with stm32duino
 *  * Board Generic STM32F0 Series
 *  * Optimize: Smallest with LTO
 *  * Use patched IRremote library
 *  
 *  IR-Receiver <-> STM32F030F4 <-> MAX3232 <-> DX733
 */

#include <IRremote.h>


int RECV_PIN = PA9;

IRrecv irrecv(RECV_PIN);

decode_results results;

// IR codes of 4X2 HDMI matrix
#define IR_HDMI_POWER 0xFF28D7
#define IR_HDMI_A_POWER 0xFFB04F
#define IR_HDMI_A_1 0xFF9867
#define IR_HDMI_A_2 0xFFA857
#define IR_HDMI_A_3 0xFF0AF5
#define IR_HDMI_A_4 0xFF52AD
#define IR_HDMI_B_POWER 0xFF40BF
#define IR_HDMI_B_1 0xFF18E7
#define IR_HDMI_B_2 0xFF8A75
#define IR_HDMI_B_3 0xFFCA35
#define IR_HDMI_B_4 0xFFA25D

// IR codes of ???
#define IR_VOLUME_POWER 0x5CB04F
#define IR_VOLUME_PLUS 0x5CA857
#define IR_VOLUME_MINUS 0x5C906F

// IR codes of TV control section of the X5 Xnano Android box
// #define IR_X5_TV_SET 0x
#define IR_X5_TV_AV 0x807F2FD0
#define IR_X5_TV_POWER 0x807F8F70
#define IR_X5_TV_VOLP 0x807F4FB0
#define IR_X5_TV_VOLM 0x807FCF30

#define IR_REPEAT  0xFFFFFFFF


#define TIMER_BLINK  TIM14
static stimer_t TimHandle;

extern void IRTimer();

void callback(stimer_t *htim){
  UNUSED(htim);
  IRTimer();
}

void setup() {
 Serial.begin(9600);
 pinMode(LED_BUILTIN, OUTPUT);
 //delay(5000);
 //Serial.println("ID00IR101");
 irrecv.enableIRIn();
 
 TimHandle.timer = TIMER_BLINK;
 //TimerHandleInit(&TimHandle, 10000 - 1, ((uint32_t)(getTimerClkFreq(TIMER_BLINK) / (1000000)) - 1));
 TimerHandleInit(&TimHandle, 50 - 1, ((uint32_t)(getTimerClkFreq(TIMER_BLINK) / (1000000)) - 1));
 attachIntHandle(&TimHandle, callback);
 
}

uint32_t lastbutton = -1;
uint32_t lastseen = 0;
int repeated = 0;

bool beamer_on = false;

bool  processbutton(uint32_t button, bool repeat){
  if(!repeat && button == IR_X5_TV_POWER){
    beamer_on = !beamer_on;
    if(beamer_on){
      Serial.println("ID00IR101");
    } else {
      Serial.println("ID00IR100");
    }
    digitalWrite(LED_BUILTIN, beamer_on);
  }
  return false;
}

void loop() {
  if (irrecv.decode(&results)) {
    uint32_t button = results.value;
    //Serial.println(button, HEX);
    if (button == IR_REPEAT){
      if (millis() - 1000 < lastseen){
        repeated++;
        if (repeated > 5){
          repeated = 0;
          processbutton(lastbutton, true);
        }
      }
    } else {
      lastbutton = button;
      repeated = 0;
      bool setlastmode = processbutton(button, false);
    }

    lastseen = millis();
    irrecv.resume(); // Receive the next value
  }
}
