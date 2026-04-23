#pragma once

#include <unordered_map>
#include <vector>
#include <string>
#include <functional>

class Entity;

class StateMachine
{
public:
    using StateCallback = std::function<void(Entity*, float dt)>;
    using ConditionCallback = std::function<bool(Entity*, float dt)>;

private:

    struct State
    {
        StateCallback onBegin = nullptr;
        StateCallback onUpdate = nullptr;
        StateCallback onEnd = nullptr;
    };

    struct Transition
    {
        std::string source;
        std::string target;
        std::vector<ConditionCallback> conditions;
    };

private:
    Entity* m_owner = nullptr;

    std::unordered_map<std::string, State> m_states;
    std::vector<Transition> m_transitions;

    std::string m_currentState;
    float m_stateTime = 0.0f;

public:
    StateMachine(Entity* owner);

    void AddState(const std::string& name, StateCallback onBegin = nullptr, StateCallback onUpdate = nullptr, StateCallback onEnd = nullptr);

    void SetInitialState(const std::string& name);

    void AddTransition(const std::string& source, const std::string& target, std::vector<ConditionCallback> conditions);

    void Update(float deltaTime);

    void ChangeState(const std::string& target);

    const std::string& GetCurrentState() const;
    float GetStateTime() const;

    Entity* GetOwner();
};