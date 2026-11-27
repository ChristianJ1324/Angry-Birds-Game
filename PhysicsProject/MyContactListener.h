/***********************************************************************
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) [2026] Media Design School
File Name : MyContactListener.h
Description : Handles physics contacts and impact damage
Author : Christian Jamieson
Mail : christian.jamieson@mds.ac.nz
**************************************************************************/

#pragma once

#include "box2d/box2d.h"

class MyContactListener : public b2ContactListener
{
public:
    // Contact handling
    void PreSolve(b2Contact* contact, const b2Manifold* oldManifold) override;
};