#pragma once

class RailEditorScript : public Script
{
public:
    RailEditorScript() = default;
    virtual ~RailEditorScript() = default;

    virtual void Init() override;
    virtual void Update(float dt) override;
    virtual std::unique_ptr<Script> Clone() const override;

    float m_rotationSpeed = 180.f;
    float m_pitchSpeed = 180.f;

private:
    XMFLOAT3 m_initialPosition{ 0.f, 0.f, 0.f };
    XMFLOAT4 m_initialRotation{ 0.f, 0.f, 0.f, 1.f };
};