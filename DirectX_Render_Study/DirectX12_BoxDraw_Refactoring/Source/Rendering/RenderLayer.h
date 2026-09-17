#pragma once

enum class RenderLayer
{
    Opaque = 0,       // 3D Opaque meshes (Z-Write ON, Z-Test ON)
    Transparent = 1,  // 3D Transparent / Billboards / Effects (Z-Write OFF, Z-Test ON, Alpha blend)
    UI = 2,           // 2D Sprites, Fonts, HUD (Z-Test OFF, Frontmost)
    COUNT = 3
};

inline const char* GetRenderLayerName(RenderLayer layer)
{
    switch (layer)
    {
    case RenderLayer::Opaque:      return "Opaque";
    case RenderLayer::Transparent: return "Transparent";
    case RenderLayer::UI:          return "UI";
    default:                       return "Unknown";
    }
}
