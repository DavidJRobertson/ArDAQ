#include "FiniteStateMachine.h"

// STATE
State::State() {
	userEnter  = 0;
	userUpdate = 0;
	userExit   = 0;
}
State::State( void (*updateFunction)() ) {
	userEnter  = 0;
	userUpdate = updateFunction;
	userExit   = 0;
}
State::State( void (*enterFunction)(), void (*updateFunction)(), void (*exitFunction)() ){
	userEnter  = enterFunction;
	userUpdate = updateFunction;
	userExit   = exitFunction;
}
void State::enter(){
	if (userEnter){
		userEnter();
	}
}
void State::update(){
	if (userUpdate){
		userUpdate();
	}
}
void State::exit(){
	if (userExit){
		userExit();
	}
}




// FINITE STATE MACHINE
FSM::FSM(State& current){
	needToTriggerEnter = true;
	currentState = nextState = &current;
}
FSM& FSM::update() {
	//simulate a transition to the first state
	//this only happens the first time update is called
	if (needToTriggerEnter) {
		currentState->enter();
		needToTriggerEnter = false;
	} else {
		if (currentState != nextState){
			immediateTransitionTo(*nextState);
		}
		currentState->update();
	}
	return *this;
}
FSM& FSM::transitionTo(State& state){
	nextState = &state;
	return *this;
}
FSM& FSM::immediateTransitionTo(State& state){
	currentState->exit();
	currentState = nextState = &state;
	currentState->enter();
	return *this;
}
State& FSM::getCurrentState() {
	return *currentState;
}
bool FSM::isInState( State &state ) const {
	return (&state == currentState);
}
