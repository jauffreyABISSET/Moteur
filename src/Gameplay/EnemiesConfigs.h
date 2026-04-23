#pragma once

struct EnemyConfig
{
    float speed = 0.f;
    float followDistance = 0.f;
    float attackDistance = 0.f;
    float attackCooldown = 0.f;
    int healthPoint = 0;
    int damage = 0;

    int pointsGiven = 0;

    XMFLOAT4 baseColor = Color::Black;
};

struct GhostEnemyConfig : EnemyConfig
{
    GhostEnemyConfig()
    {
        speed = 12.f;
        followDistance = 500.f;
        attackDistance = 10.f;
        attackCooldown = 2.5f;
        healthPoint = 4;
        damage = 3;

        pointsGiven = 100;

        baseColor = Color::Magenta;
    }
};

struct ShooterEnemyConfig : EnemyConfig
{
    ShooterEnemyConfig()
    {
        speed = 4.f;
        followDistance = 600.f;
        attackDistance = 60.f;
        attackCooldown = 2.5f;
        healthPoint = 4;
        damage = 2;

        pointsGiven = 200;

        baseColor = Color::Red;
    }
};

struct RailEnemyConfig : EnemyConfig
{
    RailEnemyConfig()
    {
        speed = 6.f;
        followDistance = -1.f;
        attackDistance = 12.f;
        attackCooldown = 1.8f;
        healthPoint = 2;
        damage = 2;

        pointsGiven = 150;

        baseColor = Color::BlueViolet;
    }
};