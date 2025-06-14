# MapleStory S

## Game Introduction

A side-scrolling action game inspired by MapleStory!  
Developed in C++ using the Allegro 5 game framework, this game lets players control a character to move, attack, and unleash special skills while fending off waves of incoming monsters.  

The ultimate goal is to defeat the Boss to win the game. If the player's HP drops to zero, the game is over.


## Controls

- **LEFT / RIGHT**: Move the character left/right
- **DOWN**: Prone
- **ALT**: Jump
- **DOWN + ALT**: Drop through platforms
- **LEFT CTRL**: Basic attack
- **C**: Special skill
- **H**: Use Red Potions / coins to restore HP
- **M**: Use Blue Potions to restore MP
- **Q**: Save the game 
- **SPACE**: Switch weapons

## Pick-up Items

- **Red Potion**: Restores HP. Earned by defeating monsters or purchased with coins.
- **Blue Potion**: Restores MP. Earned by defeating monsters or purchased with coins.
- **Coins**: The in-game currency. Earned by defeating monsters and used to purchase potions.

## Features

- **[Easy]**
  - Player UI (HP, MP, EXP, Level indicator)
  - Monster UI (HP)
  - Item counters UI (coins, red potions, blue potions)
  - Sound effects

- **[Medium]**
  - Item animations
  - Saving system
  - Multi-weapon attack logic & animations (unarmed, sword, gun)
  - Key controls

- **[Hard]**
  - **Player System**
    - Movement logic & animation
    - Attack logic & animation
    - Level-up mechanism
    - Special skill implementation
    - HP / MP restoration
    - Item counter
  - **Monster System**
    - Movement logic
    - Animation
    - Chasing behavior
    - Procedurally generated monster AI
    - Collision detection
  - **Map System**
    - Map rendering
    - Mini-map (real-time player tracking)
    - Teleport UI
    - Teleport collision detection
    - Scene switching
  - **Boss attack system**
    - Triangle orbiting projectile pattern

## Individual Contribution

- **109000201 陳彤 (33.3%)**: Player UI, sound effects, weapon logic & animations, Key controls, player System
- **109062205 陳紗寧 (33.3%)**: Item counters UI, saving system, key controls, map system  
- **109062234 李傳中 (33.3%)**: Monster UI, item animations, monster system, boss attack system  
