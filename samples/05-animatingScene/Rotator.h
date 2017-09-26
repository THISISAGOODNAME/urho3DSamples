//
// Created by AICDG on 2017/9/26.
//

#ifndef URHO3DSAMPLES_ROTATOR_H
#define URHO3DSAMPLES_ROTATOR_H

#include <Urho3D/Scene/LogicComponent.h>

using namespace Urho3D;

class Rotator : public LogicComponent {
    URHO3D_OBJECT(Rotator, LogicComponent);

public:
    Rotator(Context* context);

    /// Set rotation speed about the Euler axes. Will be scaled with scene update time step.
    void SetRotationSpeed(const Vector3& speed);
    /// Handle scene update. Called by LogicComponent base class.
    virtual void Update(float timeStep);

    /// Return rotation speed.
    const Vector3& GetRotationSpeed() const { return rotationSpeed_; }

private:
    /// Rotation speed.
    Vector3 rotationSpeed_;
};


#endif //URHO3DSAMPLES_ROTATOR_H
