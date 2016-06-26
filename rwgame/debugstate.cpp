#include "debugstate.hpp"
#include "RWGame.hpp"
#include <ai/PlayerController.hpp>
#include <objects/CharacterObject.hpp>
#include <objects/InstanceObject.hpp>
#include <objects/VehicleObject.hpp>
#include <engine/GameState.hpp>
#include <items/InventoryItem.hpp>
#include <data/WeaponData.hpp>
#include <sstream>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>

constexpr float kDebugEntryHeight = 14.f;
const glm::vec2 kDebugMenuOffset = glm::vec2(10.f, 50.f);

static void jumpCharacter(RWGame* game, CharacterObject* player, const glm::vec3& target, bool ground = true)
{
	glm::vec3 newPosition = target;
	if (ground) {
		newPosition = game->getWorld()->getGroundAtPosition(newPosition) + glm::vec3(0.f, 0.f, 1.f);
	}
	if( player )
	{
		if( player->getCurrentVehicle() )
		{
			player->getCurrentVehicle()->setPosition(newPosition);
		}
		else
		{
			player->setPosition(newPosition);
		}
	}
}

Menu* DebugState::createDebugMenu()
{
	CharacterObject* player = nullptr;
	if (game->getPlayer()) {
		player = game->getPlayer()->getCharacter();
	}

	Menu* m = new Menu(2);
	m->offset = kDebugMenuOffset;
#if 0
	m->addEntry(Menu::lambda("Random Vehicle", [this] {
		auto it = getWorld()->vehicleTypes.begin();
		std::uniform_int_distribution<int> uniform(0, 3);
		for(size_t i = 0, n = uniform(getWorld()->randomEngine); i != n; i++) {
			it++;
		}
		spawnVehicle(it->first);
	}, entryHeight));
	m->addEntry(Menu::lambda("Open All Doors/Flaps", [=] {
		auto pc = getWorld()->state->player->getCharacter();
		auto pv = pc->getCurrentVehicle();
		if( pv ) {
			for(auto& it : pv->_hingedObjects) {
				pv->setHingeLocked(it.first, false);
			}
		}
	}, entryHeight));

	m->addEntry(Menu::lambda("Spawn Pedestrians", [=] {
		glm::vec3 hit, normal;
		if(game->hitWorldRay(hit, normal)) {
			glm::vec3 spawnPos = hit + glm::vec3(-5, 0.f, 0.0) + normal;
			size_t k = 1;
			// Spawn every pedestrian.
			for(auto it = getWorld()->pedestrianTypes.begin();
				it != getWorld()->pedestrianTypes.end(); ++it) {
				getWorld()->createPedestrian(it->first, spawnPos);
				spawnPos += glm::vec3(2.5, 0, 0);
				if((k++ % 6) == 0) { spawnPos += glm::vec3(-15, -2.5, 0); }
			}
		}
	}, entryHeight));

	int vehiclesMax = 16, i = 0;
	for( auto& v : getWorld()->vehicleTypes ) {
		if( (i++) > vehiclesMax ) break;
		m->addEntry(Menu::lambda(v.second->handlingID, [=] {
			spawnVehicle(v.first);
		}, entryHeight));
	}
#endif
	m->addEntry(Menu::lambda("Jump to Debug Camera", [=] {
		jumpCharacter(game, player, _debugCam.position + _debugCam.rotation * glm::vec3(3.f, 0.f, 0.f), false);
	}, kDebugEntryHeight));

	m->addEntry(Menu::lambda("-Map", [=] {
		this->enterMenu(createMapMenu());
	}, kDebugEntryHeight));
	m->addEntry(Menu::lambda("-Vehicles", [=] {
		this->enterMenu(createVehicleMenu());
	}, kDebugEntryHeight));
	m->addEntry(Menu::lambda("-AI", [=] {
		this->enterMenu(createAIMenu());
	}, kDebugEntryHeight));
	m->addEntry(Menu::lambda("-Weapons", [=] {
		this->enterMenu(createWeaponMenu());
	}, kDebugEntryHeight));

	m->addEntry(Menu::lambda("Set Super Jump", [=] {
		player->setJumpSpeed(20.f);
	}, kDebugEntryHeight));
	m->addEntry(Menu::lambda("Set Normal Jump", [=] {
		player->setJumpSpeed(CharacterObject::DefaultJumpSpeed);
	}, kDebugEntryHeight));
	m->addEntry(Menu::lambda("Full Health", [=] {
		player->getCurrentState().health = 100.f;
	}, kDebugEntryHeight));
	m->addEntry(Menu::lambda("Full Armour", [=] {
		player->getCurrentState().armour = 100.f;
	}, kDebugEntryHeight));
	m->addEntry(Menu::lambda("Cull Here", [=] {
		game->getRenderer()->setCullOverride(true, _debugCam);
	}, kDebugEntryHeight));


	// Optional block if the player is in a vehicle
	auto cv = player->getCurrentVehicle();
	if(cv) {
		m->addEntry(Menu::lambda("Flip vehicle", [=] {
			cv->setRotation(cv->getRotation() * glm::quat(glm::vec3(0.f, glm::pi<float>(), 0.f)));
		}, kDebugEntryHeight));
	}

	return m;
}

