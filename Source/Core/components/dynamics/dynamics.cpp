#include "dynamics.h"
#include <dynamics/rigid_body.h>

void Dynamics::Step(float dt)
{
    world->stepSimulation(dt);
    for(unsigned i = 0; i < rigidBodies.size(); ++i)
        rigidBodies[i]->Update();
}

void Dynamics::AddRigidBody(RigidBody* rb)
{
    RemoveRigidBody(rb);
    rigidBodies.push_back(rb);
    world->addRigidBody(rb->rigidBody);
}

void Dynamics::RemoveRigidBody(RigidBody* rb)
{
    for(unsigned i = 0; i < rigidBodies.size(); ++i)
    {
        if(rigidBodies[i] == rb)
        {
            rigidBodies.erase(rigidBodies.begin() + i);
            world->removeRigidBody(rb->rigidBody);
            break;
        }
    }
}
