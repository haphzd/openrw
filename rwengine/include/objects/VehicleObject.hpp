#pragma once
#ifndef _VEHICLEOBJECT_HPP_
#define _VEHICLEOBJECT_HPP_
#include <engine/GameObject.hpp>
#include <map>
#include <objects/VehicleInfo.hpp>
#include <data/CollisionInstance.hpp>

/**
 * @class VehicleObject
 * Implements Vehicle behaviours.
 */
struct VehicleObject : public GameObject
{
public:

	enum /*DamageFlags*/ {
		DF_Bonnet     =  1,
		DF_Door_lf    =  2,
		DF_Door_rf    =  4,
		DF_Door_lr    =  8,
		DF_Door_rr    = 16,
		DF_Boot       = 32,
		DF_Windscreen = 64,
		DF_Bump_front = 128,
		DF_Bump_rear  = 256,
		DF_Wing_lf    = 512,
		DF_Wing_rf    = 1024,
		DF_Wing_lr    = 2048,
		DF_Wing_rr    = 4096,
	};

private:
	float steerAngle;
	float throttle;
	float brake;
	bool handbrake;
	unsigned int damageFlags;
public:

	VehicleDataHandle vehicle;
	VehicleInfoHandle info;
	glm::vec3 colourPrimary;
	glm::vec3 colourSecondary;
	
	std::map<size_t, GameObject*> seatOccupants;

	CollisionInstance* collision;
	btRigidBody* physBody;
	btVehicleRaycaster* physRaycaster;
	btRaycastVehicle* physVehicle;

	struct HingeInfo {
		btRigidBody* body;
		btHingeConstraint* constraint;
	};

	std::map<ModelFrame*, HingeInfo> _hingedObjects;

	VehicleObject(GameWorld* engine,
			   const glm::vec3& pos,
			   const glm::quat& rot,
			   ModelHandle* model,
			   VehicleDataHandle data,
			   VehicleInfoHandle info,
			   const glm::vec3& prim,
			   const glm::vec3& sec);
	
	virtual ~VehicleObject();
	
	void setPosition(const glm::vec3& pos);

	glm::vec3 getPosition() const;

	void setRotation(const glm::quat &orientation);

	glm::quat getRotation() const;

	Type type() { return Vehicle; }

	void setSteeringAngle(float);

	float getSteeringAngle() const;

	void setThrottle(float);

	float getThrottle() const;

	void setBraking(float);

	float getBraking() const;

	void setHandbraking(bool);

	bool getHandbraking() const;

	void tick(float dt);

	void tickPhysics(float dt);
	
	void ejectAll();
	
	GameObject* getOccupant(size_t seat);
	
	void setOccupant(size_t seat, GameObject* occupant);

	glm::vec3 getSeatEntryPosition(size_t seat) const {
		auto pos = info->seats[seat].offset;
		pos -= glm::vec3(glm::sign(pos.x) * -0.81756252f, 0.34800607f, -0.486281008f);
		return getPosition() + getRotation() * pos;
	}
	
    virtual bool takeDamage(const DamageInfo& damage);

	enum FrameState {
		OK,
		DAM,
		BROKEN
	};

	void setFrameState(ModelFrame *f, FrameState state);

	void applyWaterFloat(const glm::vec3& relPt);

	void setHingeLocked(ModelFrame* frame, bool locked);

private:

	void createObjectHinge(btTransform &local, ModelFrame* frame);
	void destroyObjectHinge(HingeInfo& hinge);
};

/**
 * Implements vehicle ray casting behaviour.
 * i.e. ignore the god damn vehicle body when casting rays.
 */
class VehicleRaycaster : public btVehicleRaycaster
{
	btDynamicsWorld* _world;
	VehicleObject* _vehicle;
public:
	VehicleRaycaster(VehicleObject* vehicle, btDynamicsWorld* world)
		: _world(world), _vehicle(vehicle) {}

	void* castRay(const btVector3 &from, const btVector3 &to, btVehicleRaycasterResult &result);
};

#endif
