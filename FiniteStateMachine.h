#ifndef FINITE_STATE_MACHINE_H
#define FINITE_STATE_MACHINE_H

#define NO_ENTER  (NULL)
#define NO_UPDATE (NULL)
#define NO_EXIT   (NULL)

class State {
	public:
		State();
		State( void (*updateFunction)() );
		State( void (*enterFunction)(), void (*updateFunction)(), void (*exitFunction)() );

		void enter();
		void update();
		void exit();
	private:
		void (*userEnter)();
		void (*userUpdate)();
		void (*userExit)();
};

class FSM {
	public:
		FSM(State& current);

		FSM& update();
		FSM& transitionTo( State& state );
		FSM& immediateTransitionTo( State& state );

		State& getCurrentState();
		bool isInState( State &state ) const;

	private:
		bool 	needToTriggerEnter;
		State* 	currentState;
		State* 	nextState;
};

#endif