Menu* DebugState::createMapMenu()
{
	CharacterObject* player = nullptr;
	if (game->getPlayer()) {
		player = game->getPlayer()->getCharacter();
	}

	Menu* m = new Menu(2);
	m->offset = kDebugMenuOffset;

	m->addEntry(Menu::lambda("Back", [=] {
		this->enterMenu(createDebugMenu());
	}, kDebugEntryHeight));

	m->addEntry(Menu::lambda("Jump to Docks", [=] {
		jumpCharacter(game, player, glm::vec3(1390.f, -837.f, 100.f));
	}, kDebugEntryHeight));
	m->addEntry(Menu::lambda("Jump to Garage", [=] {
		jumpCharacter(game, player, glm::vec3(270.f, -605.f, 40.f));
	}, kDebugEntryHeight));
	m->addEntry(Menu::lambda("Jump to Airport", [=] {
		jumpCharacter(game, player, glm::vec3(-950.f, -980.f, 12.f));
	}, kDebugEntryHeight));
	m->addEntry(Menu::lambda("Jump to Hideout", [=] {
		jumpCharacter(game, player, glm::vec3(875.0, -309.0, 100.0));
	}, kDebugEntryHeight));
	m->addEntry(Menu::lambda("Jump to Luigi's", [=] {
		jumpCharacter(game, player, glm::vec3(902.75, -425.56, 100.0));
	}, kDebugEntryHeight));
	m->addEntry(Menu::lambda("Jump to Hospital", [=] {
		jumpCharacter(game, player, glm::vec3(1123.77, -569.15, 100.0));
	}, kDebugEntryHeight));
	m->addEntry(Menu::lambda("Unsolid garage doors", [=] {

		std::vector<std::string> garageDoorModels {
			"8ballsuburbandoor",
			"amcogaragedoor",
			"bankjobdoor",
			"bombdoor",
			"crushercrush",
			"crushertop",
			"door2_garage",
			"door3_garage",
			"door4_garage",
			"door_bombshop",
			"door_col_compnd_01",
			"door_col_compnd_02",
			"door_col_compnd_03",
			"door_col_compnd_04",
			"door_col_compnd_05",
			"door_jmsgrage",
			"door_sfehousegrge",
			"double_garage_dr",
			"impex_door",
			"impexpsubgrgdoor",
			"ind_plyrwoor",
			"ind_slidedoor",
			"jamesgrge_kb",
			"leveldoor2",
			"oddjgaragdoor",
			"plysve_gragedoor",
			"SalvGarage",
			"shedgaragedoor",
			"Sub_sprayshopdoor",
			"towergaragedoor1",
			"towergaragedoor2",
			"towergaragedoor3",
			"vheistlocdoor"
		};

		auto gw = game->getWorld();
		for(auto& i : gw->instancePool.objects) {
			auto obj = static_cast<InstanceObject*>(i.second);
			if (std::find(garageDoorModels.begin(), garageDoorModels.end(), obj->model->name) != garageDoorModels.end()) {
				obj->setSolid(false);
			}
		}
	}, kDebugEntryHeight));

	return m;
}

