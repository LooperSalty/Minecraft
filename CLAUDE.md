# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Projet

**VoxelForge** — Clone fidèle de Minecraft Java Edition 1.8.9 en singleplayer. Exécutable Windows standalone (.exe). Gameplay complet du spawn au kill de l'Ender Dragon.

Le PRD (`prd.md`) est la source de vérité pour toutes les spécifications gameplay, constantes, registres de blocs/items/mobs, et recettes.

## Stack technique

- **Langage** : C++17
- **Rendu** : OpenGL 3.3 Core Profile
- **Fenêtrage/Input** : GLFW 3.x
- **Textures** : stb_image.h (header-only)
- **Audio** : OpenAL + stb_vorbis (.ogg)
- **Maths** : GLM (header-only)
- **Texte** : FreeType 2
- **Compression** : zlib
- **Build** : CMake
- **Packaging** : CPack / NSIS

## Build

```bash
# Configuration
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Compilation
cmake --build build --config Release

# Exécuter
./build/Release/VoxelForge.exe
```

Les dépendances tierces sont dans `third_party/` (GLFW, GLAD, GLM, stb, FreeType, OpenAL-Soft).

## Architecture

### Boucle principale

Fixed timestep 20 TPS (50ms) pour la logique, rendu illimité avec interpolation. Voir `prd.md` section 2.3.

### Modules (`src/`)

| Module | Responsabilité |
|--------|---------------|
| `core/` | Engine, fenêtre GLFW, input, timer, resources, config |
| `render/` | Pipeline rendu (chunks, entités, ciel, UI, particules, eau), caméra FPS, shaders, texture atlas, frustum culling |
| `world/` | Monde (chunks 16x256x16), génération procédurale (Perlin noise), biomes, grottes, minerais, éclairage (skylight + blocklight BFS), météo, explosions |
| `entity/` | Entités (base), mobs (passifs/hostiles/boss), projectiles, items au sol, véhicules, IA (A* pathfinding, goals/tasks) |
| `player/` | Contrôleur mouvement, inventaire 36 slots, faim, XP, modes de jeu |
| `item/` | Items (base, outils, armure, nourriture, potions), ItemStack, registre |
| `crafting/` | Crafting (shaped/shapeless), smelting, brewing |
| `enchanting/` | Enchantements, enclume |
| `redstone/` | Simulation redstone (poudre, torche, repeater, comparator, piston, hopper, etc.) |
| `gui/` | Écrans (menu, inventaire, craft, fourneau, enchanting, HUD, debug F3) |
| `command/` | Commandes chat (/tp, /give, /gamemode, /time, /weather, etc.) |
| `save/` | Sérialisation monde (chunks, joueur), format NBT-like |
| `util/` | Noise, PRNG, AABB, raycasting, maths, ThreadPool, logger |

### Assets (`assets/`)

- `textures/` : blocs 16x16, items, entités, GUI, particules, skybox
- `sounds/` : blocs, mobs, ambiance, musique, UI (format .ogg)
- `shaders/` : GLSL (block, entity, sky, particle, ui, water)
- `data/` : JSON (recipes, loot_tables, biomes) + schémas structures

### Rendu

6 passes dans l'ordre : Sky → Blocs opaques → Entités → Blocs transparents → Particules → UI. Un seul atlas de textures pour tous les blocs. Greedy meshing pour réduire les draw calls. Frustum culling par chunk.

### Chunks

Données : `uint16_t[16*256*16]` bloc IDs + 4 bits metadata + 4 bits skylight + 4 bits blocklight. États : UNLOADED → GENERATING → GENERATED → MESHING → READY → DIRTY. Génération async via ThreadPool (4 threads), chargement en spirale depuis le joueur.

## Constantes critiques

```
TICKS_PER_SECOND = 20       SEA_LEVEL = 63
CHUNK_SIZE = 16x256x16      MAX_LIGHT_LEVEL = 15
PLAYER_REACH = 5.0          MAX_HEALTH = 20
PLAYER_HEIGHT = 1.8         MAX_HUNGER = 20
GRAVITY = 0.08 blocs/tick²  MAX_STACK_SIZE = 64
WALK_SPEED = 4.317 blocs/s  SPRINT_SPEED = 5.612 blocs/s
```

## Contraintes de version

Cible Minecraft 1.8.9 exclusivement. Éléments **exclus** car ajoutés après 1.8 :
- Cooldown d'attaque (1.9), auto-jump, shields, elytra, shulkers
- Observer, sweet berry bushes, igloos, kelp, recipe book
- Waterlogged blocks, stripped logs, composters, barrels
- Tout contenu post-1.8.9

## Phases d'implémentation

Le PRD définit 13 phases (0-12). Voir `prd.md` section 28 pour la roadmap complète. Résumé :
- **Phase 0** : Engine core, fenêtre, caméra, chunk rendu
- **Phase 1** : Monde infini, terrain procédural, biomes, caves
- **Phase 2** : Physique joueur, block breaking/placing, éclairage, eau
- **Phase 3** : Inventaire, crafting, smelting, faim, santé
- **Phase 4** : Mobs (tous), IA, pathfinding, combat, breeding
- **Phase 5** : Redstone complète, pistons, TNT, rails
- **Phase 6** : Enchantement, brewing, beacon
- **Phase 7** : Structures (villages, temples, strongholds, ocean monuments)
- **Phase 8** : Nether complet + Wither
- **Phase 9** : End + Ender Dragon + poème de fin
- **Phase 10-12** : Audio, UI, sauvegarde, packaging .exe

## Critères de performance

| Métrique | Cible |
|----------|-------|
| FPS | >= 60 (render distance 8 chunks) |
| Lancement | < 5 secondes |
| Taille exe+assets | < 500 MB |
| RAM | < 2 GB |
