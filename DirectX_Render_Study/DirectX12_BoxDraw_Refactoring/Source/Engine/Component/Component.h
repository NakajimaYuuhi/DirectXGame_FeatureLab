#pragma once
#include <string>
#include "RenderLayer.h"

using String = std::string;

class CObject;

// Execution order phases for safe component updates
enum class UpdatePhase
{
    Input = 0,       // Player input reading, controller logic
    AI = 1,          // Enemy AI decision making, state transitions
    Movement = 2,    // Horizontal velocity calculation, slope checking
    Physics = 3,     // Gravity application, ground collision, snap-to-ground
    Animation = 4,   // Model animation updates, bone matrices
    PostPhysics = 5, // Camera following, post-movement adjustments
    COUNT = 6
};

class CComponent
{
public:
    CComponent() : m_ComponentName(""), m_ComponentIsValid(true), m_enabled(true), m_Owner(nullptr) {}
    CComponent(String _Name) : m_ComponentName(_Name), m_ComponentIsValid(true), m_enabled(true), m_Owner(nullptr) {}
    virtual ~CComponent() = default;

    // Lifecycle methods
    virtual void Init() {}
    virtual void Awake() {}
    virtual void Start() {}
    virtual void OnEnable() {}
    virtual void OnDisable() {}
    virtual void Update(float deltaTime) {}
    virtual void LateUpdate(float deltaTime) {}
    virtual void Draw() {}
    virtual void OnCollision(CObject* _Other) {}

    // Execution phase priority (default is Movement)
    virtual UpdatePhase GetUpdatePhase() const { return UpdatePhase::Movement; }

    // Render layer category (default is Opaque)
    virtual RenderLayer GetRenderLayer() const { return RenderLayer::Opaque; }

    // Enable / Disable management
    bool IsEnabled() const { return m_enabled && m_ComponentIsValid; }
    bool GetEnabled() const { return IsEnabled(); }
    void SetEnabled(bool enabled)
    {
        if (m_enabled != enabled)
        {
            m_enabled = enabled;
            if (m_enabled) OnEnable();
            else OnDisable();
        }
    }

    // Getter / Setter
    String GetName() const { return m_ComponentName; }
    bool GetIsValid() const { return m_ComponentIsValid; }
    bool GetIsVarid() const { return m_ComponentIsValid; } // Backward compatibility
    virtual void SetName(String _Name) { m_ComponentName = _Name; }
    virtual void SetIsValid(bool _IsValid) { m_ComponentIsValid = _IsValid; }
    void SetOwner(CObject* _Object) { m_Owner = _Object; }
    CObject* GetOwner() const { return m_Owner; }

protected:
    CObject* m_Owner = nullptr;
    String   m_ComponentName = "";
    bool     m_ComponentIsValid = true;
    bool     m_enabled = true;
};
