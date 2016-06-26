#pragma once
#ifndef _OBJECTINSTANCE_HPP_
#define _OBJECTINSTANCE_HPP_
#include <objects/GameObject.hpp>
#include <btBulletDynamicsCommon.h>

class CollisionInstance;

/**
 * @struct InstanceObject
 *  A simple object instance
 */
class InstanceObject : public GameObject
{
	float health;
public:
	glm::vec3 scale;
	CollisionInstance* body;
	std::shared_ptr<ObjectData> object;
	InstanceObject* LODinstance;
	std::shared_ptr<DynamicObjectData> dynamics;
	bool _enablePhysics;

	InstanceObject(GameWorld* engine, const glm::vec3& pos, const glm::quat& rot, const ModelRef& model, const glm::vec3& scale, std::shared_ptr< ObjectData > obj, InstanceObject* lod, std::shared_ptr< DynamicObjectData > dyn
			);
	~InstanceObject();

	Type type() { return Instance; }

	void tick(float dt);

	void changeModel(std::shared_ptr<ObjectData> incoming);

	glm::vec3 getPosition() const override;
	glm::quat getRotation() const override;

	virtual void setRotation(const glm::quat& r);
	
	virtual bool takeDamage(const DamageInfo& damage);

	void setSolid(bool solid);

	float getHealth() const { return health; }
};


#endif
