#ifndef HSM_H
#define HSM_H

#include <stdint.h>

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
    virtual void onSignalNotReady(    HSM &hsm) {}
    virtual void onSignalPowerOff(    HSM &hsm) {}

    // ADC events
    virtual void onAdcDataReady(HSM &hsm) {}

    // Loop update
    virtual void onUpdate(  HSM &hsm) {}
    virtual void onInitDone(HSM &hsm) {}
  };

  // INIT
  class Init : public State {
  public:
    static Init instance;
    void onInitDone(HSM &hsm);
  };

  // IDLE
  class Idle : public State {
  public:
    static Idle instance;
    void onEnter(HSM &hsm, State &fromState);
    void onExit(HSM &hsm, State &toState);
    void onSignalStart(HSM &hsm);
    void onSignalStartRequest(HSM &hsm);
    void onUpdate(HSM &hsm);
  };

  // RUN
  class Run : public State {
  public:
    static Run instance;
    void onEnter(HSM &hsm, State &fromState);
    void onExit(HSM &hsm, State &toState);
    void onUpdate(HSM &hsm);
    void onSignalStop(HSM &hsm);
  };

  class Sample : public Run {
  public:
    static Sample instance;
    State *getParentInstance() { return &Run::instance; }
    void onEnter(HSM &hsm, State &fromState);
    void onExit(HSM &hsm, State &toState);
    void onUpdate(HSM &hsm);
  };

  // SHUTDOWN
  class Shutdown : public State {
  public:
    static Shutdown instance;
  };


  // Current state and state transition logic
  State *currentState;
  void transitionTo(State &newState);

  // Constructor
  HSM();

  // Delegate events to the current state
  void onSignalStart()        { currentState->onSignalStart(*this);        }
  void onSignalStop()         { currentState->onSignalStop(*this);         }
  void onSignalShutdown()     { currentState->onSignalShutdown(*this);     }
  void onSignalStartRequest() { currentState->onSignalStartRequest(*this); }
  void onSignalPrepare()      { currentState->onSignalPrepare(*this);      }
  void onSignalNotReady()     { currentState->onSignalNotReady(*this);     }
  void onSignalPowerOff()     { currentState->onSignalPowerOff(*this);     }
  void onAdcDataReady()       { currentState->onAdcDataReady(*this);       }
  void onUpdate()             { currentState->onUpdate(*this);             }
  void onInitDone()           { currentState->onInitDone(*this);           }
};

#endif
