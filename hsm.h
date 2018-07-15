#ifndef HSM_H
#define HSM_H

class HSM {
  private:
    class State {
      // Enter/exit state events
      virtual void onEnter(HSM &hsm) {}
      virtual void onExit(HSM &hsm)  {}

      // HP system events
      virtual void onSignalStart(HSM &hsm) {}
      virtual void onSignalStop(HSM &hsm) {}
      virtual void onSignalShutdown(HSM &hsm) {}
      virtual void onSignalStartRequest(HSM &hsm) {}
      virtual void onSignalPrepare(HSM &hsm) {}
      virtual void onSignalNotReady(HSM &hsm) {}
      virtual void onSignalPowerOff(HSM &hsm) {}

      // ADC events
      virtual void onAdcDataReady(HSM &hsm) {}

      // Loop update
      virtual void onUpdate(HSM &hsm) {}
    };
    friend State;

    // IDLE
    class Idle : public State {
      void onEnter(HSM &hsm);
      void onEnter(HSM &hsm);
    }
    friend IdleState;

    // RUN
    class Run : public State {
      void onEnter(HSM &hsm);
      void onExit(HSM &hsm);
    }
    friend Run;


    class Sample : public Run {

    }
    friend Sample;

    // SHUTDOWN
    class Shutdown : public State {
    public:
      void onSwitchover(HSM &hsm, const char *msg);
      void onFaultTrigger(HSM &hsm, const char *msg);
    }
    friend Shutdown;

    // Leaf state instances:
    static Active Active_State;
    static Standby Standby_State;
    static Suspect Suspect_State;
    static Failed Failed_State;

    // Current state and state transition logic
    State *currentState;
    void transitionToState(State &state) {
      //calls to this function from within a state event handler should be the last thing to happen in that handler!
      currentState->onExit(*this);
      currentState = &state;
      currentState->onEnter(*this);
    }

  public:
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
};

#endif
