#include "pch.h"

#include "StateMachine.h"
#include "Entity.h"

StateMachine::StateMachine(Entity* owner)
    : m_owner(owner)
{
}

void StateMachine::AddState(const std::string& name, StateCallback onBegin, StateCallback onUpdate, StateCallback onEnd)
{
    m_states[name] = { onBegin, onUpdate, onEnd };
}

void StateMachine::SetInitialState(const std::string& name)
{
    m_currentState = name;
    m_stateTime = 0.0f;

    auto& state = m_states[m_currentState];
    if (state.onBegin)
        state.onBegin(m_owner, 0.f);
}

void StateMachine::AddTransition(const std::string& source, const std::string& target, std::vector<ConditionCallback> conditions)
{
    m_transitions.push_back({ source, target, conditions });
}

void StateMachine::Update(float deltaTime)
{
    if (!m_owner || m_currentState.empty())
        return;

    m_stateTime += deltaTime;

    for (const auto& transition : m_transitions)
    {
        if (transition.source != m_currentState)
            continue;

        bool canTransit = true;

        for (const auto& condition : transition.conditions)
        {
            if (!condition || !condition(m_owner, deltaTime))
            {
                canTransit = false;
                break;
            }
        }

        if (canTransit)
        {
            ChangeState(transition.target);
            return;
        }
    }

    auto& state = m_states[m_currentState];
    if (state.onUpdate)
        state.onUpdate(m_owner, deltaTime);
}

void StateMachine::ChangeState(const std::string& target)
{
    if (m_currentState == target)
        return;

    auto& current = m_states[m_currentState];
    if (current.onEnd)
        current.onEnd(m_owner, 0.f);

    m_currentState = target;
    m_stateTime = 0.0f;

    auto& next = m_states[m_currentState];
    if (next.onBegin)
        next.onBegin(m_owner, 0.f);
}

const std::string& StateMachine::GetCurrentState() const
{
    return m_currentState;
}

float StateMachine::GetStateTime() const
{
    return m_stateTime;
}

Entity* StateMachine::GetOwner()
{
    return m_owner;
}