#ifndef MENUSTATE_HPP
#define MENUSTATE_HPP

#include <SDL2/SDL_events.h>
#include "State.hpp"

class MenuState : public State
{
public:
	MenuState(RWGame* game);

	virtual void enter();
	virtual void exit();

	virtual void tick(float dt);

	virtual void enterMainMenu();
	virtual void enterLoadMenu();

	virtual void handleEvent(const SDL_Event& event);
};

#endif // MENUSTATE_HPP
