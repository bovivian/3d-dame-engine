#include <btBulletDynamicsCommon.h>

#pragma once

class Physics
{
	private:
		btBroadphaseInterface * broadphase;
		btDefaultCollisionConfiguration * collisionConfiguration;
		btCollisionDispatcher * dispatcher;
		btSequentialImpulseConstraintSolver * solver;
		btDiscreteDynamicsWorld * dynamicsWorld;
	public:
		Physics();
		~Physics();

		bool Init();
		void Update();
		btDiscreteDynamicsWorld * GetDynamicsWorld();
};
