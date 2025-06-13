#pragma once

/// <summary>
/// Defines the types of monsters that can exist in the game.
/// This enum is used for parsing monster spawn data and creating monster instances.
/// </summary>
enum class MonsterType {
    Mushroom, // Represents a Mushroom monster
    Snail,    // Represents a Snail monster
    // Add other monster types here as you create them, e.g.:
    // Goblin,
    // Dragon,
    UNKNOWN   // Default or unrecognized monster type
};