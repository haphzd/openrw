#ifndef LOADINGSTATE_HPP
#define LOADINGSTATE_HPP

#include <SDL2/SDL_events.h>
#include "State.hpp"

class LoadingState : public State
{
	State* next;
public:
	LoadingState(RWGame* game);

	virtual void enter();
	virtual void exit();

	virtual void tick(float dt);

	virtual void draw(GameRenderer* r);

	void setNextState(State* nextState);
	
    virtual bool shouldWorldUpdate();

	virtual void handleEvent(const SDL_Event& event);
};

#endif // LOADINGSTATE_HPP
