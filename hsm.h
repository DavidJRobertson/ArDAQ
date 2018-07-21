#ifndef HSM_H
#define HSM_H

#include <stdint.h>
class HPSystem;
class ADS1232;
class RTC_DS1307;
#include <SPI.h>
#include "SdFat.h"

class HSM {
  public:

  class State {
    public:
      static State instance;
      virtual State *getParentInstance() { return 0; }
      bool isDescendantOf(State *s);

      // Enter/exit state events
      virtual void onEnter(HSM &hsm, State &fromState) {}
      virtual void onInit( HSM &hsm, State &fromState) {}
      virtual void onExit( HSM &hsm, State &toState)   {}

      // HP system events
      virtual void onSignalStart(       HSM &hsm) {}
      virtual void onSignalStop(        HSM &hsm) {}
      virtual void onSignalShutdown(    HSM &hsm) {}
      virtual void onSignalStartRequest(HSM &hsm) {}
      virtual void onSignalPrepare(     HSM &hsm) {}
      virtual void onSignalReady(       HSM &hsm) {}
      virtual void onSignalNotReady(    HSM &hsm) {}
      virtual void onSignalPowerOff(    HSM &hsm) {}
      virtual void onSignalPowerOn(     HSM &hsm) {}

      // ADC events
      virtual void onAdcDataReady(HSM &hsm) {}

      // Loop update
      virtual void onUpdate(  HSM &hsm) {}
      virtual void onInitDone(HSM &hsm) {}
      virtual void onSerialAvailable(HSM &hsm) {}
  };

  // Init
  class Init : public State {
    public:
      static Init instance;
      virtual void onInitDone(HSM &hsm);
  };

  // SendStartRequest
  class SendStartRequest : public State {
    public:
      static SendStartRequest instance;
      virtual void onInit(HSM &hsm, State &fromState);
  };

  // Idle
  class Idle : public State {
    public:
      static Idle instance;
      virtual void onEnter(HSM &hsm, State &fromState);
      virtual void onExit(HSM &hsm, State &toState);
      virtual void onSignalStart(HSM &hsm);
      virtual void onSerialAvailable(HSM &hsm);
      virtual void onSignalNotReady(HSM &hsm);
      virtual void onSignalReady(HSM &hsm);
      virtual void onSignalPowerOff(HSM &hsm);
      virtual void onSignalPowerOn(HSM &hsm);
  };

  // Run
  class Run : public State {
    public:
      static Run instance;
      virtual void onEnter(HSM &hsm, State &fromState);
      virtual void onExit(HSM &hsm, State &toState);
      virtual void onInit(HSM &hsm, State &fromState);
      virtual void onSignalStop(HSM &hsm);
      virtual void onSerialAvailable(HSM &hsm);
      virtual void onSignalNotReady(HSM &hsm);
      virtual void onSignalReady(HSM &hsm);
      virtual void onSignalPowerOff(HSM &hsm);
      virtual void onSignalPowerOn(HSM &hsm);
  };

  // Run > WaitForConversion
  class WaitForConversion : public Run {
    public:
      static WaitForConversion instance;
      virtual State *getParentInstance() { return &Run::instance; }
      virtual void onEnter(HSM &hsm, State &fromState);
      virtual void onExit(HSM &hsm, State &toState);
      virtual void onInit(HSM &hsm, State &fromState) {}
      virtual void onAdcDataReady(HSM &hsm);
  };

  // Run > Sample
  class Sample : public Run {
    public:
      static Sample instance;
      virtual State *getParentInstance() { return &Run::instance; }
      virtual void onEnter(HSM &hsm, State &fromState);
      virtual void onExit(HSM &hsm, State &toState);
      virtual void onInit(HSM &hsm, State &fromState);
  };

  // Shutdown
  class Shutdown : public State {
    public:
      static Shutdown instance;
  };

  // Constructor & transitionTo method
  HSM(HPSystem &_hp, ADS1232 &_adc, RTC_DS1307 &_rtc, uint8_t _ledPin, uint8_t _sdCsPin);
  void transitionTo(State &newState);

  // Delegate events to the current state
  void onSignalStart()        { currentState->onSignalStart(*this);        }
  void onSignalStop()         { currentState->onSignalStop(*this);         }
  void onSignalShutdown()     { currentState->onSignalShutdown(*this);     }
  void onSignalStartRequest() { currentState->onSignalStartRequest(*this); }
  void onSignalPrepare()      { currentState->onSignalPrepare(*this);      }
  void onSignalNotReady()     { currentState->onSignalNotReady(*this);     }
  void onSignalReady()        { currentState->onSignalReady(*this);        }
  void onSignalPowerOff()     { currentState->onSignalPowerOff(*this);     }
  void onSignalPowerOn()      { currentState->onSignalPowerOn(*this);      }
  void onAdcDataReady()       { currentState->onAdcDataReady(*this);       }
  void onUpdate()             { currentState->onUpdate(*this);             }
  void onInitDone()           { currentState->onInitDone(*this);           }
  void onSerialAvailable()    { currentState->onSerialAvailable(*this);    }

private:
  State *currentState;

  HPSystem *hp;
  ADS1232 *adc;
  RTC_DS1307 *rtc;
  uint8_t ledPin;
  uint8_t sdCsPin;
  bool debug = false;

  uint32_t startTime;
  uint32_t sampleNumber;

  void debugPrintln(const char *str);
  void messagePrintln(const char *str);

  bool sdLogInit();
  void sdLogClose();
  bool sdLog(const char* logEntry);
  SdFat sd; // File system object.
  bool sdLogActive = false;
  SdFile file; // Log file.
};

#endif