Menu* DebugState::createVehicleMenu()
{
	Menu* m = new Menu(2);
	m->offset = kDebugMenuOffset;

	m->addEntry(Menu::lambda("Back", [=] {
		this->enterMenu(createDebugMenu());
	}, kDebugEntryHeight));

	const std::map<std::string, int> kVehicleTypes = {
	    {"Landstalker", 90},
	    {"Taxi", 110},
	    {"Firetruck", 97},
	    {"Police", 116},
	    {"Ambulance", 106},
	    {"Bobcat", 112},
	    {"Banshee", 119},
	    {"Rhino", 122},
	    {"Barracks", 123},
	    {"Rumpo", 130},
	    {"Columbian", 138},
	    {"Dodo", 126},
	    {"Speeder", 142},
	    {"Yakuza", 136},
	};

	for (auto& e : kVehicleTypes) {
		m->addEntry(Menu::lambda(e.first, [=] { spawnVehicle(e.second); },
		                         kDebugEntryHeight));
	}

	return m;
}

Menu* DebugState::createAIMenu()
{
	Menu* m = new Menu(2);
	m->offset = kDebugMenuOffset;

	m->addEntry(Menu::lambda("Back", [=] {
		this->enterMenu(createDebugMenu());
	}, kDebugEntryHeight));

	const std::map<std::string, int> kPedTypes = {
	    {"Triad", 12},
	    {"Cop", 1},
	    {"SWAT", 2},
	    {"FBI", 3},
	    {"Fireman", 6},
	    {"Construction", 74},
	};

	for (auto& e : kPedTypes) {
		m->addEntry(Menu::lambda(e.first + " Follower", [=] {
			spawnFollower(e.second);
		}, kDebugEntryHeight));
	}

	m->addEntry(Menu::lambda("Kill All Peds", [=] {
		for (auto& p : game->getWorld()->pedestrianPool.objects) {
			if (p.second->getLifetime() == GameObject::PlayerLifetime) {
				continue;
			}
			p.second->takeDamage({p.second->getPosition(),
			                      p.second->getPosition(), 100.f,
			                      GameObject::DamageInfo::Explosion, 0.f});
		}
	}, kDebugEntryHeight));
	return m;
}

Menu*DebugState::createWeaponMenu()
{
	Menu* m = new Menu(2);
	m->offset = kDebugMenuOffset;

	m->addEntry(Menu::lambda("Back", [=] {
		this->enterMenu(createDebugMenu());
	}, kDebugEntryHeight));

	for (int i = 1; i < maxInventorySlots; ++i) {
		auto item = getWorld()->getInventoryItem(i);
		auto& name = getWorld()->data->weaponData[i]->name;
		m->addEntry(Menu::lambda(name, [=] {
			giveItem(item);
		}, kDebugEntryHeight));
	}

	return m;
}

DebugState::DebugState(RWGame* game, const glm::vec3& vp, const glm::quat& vd)
	: State(game)
	, _freeLook( false )
	, _sonicMode( false )
{
	this->enterMenu(createDebugMenu());

	_debugCam.position = vp;
	_debugCam.rotation = vd;
}

void DebugState::enter()
{
	getWindow().showCursor();
}

void DebugState::exit()
{

}

void DebugState::tick(float dt)
{
	/*if(debugObject) {
		auto p = debugObject->getPosition();
		ss << "Position: " << p.x << " " << p.y << " " << p.z << std::endl;
		ss << "Health: " << debugObject->mHealth << std::endl;
		if(debugObject->model) {
			auto m = debugObject->model;
			ss << "Textures: " << std::endl;
			for(auto it = m->geometries.begin(); it != m->geometries.end();
				++it )
			{
				auto g = *it;
				for(auto itt = g->materials.begin(); itt != g->materials.end();
					++itt)
				{
					for(auto tit = itt->textures.begin(); tit != itt->textures.end();
						++tit)
					{
						ss << " " << tit->name << std::endl;
					}
				}
			}
		}
		if(debugObject->type() == GameObject::Vehicle) {
			GTAVehicle* vehicle = static_cast<GTAVehicle*>(debugObject);
			ss << "ID: " << vehicle->info->handling.ID << std::endl;
		}
	}*/

	if (_freeLook) {
		_debugCam.rotation = glm::angleAxis(_debugLook.x, glm::vec3(0.f, 0.f, 1.f))
			* glm::angleAxis(_debugLook.y, glm::vec3(0.f, 1.f, 0.f));

		_debugCam.position += _debugCam.rotation * _movement * dt * (_sonicMode ? 1000.f : 100.f);
	}
}

