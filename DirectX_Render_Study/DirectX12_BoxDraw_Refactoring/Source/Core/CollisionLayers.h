#pragma once
#include <cstdint>

namespace CollisionLayer
{
    constexpr uint32_t None         = 0;
    constexpr uint32_t Default      = 1 << 0;
    constexpr uint32_t Player       = 1 << 1;
    constexpr uint32_t Enemy        = 1 << 2;
    constexpr uint32_t PlayerBullet = 1 << 3;
    constexpr uint32_t EnemyBullet  = 1 << 4;
    constexpr uint32_t Terrain      = 1 << 5;
    constexpr uint32_t Obstacle     = 1 << 6;
    constexpr uint32_t Trigger      = 1 << 7;
    constexpr uint32_t All          = 0xFFFFFFFF;

    // Helper to get display name for ImGui
    inline const char* GetLayerName(uint32_t layer)
    {
        switch (layer)
        {
        case Default:      return "Default";
        case Player:       return "Player";
        case Enemy:        return "Enemy";
        case PlayerBullet: return "PlayerBullet";
        case EnemyBullet:  return "EnemyBullet";
        case Terrain:      return "Terrain";
        case Obstacle:     return "Obstacle";
        case Trigger:      return "Trigger";
        default:           return "Custom";
        }
    }
}
