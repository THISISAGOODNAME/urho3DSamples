//
// Created by AICDG on 2017/9/26.
//

#ifndef URHO3DSAMPLES_MOVER_H
#define URHO3DSAMPLES_MOVER_H

#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D/Math/BoundingBox.h>

using namespace Urho3D;

class Mover : public LogicComponent {
    URHO3D_OBJECT(Mover, LogicComponent);

public:
    Mover(Context* context);

    /// Set motion parameters: forward movement speed, rotation speed, and movement boundaries.
    void SetParameters(float moveSpeed, float rotateSpeed, const BoundingBox& bounds);
    /// Handle scene update. Called by LogicComponent base class.
    virtual void Update(float timeStep);

    /// Return forward movement speed.
    float GetMoveSpeed() const { return moveSpeed_; }
    /// Return rotation speed.
    float GetRotationSpeed() const { return rotationSpeed_; }
    /// Return movement boundaries.
    const BoundingBox& GetBounds() const { return bounds_; }

private:
    /// Forward movement speed.
    float moveSpeed_;
    /// Rotation speed.
    float rotationSpeed_;
    /// Movement boundaries.
    BoundingBox bounds_;
};


#endif //URHO3DSAMPLES_MOVER_H