void DebugState::draw(GameRenderer* r)
{
	// Draw useful information like camera position.
	std::stringstream ss;
	ss << "Camera Position: " << glm::to_string(_debugCam.position);

	TextRenderer::TextInfo ti;
	ti.text = ss.str();
	ti.font = 2;
	ti.screenPosition = glm::vec2( 10.f, 10.f );
	ti.size = 15.f;
	ti.baseColour = glm::u8vec3(255);
	r->text.renderText(ti);

	State::draw(r);
}

void DebugState::handleEvent(const SDL_Event& event)
{
	switch(event.type) {
	case SDL_KEYDOWN:
		switch(event.key.keysym.sym) {
		default: break;
		case SDLK_ESCAPE:
			StateManager::get().exit();
			break;
		case SDLK_w:
			_movement.x = 1.f;
			break;
		case SDLK_s:
			_movement.x =-1.f;
			break;
		case SDLK_a:
			_movement.y = 1.f;
			break;
		case SDLK_d:
			_movement.y =-1.f;
			break;
		case SDLK_f:
			_freeLook = !_freeLook;
			if (_freeLook)
				getWindow().hideCursor();
			else
				getWindow().showCursor();
			break;
		case SDLK_p:
			printCameraDetails();
			break;
		}

		switch (event.key.keysym.mod) {
		case KMOD_LSHIFT:
			_sonicMode = true;
			break;
		default: break;
		}

		break;

	case SDL_KEYUP:
		switch(event.key.keysym.sym) {
		case SDLK_w:
		case SDLK_s:
			_movement.x = 0.f;
			break;
		case SDLK_a:
		case SDLK_d:
			_movement.y = 0.f;
			break;
		}

		switch (event.key.keysym.mod) {
		case KMOD_LSHIFT:
			_sonicMode = false;
			break;
		default: break;
		}

		break;

	case SDL_MOUSEMOTION:
		if (game->hasFocus())
		{
			glm::ivec2 screenSize = getWindow().getSize();
			glm::vec2 mouseMove(event.motion.xrel / static_cast<float>(screenSize.x),
			                    event.motion.yrel / static_cast<float>(screenSize.y));

			if (!_invertedY)
				mouseMove.y = -mouseMove.y;

			_debugLook.x -= mouseMove.x;

			float qpi = glm::half_pi<float>();
			_debugLook.y -= glm::clamp(mouseMove.y, -qpi, qpi);
		}
		break;

	default: break;
	}
	State::handleEvent(event);
}

void DebugState::printCameraDetails()
{
	std::cout << " " << _debugCam.position.x << " " << _debugCam.position.y << " " << _debugCam.position.z
			  << " " << _debugCam.rotation.x << " " << _debugCam.rotation.y << " " << _debugCam.rotation.z
			  << " " << _debugCam.rotation.w << std::endl;
}

void DebugState::spawnVehicle(unsigned int id)
{
	auto ch = game->getPlayer()->getCharacter();
	if (!ch)
		return;

	auto playerRot = ch->getRotation();
	auto spawnPos = ch->getPosition();
	spawnPos += playerRot * glm::vec3(0.f, 3.f, 0.f);
	auto spawnRot = glm::quat(
	    glm::vec3(0.f, 0.f, glm::roll(playerRot) + glm::half_pi<float>()));
	getWorld()->createVehicle(id, spawnPos, spawnRot);
}

void DebugState::spawnFollower(unsigned int id)
{
	auto ch = game->getPlayer()->getCharacter();
	if(! ch) return;

	glm::vec3 fwd = ch->rotation * glm::vec3(0.f, 1.f, 0.f);

	glm::vec3 hit, normal;
	if(game->hitWorldRay(ch->position + (fwd * 10.f), {0.f, 0.f, -2.f}, hit, normal)) {
		auto spawnPos = hit + normal;
		auto follower = game->getWorld()->createPedestrian(id, spawnPos);
		jumpCharacter(game, follower, spawnPos);
		follower->controller->setGoal(CharacterController::FollowLeader);
		follower->controller->setTargetCharacter(ch);
	}
}

void DebugState::giveItem(InventoryItem* item)
{
	CharacterObject* player = nullptr;
	if (game->getPlayer()) {
		player = game->getPlayer()->getCharacter();
	}

	if (player) {
		player->addToInventory(item);
		auto& wep =
		    player->getCurrentState().weapons[item->getInventorySlot()];
		wep.bulletsTotal = 100;
		wep.bulletsClip = 0;
	}
}

const ViewCamera &DebugState::getCamera()
{
	return _debugCam;
}
