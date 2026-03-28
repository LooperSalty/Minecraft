# PRD — VoxelForge : Clone Minecraft 1.8.9 Complet

## Document de Référence Produit (PRD) — Version 1.0

**Nom de code :** VoxelForge
**Cible :** Minecraft Java Edition 1.8.9 — Reproduction fidèle
**Livrable :** Exécutable Windows (.exe) standalone
**Auteur :** Paul
**Date :** Mars 2026

---

## TABLE DES MATIÈRES

1. [Vision & Objectifs](#1-vision--objectifs)
2. [Stack Technique & Architecture](#2-stack-technique--architecture)
3. [Phase 0 — Engine Core & Rendu Voxel](#3-phase-0--engine-core--rendu-voxel)
4. [Phase 1 — Monde & Génération Procédurale](#4-phase-1--monde--génération-procédurale)
5. [Phase 2 — Système de Blocs Complet](#5-phase-2--système-de-blocs-complet)
6. [Phase 3 — Joueur, Physique & Survie](#6-phase-3--joueur-physique--survie)
7. [Phase 4 — Système d'Inventaire & Crafting](#7-phase-4--système-dinventaire--crafting)
8. [Phase 5 — Mobs & IA](#8-phase-5--mobs--ia)
9. [Phase 6 — Combat (Style 1.8)](#9-phase-6--combat-style-18)
10. [Phase 7 — Redstone & Mécanismes](#10-phase-7--redstone--mécanismes)
11. [Phase 8 — Enchantement, Brewing & Anvil](#11-phase-8--enchantement-brewing--anvil)
12. [Phase 9 — Structures & Villages](#12-phase-9--structures--villages)
13. [Phase 10 — Le Nether](#13-phase-10--le-nether)
14. [Phase 11 — L'End & Dragon](#14-phase-11--lend--dragon)
15. [Phase 12 — Audio, Particules & Polish](#15-phase-12--audio-particules--polish)
16. [Phase 13 — UI/UX & Menus](#16-phase-13--uiux--menus)
17. [Phase 14 — Sauvegarde & Packaging](#17-phase-14--sauvegarde--packaging)
18. [Registre Complet des Blocs](#18-registre-complet-des-blocs)
19. [Registre Complet des Items](#19-registre-complet-des-items)
20. [Registre Complet des Recettes de Craft](#20-registre-complet-des-recettes-de-craft)
21. [Registre Complet des Mobs](#21-registre-complet-des-mobs)
22. [Registre Complet des Biomes](#22-registre-complet-des-biomes)
23. [Registre Complet des Enchantements](#23-registre-complet-des-enchantements)
24. [Registre Complet des Potions](#24-registre-complet-des-potions)
25. [Registre Complet des Structures](#25-registre-complet-des-structures)
26. [Spécifications de la Génération de Monde](#26-spécifications-de-la-génération-de-monde)
27. [Spécifications Techniques du Rendu](#27-spécifications-techniques-du-rendu)
28. [Roadmap & Milestones](#28-roadmap--milestones)

---

## 1. VISION & OBJECTIFS

### 1.1 Vision

Recréer une expérience Minecraft 1.8.9 fidèle et complète, jouable en singleplayer, compilée en un exécutable Windows natif. Le jeu doit couvrir l'intégralité du gameplay loop : du premier arbre cassé jusqu'au kill de l'Ender Dragon et l'affichage du poème de fin.

### 1.2 Scope Définitif — Ce qui est INCLUS

- Monde infini en 3 dimensions (Overworld, Nether, End)
- Tous les blocs de la 1.8.9 (~200 types)
- Tous les items de la 1.8.9 (~400 items)
- Crafting complet (grille 2x2 et 3x3) avec toutes les recettes
- Smelting (fourneau) avec tous les combustibles et recettes
- Système de mobs complet : passifs, hostiles, neutres, boss
- IA des mobs fidèle au comportement 1.8
- Combat style 1.8 (PAS de cooldown d'attaque, spam-click)
- Système de faim, santé, armure, XP
- Enchantement (table + enclume + livre)
- Brewing (potions complètes)
- Redstone fonctionnelle (poudre, torche, repeater, comparator, piston, etc.)
- Structures générées (villages, temples, donjons, forteresses, etc.)
- Cycle jour/nuit avec mobs nocturnes
- Météo (pluie, orage, neige)
- Biomes complets de la 1.8
- Portail Nether + génération Nether complète
- Portail End + combat Ender Dragon + poème de fin
- Système de sauvegarde/chargement
- Audio (musique d'ambiance + effets sonores)
- Interface utilisateur complète

### 1.3 Scope — Ce qui est EXCLU (pour alléger)

- Multijoueur / réseau
- Skins personnalisés (skin par défaut Steve uniquement)
- Tableau d'affichage (scoreboard)
- Blocs de commande
- Mode Spectateur
- Capes et cosmétiques
- Launcher (le .exe lance directement le jeu)
- Realms / Serveurs
- Mods API
- Succès/Achievements (optionnel, basse priorité)

### 1.4 Critères de Succès

| Critère | Cible |
|---------|-------|
| FPS moyen | ≥ 60 FPS (render distance 8 chunks) |
| Temps de lancement | < 5 secondes |
| Taille de l'exe + assets | < 500 MB |
| RAM usage | < 2 GB |
| Monde jouable | Infini (chargement par chunks) |
| Gameplay complet | Du spawn au kill dragon |

---

## 2. STACK TECHNIQUE & ARCHITECTURE

### 2.1 Choix Technologique

| Composant | Technologie | Justification |
|-----------|-------------|---------------|
| **Langage** | C++ 17 | Performance native, contrôle mémoire, standard industrie jeux vidéo |
| **Rendu** | OpenGL 3.3+ Core Profile | Compatible Windows/Linux, suffisant pour voxel, bien documenté |
| **Fenêtrage** | GLFW 3.x | Léger, cross-platform, gestion input/fenêtre |
| **Chargement textures** | stb_image.h | Header-only, simple, supporte PNG |
| **Audio** | OpenAL + stb_vorbis | Audio 3D positionnel, format .ogg comme Minecraft |
| **Maths** | GLM (OpenGL Mathematics) | Header-only, opérations matrices/vecteurs |
| **UI text** | FreeType 2 | Rendu de texte bitmap, qualité |
| **Compression** | zlib | Compression des chunks sauvegardés |
| **Sérialisation** | Format binaire custom ou NBT-like | Sauvegarde monde compatible |
| **Build system** | CMake | Standard C++, génération .exe facile |
| **Packaging** | CPack / NSIS | Création installer ou portable .exe |

### 2.2 Architecture Globale

```
VoxelForge/
├── CMakeLists.txt
├── assets/
│   ├── textures/
│   │   ├── blocks/          # Textures 16x16 de chaque bloc
│   │   ├── items/           # Textures items
│   │   ├── entities/        # Textures mobs (modèles)
│   │   ├── gui/             # Éléments UI
│   │   ├── particles/       # Sprites particules
│   │   └── environment/     # Skybox, soleil, lune
│   ├── sounds/
│   │   ├── blocks/          # Sons de blocs
│   │   ├── mobs/            # Sons mobs
│   │   ├── ambient/         # Ambiance
│   │   ├── music/           # Musique (C418 style — créer originale ou libre)
│   │   └── ui/              # Sons interface
│   ├── shaders/
│   │   ├── block.vert
│   │   ├── block.frag
│   │   ├── entity.vert
│   │   ├── entity.frag
│   │   ├── sky.vert
│   │   ├── sky.frag
│   │   ├── particle.vert
│   │   ├── particle.frag
│   │   ├── ui.vert
│   │   ├── ui.frag
│   │   └── water.frag       # Shader eau spécial
│   └── data/
│       ├── recipes.json      # Toutes les recettes
│       ├── loot_tables.json  # Loot tables mobs/coffres
│       ├── biomes.json       # Config biomes
│       └── structures/       # Schémas structures (NBT-like)
├── src/
│   ├── main.cpp
│   ├── core/
│   │   ├── Engine.h/cpp           # Boucle principale, init, shutdown
│   │   ├── Window.h/cpp           # Abstraction GLFW
│   │   ├── InputManager.h/cpp     # Clavier, souris, bindings
│   │   ├── Timer.h/cpp            # Delta time, tick rate
│   │   ├── ResourceManager.h/cpp  # Chargement assets centralisé
│   │   └── Config.h/cpp           # Settings joueur
│   ├── render/
│   │   ├── Renderer.h/cpp         # Pipeline rendu principal
│   │   ├── Camera.h/cpp           # Caméra FPS
│   │   ├── Shader.h/cpp           # Compilation/gestion shaders
│   │   ├── TextureAtlas.h/cpp     # Atlas de textures blocs
│   │   ├── ChunkMesher.h/cpp      # Génération mesh d'un chunk
│   │   ├── ChunkRenderer.h/cpp    # Rendu batch de chunks
│   │   ├── SkyRenderer.h/cpp      # Ciel, soleil, lune, étoiles
│   │   ├── EntityRenderer.h/cpp   # Rendu des entités/mobs
│   │   ├── ParticleRenderer.h/cpp # Système de particules
│   │   ├── UIRenderer.h/cpp       # Rendu 2D interface
│   │   ├── TextRenderer.h/cpp     # Rendu texte FreeType
│   │   ├── WaterRenderer.h/cpp    # Rendu eau semi-transparent
│   │   └── Frustum.h/cpp          # Frustum culling
│   ├── world/
│   │   ├── World.h/cpp            # Monde principal, tick, gestion chunks
│   │   ├── Chunk.h/cpp            # Données d'un chunk 16x256x16
│   │   ├── ChunkManager.h/cpp     # Chargement/déchargement async
│   │   ├── Block.h/cpp            # Définition et registre de blocs
│   │   ├── BlockState.h/cpp       # États de blocs (orientation, etc.)
│   │   ├── WorldGenerator.h/cpp   # Générateur principal
│   │   ├── BiomeGenerator.h/cpp   # Carte de biomes
│   │   ├── TerrainGenerator.h/cpp # Heightmap + caves
│   │   ├── StructureGenerator.h/cpp # Arbres, structures
│   │   ├── OreGenerator.h/cpp     # Placement minerais
│   │   ├── Dimension.h/cpp        # Overworld, Nether, End
│   │   ├── Lighting.h/cpp         # Propagation lumière (block + sky)
│   │   ├── Weather.h/cpp          # Pluie, orage, neige
│   │   └── Explosion.h/cpp        # Mécanique d'explosion (TNT, creeper)
│   ├── entity/
│   │   ├── Entity.h/cpp           # Classe de base entité
│   │   ├── Player.h/cpp           # Joueur
│   │   ├── MobBase.h/cpp          # Base mob avec IA
│   │   ├── passive/               # Mobs passifs
│   │   │   ├── Cow.h/cpp
│   │   │   ├── Pig.h/cpp
│   │   │   ├── Sheep.h/cpp
│   │   │   ├── Chicken.h/cpp
│   │   │   ├── Squid.h/cpp
│   │   │   ├── Bat.h/cpp
│   │   │   ├── Horse.h/cpp
│   │   │   ├── Rabbit.h/cpp
│   │   │   ├── Mooshroom.h/cpp
│   │   │   ├── Ocelot.h/cpp
│   │   │   ├── Wolf.h/cpp         # Neutre (passif si non provoqué)
│   │   │   └── Villager.h/cpp
│   │   ├── hostile/               # Mobs hostiles
│   │   │   ├── Zombie.h/cpp
│   │   │   ├── Skeleton.h/cpp
│   │   │   ├── Creeper.h/cpp
│   │   │   ├── Spider.h/cpp
│   │   │   ├── CaveSpider.h/cpp
│   │   │   ├── Enderman.h/cpp
│   │   │   ├── Slime.h/cpp
│   │   │   ├── MagmaCube.h/cpp
│   │   │   ├── Ghast.h/cpp
│   │   │   ├── Blaze.h/cpp
│   │   │   ├── WitherSkeleton.h/cpp
│   │   │   ├── ZombiePigman.h/cpp # Neutre sauf si attaqué
│   │   │   ├── Witch.h/cpp
│   │   │   ├── Guardian.h/cpp
│   │   │   ├── ElderGuardian.h/cpp
│   │   │   ├── Endermite.h/cpp
│   │   │   ├── Silverfish.h/cpp
│   │   │   └── Shulker.h/cpp      # Absent en 1.8, mais confirmé? Non, ajouté 1.9. EXCLU.
│   │   ├── boss/
│   │   │   ├── EnderDragon.h/cpp
│   │   │   └── Wither.h/cpp
│   │   ├── projectile/
│   │   │   ├── Arrow.h/cpp
│   │   │   ├── Snowball.h/cpp
│   │   │   ├── Egg.h/cpp
│   │   │   ├── EnderPearl.h/cpp
│   │   │   ├── Fireball.h/cpp     # Ghast + Blaze
│   │   │   ├── FishingBobber.h/cpp
│   │   │   └── ThrownPotion.h/cpp
│   │   ├── item_entity/
│   │   │   ├── ItemEntity.h/cpp   # Items au sol
│   │   │   ├── XPOrb.h/cpp
│   │   │   └── FallingBlock.h/cpp # Sable, gravier
│   │   ├── vehicle/
│   │   │   ├── Boat.h/cpp
│   │   │   └── Minecart.h/cpp     # + variantes (chest, hopper, TNT, furnace)
│   │   └── ai/
│   │       ├── AIController.h/cpp    # Système IA principal
│   │       ├── Pathfinding.h/cpp     # A* sur grille voxel
│   │       ├── tasks/
│   │       │   ├── WanderTask.h/cpp
│   │       │   ├── FleeTask.h/cpp
│   │       │   ├── AttackTask.h/cpp
│   │       │   ├── FollowTask.h/cpp
│   │       │   ├── LookAtPlayerTask.h/cpp
│   │       │   ├── SwimTask.h/cpp
│   │       │   ├── BreedTask.h/cpp
│   │       │   └── PanicTask.h/cpp
│   │       └── goals/
│   │           ├── MeleeAttackGoal.h/cpp
│   │           ├── RangedAttackGoal.h/cpp
│   │           ├── AvoidEntityGoal.h/cpp
│   │           └── TemptGoal.h/cpp
│   ├── player/
│   │   ├── PlayerController.h/cpp  # Mouvement, physique joueur
│   │   ├── Inventory.h/cpp         # Inventaire 36 slots + armure + craft 2x2
│   │   ├── HungerSystem.h/cpp      # Faim, saturation, régénération
│   │   ├── ExperienceSystem.h/cpp  # XP, niveaux
│   │   └── GameMode.h/cpp          # Survival, Creative, Adventure
│   ├── item/
│   │   ├── Item.h/cpp              # Classe de base item
│   │   ├── ItemStack.h/cpp         # Stack d'items (quantité, durabilité, NBT)
│   │   ├── ItemRegistry.h/cpp      # Registre de tous les items
│   │   ├── ToolItem.h/cpp          # Pioche, hache, pelle, épée, houe
│   │   ├── ArmorItem.h/cpp         # Casque, plastron, jambières, bottes
│   │   ├── FoodItem.h/cpp          # Nourriture (valeurs faim + saturation)
│   │   ├── BowItem.h/cpp           # Arc
│   │   ├── FishingRodItem.h/cpp    # Canne à pêche
│   │   ├── PotionItem.h/cpp        # Potions
│   │   ├── EnchantedBookItem.h/cpp # Livres enchantés
│   │   └── SpecialItems.h/cpp      # Boussole, horloge, carte, etc.
│   ├── crafting/
│   │   ├── CraftingManager.h/cpp   # Moteur de crafting
│   │   ├── Recipe.h/cpp            # Recette shaped + shapeless
│   │   ├── SmeltingManager.h/cpp   # Recettes fourneau
│   │   └── BrewingManager.h/cpp    # Recettes brewing
│   ├── enchanting/
│   │   ├── EnchantmentSystem.h/cpp # Système d'enchantement
│   │   ├── Enchantment.h/cpp       # Définition enchantements
│   │   └── AnvilSystem.h/cpp       # Combinaison/renommage
│   ├── redstone/
│   │   ├── RedstoneEngine.h/cpp    # Simulation redstone
│   │   ├── RedstoneDust.h/cpp      # Poudre, propagation signal
│   │   ├── RedstoneTorch.h/cpp     # Torche (inverseur)
│   │   ├── Repeater.h/cpp          # Repeater (délai, lock)
│   │   ├── Comparator.h/cpp        # Comparator (compare, subtract)
│   │   ├── Piston.h/cpp            # Piston + sticky piston
│   │   ├── Dispenser.h/cpp         # Dispenser + Dropper
│   │   ├── Hopper.h/cpp            # Hopper (transfert items)
│   │   ├── Observer.h/cpp          # Non existant en 1.8, EXCLU
│   │   ├── Lever.h/cpp             # Levier
│   │   ├── Button.h/cpp            # Boutons bois/pierre
│   │   ├── PressurePlate.h/cpp     # Plaques de pression
│   │   ├── Tripwire.h/cpp          # Fil piège
│   │   ├── DaylightSensor.h/cpp    # Capteur lumière
│   │   ├── TrappedChest.h/cpp      # Coffre piégé
│   │   └── NoteBlock.h/cpp         # Note block
│   ├── gui/
│   │   ├── Screen.h/cpp            # Écran de base
│   │   ├── MainMenuScreen.h/cpp    # Menu principal
│   │   ├── PauseScreen.h/cpp       # Menu pause (Echap)
│   │   ├── InventoryScreen.h/cpp   # Inventaire joueur
│   │   ├── CraftingScreen.h/cpp    # Table de craft
│   │   ├── FurnaceScreen.h/cpp     # Fourneau
│   │   ├── ChestScreen.h/cpp       # Coffre (simple + double)
│   │   ├── EnchantingScreen.h/cpp  # Table d'enchantement
│   │   ├── AnvilScreen.h/cpp       # Enclume
│   │   ├── BrewingScreen.h/cpp     # Alambic
│   │   ├── VillagerTradeScreen.h/cpp # Trading villageois
│   │   ├── HorseScreen.h/cpp       # Inventaire cheval
│   │   ├── BeaconScreen.h/cpp      # Beacon
│   │   ├── HopperScreen.h/cpp      # Hopper
│   │   ├── DispenserScreen.h/cpp   # Dispenser/Dropper
│   │   ├── CreativeScreen.h/cpp    # Inventaire créatif
│   │   ├── SettingsScreen.h/cpp    # Options
│   │   ├── WorldSelectScreen.h/cpp # Sélection monde
│   │   ├── CreateWorldScreen.h/cpp # Création monde
│   │   ├── DeathScreen.h/cpp       # Écran de mort
│   │   ├── HUD.h/cpp               # Heads-Up Display in-game
│   │   ├── ChatBox.h/cpp           # Barre de chat (commandes)
│   │   └── DebugOverlay.h/cpp      # F3 debug screen
│   ├── command/
│   │   ├── CommandParser.h/cpp     # Parse commandes /
│   │   ├── GameRuleCommand.h/cpp   # /gamerule
│   │   ├── TeleportCommand.h/cpp   # /tp
│   │   ├── GiveCommand.h/cpp       # /give
│   │   ├── TimeCommand.h/cpp       # /time set
│   │   ├── WeatherCommand.h/cpp    # /weather
│   │   ├── GameModeCommand.h/cpp   # /gamemode
│   │   ├── KillCommand.h/cpp       # /kill
│   │   └── SummonCommand.h/cpp     # /summon
│   ├── save/
│   │   ├── WorldSave.h/cpp         # Sauvegarde/chargement monde
│   │   ├── ChunkSerializer.h/cpp   # Sérialisation chunk binaire
│   │   ├── PlayerData.h/cpp        # Sauvegarde joueur
│   │   └── NBTFormat.h/cpp         # Format NBT simplifié
│   └── util/
│       ├── Noise.h/cpp             # Perlin / Simplex noise
│       ├── Random.h/cpp            # PRNG seedé
│       ├── AABB.h/cpp              # Axis-Aligned Bounding Box
│       ├── Ray.h/cpp               # Raycasting
│       ├── MathUtils.h/cpp         # Utilitaires maths
│       ├── ThreadPool.h/cpp        # Pool de threads (génération async)
│       └── Logger.h/cpp            # Logging
└── third_party/
    ├── glfw/
    ├── glad/                       # Loader OpenGL
    ├── glm/
    ├── stb/
    ├── freetype/
    └── openal-soft/
```

### 2.3 Boucle de Jeu Principale

```
TARGET: 20 TPS (ticks par seconde) pour la logique, rendu illimité (vsync optionnel)

while (running) {
    // INPUT
    pollEvents();
    processInput();
    
    // LOGIC (fixed timestep 50ms = 20 TPS)
    while (accumulator >= TICK_RATE) {
        tick();          // Monde, mobs, redstone, météo, lumière
        accumulator -= TICK_RATE;
    }
    
    // RENDER (aussi vite que possible)
    float interpolation = accumulator / TICK_RATE;
    render(interpolation);  // Interpolation pour smooth rendering
    
    swapBuffers();
}
```

### 2.4 Système de Ticks

| Système | Fréquence | Description |
|---------|-----------|-------------|
| Game tick | 20/s (50ms) | Tick principal, tout le gameplay |
| Block tick | Random tick (3 par chunk/tick) | Croissance cultures, spread herbe |
| Redstone | Instantané + 1-4 tick delays | Propagation signal redstone |
| Mob AI | Toutes les 1-2s | Réévaluation IA mobs |
| Lighting | Par propagation BFS | Mise à jour lumière |
| Weather | ~20min cycles | Changement météo |
| Day/Night | 24000 ticks = 20 min | Cycle complet jour/nuit |

---

## 3. PHASE 0 — ENGINE CORE & RENDU VOXEL

### 3.1 Fenêtre & Contexte OpenGL

- Créer une fenêtre GLFW 854x480 par défaut (redimensionnable, fullscreen toggle F11)
- Contexte OpenGL 3.3 Core Profile
- VSync configurable
- Capture souris (mode FPS)
- Gestion input : clavier AZERTY/QWERTY, souris, scroll
- Delta time calculé avec `glfwGetTime()`

### 3.2 Caméra FPS

```
Specs caméra :
- Position : vec3 (x, y, z)
- Yaw : rotation horizontale (souris X)
- Pitch : rotation verticale (souris Y), clampé [-90°, +90°]
- FOV : 70° par défaut (configurable 30-110)
- Near plane : 0.05
- Far plane : 1000.0
- Sensibilité souris : configurable
- Matrice View : lookAt(position, position + front, up)
- Matrice Projection : perspective(fov, aspect, near, far)
```

### 3.3 Système de Chunks

```
Chunk :
- Taille : 16 x 256 x 16 blocs (X, Y, Z)
- Stockage : tableau 1D de block IDs (uint16_t[16*256*16] = 131,072 blocs)
- Section : 16x16x16 (16 sections par chunk, seules les non-vides sont stockées en mémoire)
- Coordonnées : (chunkX, chunkZ) = (blockX >> 4, blockZ >> 4)
- Metadata par bloc : 4 bits (orientation, variante, etc.)
- Skylight : 4 bits par bloc
- Blocklight : 4 bits par bloc
```

### 3.4 Chunk Loading

```
Render distance : configurable 2-16 chunks (défaut 8)
Loading pattern : spirale depuis la position joueur
Priorité : chunks proches chargés en premier

Pipeline :
1. ChunkManager détermine quels chunks doivent être chargés
2. File d'attente de génération (ThreadPool, 4 threads)
3. Génération terrain → placement structures → éclairage
4. Mesh building (greedy meshing ou culled face)
5. Upload GPU (VBO/VAO)
6. Chunks hors range → sauvegarde disk → libérer mémoire

Chunk states :
- UNLOADED : pas en mémoire
- GENERATING : en cours de génération (async)
- GENERATED : données prêtes, pas de mesh
- MESHING : mesh en construction
- READY : mesh uploadé GPU, rendu possible
- DIRTY : bloc modifié, re-mesh nécessaire
```

### 3.5 Mesh Building — Greedy Meshing

```
Algorithme pour chaque chunk :
1. Pour chaque face de chaque bloc visible :
   a. Vérifier si le voisin dans cette direction est opaque
   b. Si le voisin est transparent/air → cette face est visible
   c. Si le voisin est dans un chunk adjacent → requête cross-chunk
2. Greedy meshing : fusionner les quads adjacents de même texture
3. Vertex format par face :
   - Position (3 floats, compressé en 1 uint32 possible)
   - UV texture atlas (2 floats)
   - Normal (1 byte, 6 directions)
   - Light level (1 byte, combiné sky+block)
   - AO (ambient occlusion, 4 niveaux par vertex)

Optimisations :
- Ne pas mesh les sections (16³) entièrement vides ou entièrement pleines
- Réutiliser les buffers d'upload
- Batch tous les chunks en un minimum de draw calls
- Séparer les passes : opaque d'abord, transparent ensuite (eau, verre)
```

### 3.6 Texture Atlas

```
Atlas unique 256x256 (ou 512x512) contenant toutes les textures de blocs 16x16
- Chaque texture = 16x16 pixels
- Position UV calculée : (texX * 16 / atlasWidth, texY * 16 / atlasHeight)
- Un seul bind de texture pour TOUS les blocs → un seul draw call
- Filtrage : GL_NEAREST (pas de bilinéaire, aspect pixelisé Minecraft)
- Mipmapping : GL_NEAREST_MIPMAP_LINEAR pour éviter le moiré à distance
```

### 3.7 Frustum Culling

```
Algorithme :
1. Extraire les 6 plans du frustum depuis la matrice ViewProjection
2. Pour chaque chunk, tester son AABB contre les 6 plans
3. Si entièrement hors frustum → skip le draw call
4. Résultat attendu : ~50-70% des chunks non rendus
```

### 3.8 Skybox & Cycle Jour/Nuit

```
Cycle complet : 24000 ticks = 20 minutes réelles
- 0-12000 : Jour (soleil monte puis descend)
- 12000-13000 : Coucher de soleil
- 13000-23000 : Nuit (lune)
- 23000-24000 : Lever de soleil

Rendu ciel :
- Gradient de couleur interpolé selon l'heure (bleu jour → orange coucher → noir nuit)
- Soleil : quad texturé, rotation autour de l'axe Est-Ouest
- Lune : quad texturé (8 phases de lune), opposé au soleil
- Étoiles : points rendus la nuit (rotation lente)
- Brouillard : couleur et distance dépendent de l'heure et la météo
```

### 3.9 Éclairage

```
Deux systèmes combinés :

1. Sky Light (lumière du ciel) :
   - Valeur 0-15 par bloc
   - Commence à 15 au-dessus du bloc solide le plus haut
   - Se propage vers le bas sans perte (colonne directe)
   - Se propage latéralement avec -1 par bloc
   - Affecté par l'heure : valeur effective = skylight * timeFactor

2. Block Light (lumière artificielle) :
   - Valeur 0-15 par bloc
   - Sources : torche (14), glowstone (15), feu (15), redstone torch (7), etc.
   - Propagation BFS : -1 par bloc dans les 6 directions
   - Blocs opaques bloquent, blocs transparents transmettent

Lumière finale par vertex = max(blockLight, skyLight * timeMultiplier) / 15.0
Appliquer en vertex color dans le shader

Ambient Occlusion (AO) :
- Par vertex de chaque face
- 4 niveaux (0-3) basés sur les blocs adjacents au coin
- Assombrit les coins/crevasses
- Flip le quad si AO asymétrique (anti-artifact)
```

---

## 4. PHASE 1 — MONDE & GÉNÉRATION PROCÉDURALE

### 4.1 Seed & PRNG

```
- Seed : int64, entré par le joueur ou aléatoire
- PRNG : basé sur le seed, utilisé pour TOUT (terrain, structures, loot)
- Même seed = même monde, toujours
- Chaque chunk a un sous-seed dérivé : hash(worldSeed, chunkX, chunkZ)
```

### 4.2 Carte de Biomes

```
Système basé sur 2 paramètres (fidèle MC 1.8) :
- Température : Perlin noise, octaves=4, scale=1/256
- Humidité : Perlin noise, octaves=4, scale=1/256

Grille de sélection biome :
          | Aride      | Normal     | Humide     | Très Humide
----------|------------|------------|------------|-------------
Glacial   | Ice Plains | Ice Plains | Cold Taiga | Cold Taiga
Froid     | Extreme H. | Taiga      | Mega Taiga | Mega Taiga  
Tempéré   | Plains     | Forest     | Birch F.   | Roofed F.
Chaud     | Desert     | Savanna    | Jungle     | Jungle
Très chaud| Mesa       | Mesa       | Mesa       | Mesa

Biomes spéciaux :
- Ocean : altitude < 63 sur de grandes zones
- Deep Ocean : altitude < 40
- River : bruit fractal connectant des biomes
- Mushroom Island : rare, îles isolées en ocean
- Swamp : zones basses humides
- Beach : transition terre/ocean
- Stone Beach : plage abrupte en extreme hills
```

### 4.3 Génération de Terrain — Heightmap

```
Algorithme par couches :

1. Base Height (Perlin 2D) :
   - Octaves : 8
   - Persistence : 0.5
   - Scale de base : 1/256
   - Amplitude : normalisée [0, 1]
   - Mapping : height = 64 + (noise * 40)  → terrain entre Y=64 et Y=104

2. Variation par biome :
   - Plains : base ± 3
   - Extreme Hills : base + noise * 30 (pics montagneux)
   - Ocean : base - 20 à -40
   - Desert : base ± 2 (plat)
   - Jungle : base ± 5

3. River Carving :
   - Bruit Perlin 2D, seuil très étroit (|noise| < 0.02)
   - Creuse le terrain à Y=62 le long du tracé

4. Remplissage :
   - Y=0 : Bedrock (mélange aléatoire Y 0-4)
   - Y=1 à surface-4 : Stone
   - Y=surface-4 à surface-1 : Dirt (ou Sand pour desert/beach)
   - Y=surface : Grass Block (ou Sand, Mycelium, Podzol selon biome)
   - Y ≤ 62 si air : Water (niveau mers)
```

### 4.4 Grottes (Caves)

```
Algorithme : Perlin Worm / 3D Noise carving

Méthode Perlin Worm :
1. Pour chaque chunk, générer N "worms" (tunnels)
2. Chaque worm : position de départ aléatoire dans le chunk
3. Itérer 100-200 pas :
   a. Direction = Perlin noise 3D évalué à la position
   b. Avancer d'un pas dans cette direction
   c. Rayon du tunnel = 1.5 à 3.5 blocs (varie avec noise)
   d. Creuser une sphère de ce rayon
   e. Ne pas creuser sous Y=10 (pour garder la lave)
4. Les worms peuvent traverser les frontières de chunks

Caves d'eau : Si le tunnel passe sous Y=62, remplir d'eau
Ravines : Worms très grands (rayon 5-8) mais très rares, orientation verticale

Bruit 3D complémentaire :
- Perlin 3D (octaves=3, scale=1/32)
- Si noise > seuil (0.7) → creuser (grands espaces caverneux)
- Limité entre Y=5 et Y=60
```

### 4.5 Minerais

```
Chaque minerai a : bloc, taille du filon, fréquence par chunk, plage Y

| Minerai | Bloc | Taille filon | Filons/chunk | Y min | Y max |
|---------|------|-------------|--------------|-------|-------|
| Charbon | coal_ore | 17 | 20 | 0 | 128 |
| Fer | iron_ore | 9 | 20 | 0 | 64 |
| Or | gold_ore | 9 | 2 | 0 | 32 |
| Redstone | redstone_ore | 8 | 8 | 0 | 16 |
| Diamant | diamond_ore | 8 | 1 | 0 | 16 |
| Lapis | lapis_ore | 7 | 1 | 0 | 32 (pic à 16) |
| Émeraude | emerald_ore | 1 | 1-2 | 4 | 32 | (Extreme Hills only)
| Gravier | gravel | 33 | 10 | 0 | 256 |
| Terre | dirt | 33 | 10 | 0 | 256 |
| Granite | granite | 33 | 10 | 0 | 80 |
| Diorite | diorite | 33 | 10 | 0 | 80 |
| Andesite | andesite | 33 | 10 | 0 | 80 |

Algorithme de placement :
1. Pour chaque filon, position aléatoire dans la plage
2. Forme : ellipsoïde allongé (longueur = taille, rayon ~2)
3. Remplacer la stone par le minerai
```

### 4.6 Arbres

```
Types d'arbres par biome :

| Biome | Type d'arbre | Fréquence |
|-------|-------------|-----------|
| Plains | Oak (petit) | 1 par chunk |
| Forest | Oak + Birch | 7-10 par chunk |
| Birch Forest | Birch uniquement | 7-10 |
| Taiga | Spruce | 7-10 |
| Mega Taiga | Mega Spruce (2x2) | 3-5 |
| Jungle | Jungle (géant 2x2 + buissons) | 50+ (dense) |
| Roofed Forest | Dark Oak (2x2) | Dense |
| Savanna | Acacia | 2-3 |
| Swamp | Oak + lianes | 3-5 |

Algorithme Oak basique :
1. Tronc : 4-6 blocs de haut (oak_log)
2. Feuilles : blob 5x5x3 au sommet avec coins arrondis aléatoires (oak_leaves)
3. Vérification : ne pas placer si collision avec un autre arbre ou structure

Algorithme Birch :
- Comme Oak mais tronc toujours 5-7 de haut, jamais de branches

Algorithme Spruce :
- Tronc : 6-9 blocs
- Feuilles : forme conique (large en bas, pointu en haut)

Algorithme Jungle Large (2x2) :
- Tronc 2x2 : 15-25 blocs de haut
- Branches latérales
- Lianes (vines) sur les côtés
- Feuilles : couronnes sphériques aux branches

Algorithme Dark Oak (2x2) :
- Tronc 2x2 : 4-8 blocs
- Branches horizontales
- Large canopée
```

### 4.7 Décoration par Biome

```
Après terrain + arbres, ajouter la décoration :

Plains :
- Herbe haute (tall_grass) : densité élevée
- Fleurs (dandelion, poppy) : clusters aléatoires
- Villages (voir section structures)

Forest :
- Herbe + fleurs
- Champignons sous les arbres (rare)

Desert :
- Cactus : hauteur 1-3 (espacement minimum 1 bloc)
- Dead bush sur le sable
- Temples du désert (structure)
- Villages (variante désert)
- Puits de désert

Jungle :
- Herbe haute très dense
- Melon (sauvage, rare)
- Cocoa pods sur les troncs
- Temples de jungle (structure)
- Lianes omniprésentes

Swamp :
- Nénuphars sur l'eau
- Lianes sur les arbres
- Witch hut (structure)
- Slimes spawn la nuit

Taiga :
- Sweet berry bushes : Non, c'est 1.14. EXCLU.
- Ferns + herbe
- Loups (spawn)
- Igloos : Non, c'est 1.9. EXCLU.

Mesa :
- Terracotta layers (stained, bandes colorées)
- Pas de végétation
- Gold ore jusqu'à Y=80 (spécial mesa)
- Mineshafts en surface

Mushroom Island :
- Mycelium au lieu de grass
- Giant mushrooms (rouge + marron)
- Mooshrooms (spawn exclusif)
- AUCUN mob hostile ne spawn naturellement

Ocean :
- Gravel et sable au fond
- Ocean monuments (structure, deep ocean)
- Kelp : Non, c'est 1.13. Juste du gravel/sable au fond.

Extreme Hills :
- Silverfish stone (monster egg)
- Emerald ore
- Patches de neige au-dessus de Y=95
- Gravier en surface par endroits
```

---

## 5. PHASE 2 — SYSTÈME DE BLOCS COMPLET

### 5.1 Architecture Block

```cpp
struct BlockType {
    uint16_t id;                    // ID unique
    std::string name;               // Nom interne (ex: "oak_log")
    std::string displayName;        // Nom affiché (ex: "Oak Wood")
    
    // Textures (indices dans l'atlas)
    uint16_t texTop, texBottom, texNorth, texSouth, texEast, texWest;
    
    // Propriétés physiques
    float hardness;                 // Temps de base pour miner (secondes)
    float blastResistance;          // Résistance explosion
    ToolType preferredTool;         // NONE, PICKAXE, AXE, SHOVEL, SHEARS
    ToolMaterial minToolTier;       // NONE, WOOD, STONE, IRON, DIAMOND, GOLD
    
    // Propriétés visuelles
    bool isOpaque;                  // Bloque la lumière et le rendu des faces voisines
    bool isTransparent;             // Semi-transparent (eau, verre)
    bool isSolid;                   // Collision physique
    uint8_t lightEmission;          // 0-15, lumière émise
    uint8_t lightFilter;            // Combien de lumière le bloc absorbe
    
    // Propriétés gameplay
    bool hasGravity;                // Sable, gravier
    bool isFlammable;               // Peut brûler
    bool isReplaceable;             // Herbe, neige fine — remplacé en posant un bloc
    bool needsSupport;              // Fleur, torche — tombe si pas de support
    BlockShape shape;               // FULL, HALF_SLAB, STAIR, CROSS, TORCH, etc.
    
    // Loot
    uint16_t dropItem;              // Item droppé quand cassé
    uint8_t dropCount;              // Quantité droppée
    bool needsSilkTouch;            // Drop le bloc lui-même seulement avec Silk Touch
    
    // Interaction
    bool hasInteraction;            // Click droit → ouvre GUI
    bool isContainer;               // Contient un inventaire (coffre, fourneau)
    bool isRedstoneComponent;       // Participe au système redstone
};
```

### 5.2 Block Shapes (Formes non-cubiques)

```
Les blocs ne sont pas tous des cubes 1x1x1. Formes spéciales :

FULL_BLOCK : Cube standard (stone, dirt, wood...)
SLAB : Demi-dalle (top ou bottom)
STAIR : Escalier (4 orientations × 2 positions = 8 états)
FENCE : Poteau + connexions latérales (hitbox 1.5 haut)
FENCE_GATE : Portillon (ouvert/fermé)
WALL : Mur (cobble wall, mossy cobble wall)
DOOR : Porte (2 blocs haut, ouvert/fermé, 4 orientations)
TRAPDOOR : Trappe (ouvert/fermé, top/bottom, 4 orientations)
TORCH : Croix en X inclinée (sol ou mur, 5 positions)
CROSS : Plante en X (fleur, herbe, champignon)
CROP : Plante plate en # (blé, carotte, patate)
PANE : Vitre fine (connecte aux voisins, comme fence)
LADDER : Échelle (plaquée au mur)
SIGN : Panneau (sol + mur)
BED : Lit (2 blocs, tête + pied)
CHEST : Coffre (légèrement plus petit qu'un bloc)
ANVIL : Enclume (forme spéciale)
BREWING_STAND : Alambic (forme spéciale)
ENCHANTING_TABLE : Table (3/4 hauteur)
CAULDRON : Chaudron (creux)
FLOWER_POT : Pot de fleur (petit)
HEAD : Tête (mob head)
BUTTON : Bouton (petit carré sur une face)
LEVER : Levier
PRESSURE_PLATE : Plaque (fine, au sol)
RAIL : Rail (plat ou incliné)
REDSTONE_WIRE : Fil redstone (plat au sol)
REPEATER : Repeater (plat)
COMPARATOR : Comparator (plat)
CACTUS : Cactus (légèrement rétréci, 14/16 pixels)
CAKE : Gâteau (mangeable en 7 parts)
CARPET : Tapis (1/16 épaisseur)
SNOW_LAYER : Couche de neige (1/8 à 1 bloc)
LILY_PAD : Nénuphar (plat sur l'eau)
END_PORTAL_FRAME : Cadre portail End (forme spéciale)
DRAGON_EGG : Œuf de dragon (forme arrondie)
```

### 5.3 Block States

```
Certains blocs ont des états variables stockés en metadata (4 bits) :

LOG : axis (X, Y, Z) + bark_only → 3 bits
LEAVES : decay_distance (0-7) + persistent → 4 bits
SLAB : half (top, bottom) + waterlogged(1.13 non, pas en 1.8) → 1 bit
STAIR : facing (N,S,E,W) + half (top,bottom) + shape (straight,inner_L,inner_R,outer_L,outer_R) → 4+ bits
DOOR : facing + open + hinge + half + powered → 4 bits
TORCH : face (floor, wall_N, wall_S, wall_E, wall_W) → 3 bits
FURNACE : facing + lit → 3 bits
CHEST : facing + type (single, left, right) → 3 bits
PISTON : facing + extended → 4 bits
REPEATER : facing + delay (1-4) + locked → 4 bits
COMPARATOR : facing + mode (compare/subtract) + powered → 4 bits
RAIL : shape (10 variants) → 4 bits
REDSTONE_WIRE : power (0-15) → 4 bits
BED : facing + part (head/foot) + occupied → 3 bits
CROP : growth_stage (0-7) → 3 bits
FARMLAND : moisture (0-7) → 3 bits
WATER/LAVA : level (0-7) + falling → 4 bits
```

---

## 6. PHASE 3 — JOUEUR, PHYSIQUE & SURVIE

### 6.1 Physique du Joueur

```
Hitbox joueur : 0.6 × 1.8 × 0.6 (largeur × hauteur × profondeur)
Hitbox accroupi : 0.6 × 1.65 × 0.6
Position yeux : Y + 1.62 (debout), Y + 1.54 (accroupi)

Mouvement :
- Vitesse marche : 4.317 blocs/s
- Vitesse sprint : 5.612 blocs/s (30% boost)
- Vitesse accroupi : 1.295 blocs/s
- Vitesse vol (creative) : 10.89 blocs/s

Saut :
- Hauteur : 1.252 blocs (suffisant pour 1 bloc)
- Jump boost : +0.5 blocs par niveau
- Auto-jump : NON (pas en 1.8)

Gravité :
- Accélération : -0.08 blocs/tick²
- Drag (air) : velocity *= 0.98 par tick
- Drag (eau) : velocity *= 0.8 par tick
- Vitesse terminale : ~3.92 blocs/tick (≈ 78 blocs/s)
- Damage de chute : (distance - 3) demi-cœurs (au-delà de 3 blocs)

Natation :
- Le joueur coule par défaut
- Appuyer sur espace pour nager vers le haut
- Vitesse horizontale réduite dans l'eau
- Respiration : 15 secondes sous l'eau (300 ticks), puis 1 dégât/seconde

Collision :
- AABB vs AABB contre tous les blocs solides
- Résolution : axe par axe (Y d'abord, puis X/Z)
- Step-up : 0.5 blocs automatique (monter les dalles sans sauter)
- Sneaking empêche de tomber des bords
```

### 6.2 Raycasting (Block Breaking/Placing)

```
Portée : 5 blocs (survival), 5 blocs (creative aussi en 1.8)

Algorithme DDA (Digital Differential Analyzer) :
1. Depuis la position des yeux
2. Direction = vecteur regard normalisé
3. Avancer bloc par bloc le long du rayon
4. Premier bloc non-air touché = cible
5. Retourner : position du bloc + face touchée

Breaking :
- Maintenir clic gauche
- Temps = hardness * toolSpeedMultiplier * enchantmentMultiplier
- Afficher les craquelures (10 stages de texture overlay)
- Drop l'item si outil adéquat (sinon rien pour certains blocs)
- Particules de destruction

Placing :
- Clic droit
- Placer sur la face touchée du bloc ciblé
- Vérifier : pas de collision avec le joueur ou les entités
- Vérifier : le bloc destination est air ou remplaçable
- Son de placement
```

### 6.3 Santé & Dégâts

```
Santé : 20 points (10 cœurs)
Régénération naturelle : 1 HP toutes les 4 secondes si faim ≥ 18

Sources de dégâts :
- Chute : max(0, distance - 3) points
- Attaque mob : variable par mob
- Feu/lave : 1/s (feu), 4/s (lave)
- Noyade : 2/s après 15s sous l'eau
- Suffocation (dans un bloc) : 1/s
- Cactus : 1/contact
- Explosion : variable (distance + exposure)
- Void (Y < 0) : 4/s
- Famine (faim = 0) : 1/s (Easy), 1/s jusqu'à 1HP (Normal), 1/s mort (Hard)
- Wither effect : 1 par seconde
- Poison effect : 1 par 1.25s (ne tue pas, min 1HP)
- Anvil falling : variable

Armure :
- Réduction = min(20, totalArmorPoints) / 25
- Chaque pièce contribue des points d'armure
- Absorption de dégâts : damage * (1 - reduction)

| Pièce | Cuir | Maille | Fer | Or | Diamant |
|-------|------|--------|-----|-----|---------|
| Casque | 1 | 2 | 2 | 2 | 3 |
| Plastron | 3 | 5 | 6 | 5 | 8 |
| Jambières | 2 | 4 | 5 | 3 | 6 |
| Bottes | 1 | 1 | 2 | 1 | 3 |
| Total | 7 | 12 | 15 | 11 | 20 |
```

### 6.4 Faim & Nourriture

```
Barre de faim : 20 points (10 jambons)
Saturation : valeur cachée, 0 à 20 (cap = hunger level)
Exhaustion : compteur caché, quand ≥ 4.0 → -1 saturation (ou -1 hunger si sat=0)

Actions qui augmentent l'exhaustion :
- Sprint : 0.1/mètre
- Sauter : 0.05
- Sauter en sprint : 0.2
- Attaquer : 0.1
- Miner un bloc : 0.005
- Nager : 0.01/mètre
- Dégâts de faim : 6.0/demi-cœur

Effets du niveau de faim :
- Faim ≥ 18 : Régénération naturelle (1 HP/4s)
- Faim ≤ 6 : Impossible de sprinter
- Faim = 0 : Dégâts de famine

Aliments (principaux) :
| Nourriture | Faim | Saturation |
|-----------|------|------------|
| Pomme | 4 | 2.4 |
| Pain | 5 | 6.0 |
| Steak | 8 | 12.8 |
| Porc cuit | 8 | 12.8 |
| Poulet cuit | 6 | 7.2 |
| Poulet cru | 2 | 1.2 (+30% poison) |
| Carotte | 3 | 3.6 |
| Pomme de terre cuite | 5 | 6.0 |
| Pastèque (tranche) | 2 | 1.2 |
| Cookie | 2 | 0.4 |
| Tarte citrouille | 8 | 4.8 |
| Gâteau | 14 total (7 tranches × 2) | 2.8 total |
| Golden Apple | 4 | 9.6 + Absorption 2 + Regen II 5s |
| Enchanted G.Apple | 4 | 9.6 + Abs 4 + Regen V 30s + Resist 5min + Fire Resist 5min |
| Poisson cru | 2 | 0.4 |
| Poisson cuit | 5 | 6.0 |
| Saumon cuit | 6 | 9.6 |
| Viande pourrie | 4 | 0.8 (+80% Hunger effect 30s) |
| Spider Eye | 2 | 3.2 (+poison) |
```

### 6.5 Expérience (XP)

```
Sources d'XP :
- Mobs : 1-5 orbs par mob (variable)
- Mining : charbon=0-1, diamant=3-7, redstone=1-5, lapis=2-5, quartz=2-5
- Smelting : 0.1-1.0 par item fondu (récolté en extrayant l'output)
- Breeding : 1-7
- Fishing : 1-6
- Trading : 3-6

Formule de niveau :
- Niveaux 0-16 : XP_total = 2*level² + 7*level
- Niveaux 17-31 : XP_total = 5*level² - 38*level + 360
- Niveaux 32+ : XP_total = 9*level² - 158*level + 2220

Utilisations :
- Enchantement (dépense 1-3 niveaux + lapis)
- Enclume (dépense niveaux variables)
- Mending : Non, ajouté en 1.9. EXCLU.
```

---

## 7. PHASE 4 — SYSTÈME D'INVENTAIRE & CRAFTING

### 7.1 Inventaire du Joueur

```
Layout :
┌─────────────────────────────────┐
│  [Craft 2x2]  [Output]         │
│  [Helmet] [Player  ] [Shield?] │  ← Pas de shield en 1.8
│  [Chest ] [Preview ] [        ]│
│  [Legs  ] [        ] [        ]│
│  [Boots ] [        ] [        ]│
│                                 │
│  [0][1][2][3][4][5][6][7][8]   │  ← Inventaire principal (27 slots)
│  [9][10][11]...           [26] │
│  [27][28]...              [35] │
│                                 │
│  [HB0][HB1][HB2]...[HB8]      │  ← Hotbar (9 slots)
└─────────────────────────────────┘

Interactions :
- Clic gauche : prendre/poser stack complet
- Clic droit : prendre moitié / poser un seul
- Shift+clic : transfert rapide (inventaire ↔ hotbar, inventaire ↔ coffre)
- Double clic : rassembler tous les items identiques
- Drag : répartir items sur plusieurs slots
- Q : jeter 1 item de la hotbar
- Ctrl+Q : jeter le stack entier
- Clic molette (creative) : copier le bloc ciblé
- 1-9 : sélectionner slot hotbar
```

### 7.2 Table de Crafting

```
Grille 3x3 avec slot output
Pattern matching :
1. Shaped recipes : le pattern doit matcher exactement (translation autorisée)
2. Shapeless recipes : tous les ingrédients présents, peu importe la position
3. Le joueur peut aussi crafter en 2x2 (recettes compatibles)

Exemple de pattern shaped :
Pioche en diamant :
[diamond] [diamond] [diamond]
[  none ] [ stick ] [  none ]
[  none ] [ stick ] [  none ]

Ce pattern peut être posé n'importe où dans la grille tant que la forme est respectée.
```

### 7.3 Fourneau (Furnace)

```
┌────────────────────┐
│  [Input     ]      │
│                     │
│  [Fuel      ] →→ [Output]  │
│  ████████████      │  ← Barre progression fuel
│  >>>>>>>>>>>       │  ← Barre progression smelt
└────────────────────┘

Logique :
- 1 smelt = 200 ticks (10 secondes)
- Le fuel se consume, chaque fuel a une durée en ticks
- Si plus de fuel et input présent → s'éteint
- Fuel restant brûle même sans input

Combustibles principaux :
| Fuel | Durée (ticks) | Items smeltés |
|------|--------------|---------------|
| Bois (planks) | 300 | 1.5 |
| Charbon | 1600 | 8 |
| Charbon de bois | 1600 | 8 |
| Seau de lave | 20000 | 100 |
| Blaze rod | 2400 | 12 |
| Bâton | 100 | 0.5 |
| Tronc | 300 | 1.5 |
| Bloc de charbon | 16000 | 80 |

Recettes smelting principales :
| Input | Output | XP |
|-------|--------|-----|
| Cobblestone | Stone | 0.1 |
| Sand | Glass | 0.1 |
| Iron ore | Iron ingot | 0.7 |
| Gold ore | Gold ingot | 1.0 |
| Raw Chicken | Cooked Chicken | 0.35 |
| Raw Beef | Steak | 0.35 |
| Raw Porkchop | Cooked Porkchop | 0.35 |
| Raw Fish | Cooked Fish | 0.35 |
| Clay ball | Brick | 0.3 |
| Netherrack | Nether Brick item | 0.1 |
| Wet sponge | Sponge | 0.15 |
| Log (any) | Charcoal | 0.15 |
| Cactus | Green Dye | 0.2 |
| Stone Bricks | Cracked Stone Bricks | 0.1 |
| Clay block | Terracotta | 0.35 |
```

---

## 8. PHASE 5 — MOBS & IA

### 8.1 Système d'Entités

```
Hiérarchie :
Entity (base)
├── LivingEntity
│   ├── Player
│   ├── PassiveMob
│   │   ├── Cow, Pig, Sheep, Chicken, Squid, Bat...
│   │   └── Villager
│   ├── HostileMob
│   │   ├── Zombie, Skeleton, Creeper, Spider...
│   │   └── EnderDragon, Wither
│   └── NeutralMob
│       ├── Wolf, ZombiePigman, Enderman
│       └── IronGolem (si attaqué)
├── Projectile
│   ├── Arrow, Snowball, Egg, EnderPearl, Fireball
│   └── ThrownPotion
├── ItemEntity (item au sol)
├── XPOrb
├── FallingBlock (sable, gravier)
├── TNTPrimed
├── Boat
├── Minecart (+ variantes)
└── Painting, ItemFrame, ArmorStand

Propriétés communes LivingEntity :
- Position (x, y, z) en double précision
- Velocity (vx, vy, vz)
- Rotation (yaw, pitch)
- Health (float)
- Max Health
- Hurt timer (invulnérabilité 0.5s après dégât)
- Fire timer
- Age
- AABB (hitbox)
```

### 8.2 Système IA

```
Architecture Goal-Based AI (fidèle Minecraft) :

Chaque mob a une liste de Goals avec priorités.
Chaque tick, l'AIController :
1. Parcourt les goals par priorité (bas = plus important)
2. Un goal peut start() si canStart() retourne true
3. Un goal actif continue tant que canContinue()
4. Un goal de priorité supérieure interrompt les goals inférieurs

class AIGoal {
    int priority;
    bool canStart();
    bool canContinue();
    void start();
    void tick();
    void stop();
};
```

### 8.3 Pathfinding A*

```
Grille : le monde voxel lui-même
Noeud : position bloc (x, y, z) + coût
Voisins : 4 directions cardinales (pas de diagonale pour les blocs)
  + check si le mob peut monter (bloc devant + bloc au-dessus = air)
  + check si le mob peut descendre (pas de chute fatale pour le mob)

Coûts :
- Déplacement normal : 1.0
- Eau : 2.0 (les mobs n'aiment pas l'eau)
- Trapdoor ouvert : pathable
- Porte fermée : les Villagers peuvent ouvrir → pathable pour eux

Heuristique : distance Manhattan 3D
Distance max : 32 blocs (au-delà, le mob abandonne)
Max nodes évalués : 200 (pour performance)
Recalcul : toutes les 1-2 secondes
```

### 8.4 Spawn Rules

```
Spawning naturel :
- Vérifié chaque tick de jeu
- Spawn cap par catégorie :
  - Hostile : 70
  - Passive : 10
  - Water : 5
  - Ambient (bat) : 15
- Distance joueur : 24-128 blocs (spawn dans cette zone)
- Despawn si > 128 blocs du joueur
- Despawn aléatoire si 32-128 blocs

Conditions :
- Hostile : light level ≤ 7 sur le bloc, bloc solide en dessous
- Passive : light level ≥ 9, grass block en dessous (sauf squid → eau)
- Cave Spider : uniquement dans les mineshafts
- Enderman : tous biomes, night or cave
- Slime : uniquement dans certains chunks (seed-based) sous Y=40, OU dans les swamps la nuit
- Blaze, Ghast, etc. : Nether uniquement, dans/près des forteresses
- Guardian : Ocean Monument uniquement
```

### 8.5 Détail de chaque Mob

#### Mobs Passifs

```
COW
- HP : 10 (5 cœurs)
- Drops : 1-3 Raw Beef, 0-2 Leather
- Comportement : wander, follow player (wheat), breed (wheat)
- Interaction : seau → lait

PIG
- HP : 10
- Drops : 1-3 Raw Porkchop
- Comportement : wander, follow (carrot, carrot on stick), breed (carrot)
- Montable avec selle + carrot on stick

SHEEP
- HP : 8
- Drops : 1 Wool (couleur), 1-2 Mutton
- Comportement : wander, eat grass (régénère laine), breed (wheat)
- Shears → 1-3 wool sans tuer
- 16 couleurs possibles (spawn : 81.836% blanc, 5% gris clair, 5% gris, 5% noir, 3% marron, 0.164% rose)

CHICKEN
- HP : 4
- Drops : 1 Raw Chicken, 0-2 Feather
- Comportement : wander, follow (seeds), breed (seeds), pond egg toutes les 5-10 min
- Tombe lentement (bat des ailes)

SQUID
- HP : 10
- Drops : 1-3 Ink Sac
- Spawn : dans l'eau
- Comportement : nage aléatoire, jet d'encre (particules) quand attaqué

BAT
- HP : 6
- Drops : rien
- Spawn : grottes (light ≤ 4, sous Y=63)
- Comportement : vol aléatoire, se pose au plafond à l'envers

HORSE
- HP : 15-30 (variable)
- Jump : 1.0-5.0 blocs (variable)
- Speed : 4.8-14.5 blocs/s (variable)
- Comportement : wander, tameable (monter dessus plusieurs fois)
- Équipable : selle + armure de cheval (fer, or, diamant)
- Variantes : cheval, âne, mule (breeding cheval + âne)
- Âne/mule : coffre portable

RABBIT
- HP : 3
- Drops : 0-1 Rabbit Hide, 0-1 Raw Rabbit, 10% Rabbit's Foot
- Comportement : wander, fuit joueur, mange carrots dans farmland
- Variantes : 6 skins + rare Killer Bunny (hostile)

OCELOT
- HP : 10
- Drops : 1-3 XP
- Spawn : Jungle uniquement
- Tameable : raw fish, devient chat (3 variantes)
- Fait fuir les creepers

WOLF
- HP : 8 (sauvage), 20 (apprivoisé)
- Drops : rien
- Spawn : Taiga, Forest
- Neutre : attaque si provoqué (meute entière)
- Tameable : os, attaque les mobs que le joueur attaque
- Heal : nourriture (viande)

MOOSHROOM
- HP : 10
- Spawn : Mushroom Island uniquement
- Comme la vache mais : shears → 5 champignons + devient vache, bol → soupe de champignons

VILLAGER
- HP : 20
- Spawn : Villages
- 6 professions (Farmer, Librarian, Priest, Blacksmith, Butcher, Nitwit)
- Trading : interface d'échange (offres progressives)
- Breeding : besoin de portes + nourriture
- Zombie Villager : 5% des zombies, curable (faiblesse + pomme dorée)
```

#### Mobs Hostiles

```
ZOMBIE
- HP : 20
- Dégâts : 2-4 (Easy/Normal/Hard)
- Drops : 0-2 Rotten Flesh, rare : Iron Ingot, Carrot, Potato, Iron Shovel/Sword
- Comportement : poursuit joueur dans un rayon de 35 blocs, peut casser les portes (Hard)
- Brûle au soleil (sauf casque ou à l'ombre)
- Baby zombie : 5% chance, plus rapide, ne brûle pas au soleil
- Zombie Villager : 5% chance, même comportement

SKELETON
- HP : 20
- Dégâts : 1-4 (variable, arc)
- Drops : 0-2 Bones, 0-2 Arrows, rare : arc, enchanted bow
- Comportement : garde distance ~15 blocs, tire des flèches, fuit quand trop proche
- Brûle au soleil
- Peut ramasser armure/armes au sol

CREEPER
- HP : 20
- Explosion : power 3 (rayon ~3.5 blocs)
- Drops : 0-2 Gunpowder, Music Disc (si tué par skeleton)
- Comportement : approche silencieusement, 1.5s fuse, flash blanc
- Charged Creeper : frappé par la foudre, explosion ×2
- NE brûle PAS au soleil

SPIDER
- HP : 16
- Dégâts : 2-3
- Drops : 0-2 String, 0-1 Spider Eye
- Comportement : hostile la nuit, neutre le jour (sauf si provoqué)
- Peut grimper les murs verticalement
- Saute sur le joueur
- Spider Jockey : 1% chance, skeleton monté sur spider

CAVE SPIDER
- HP : 12
- Dégâts : 2 + Poison (Normal/Hard)
- Drops : comme Spider
- Spawn : Mineshafts uniquement (avec spawners)
- Plus petit que le spider normal (passe dans des espaces 1×0.5)

ENDERMAN
- HP : 40
- Dégâts : 4-10
- Drops : 0-1 Ender Pearl
- Comportement : téléporte aléatoirement, prend/pose certains blocs
- Hostile SEULEMENT si le joueur le regarde (crosshair sur le haut du corps)
- Vulnérable à l'eau (téléporte pour fuir)
- Très rapide
- Spawne dans tous les biomes (rare)

SLIME
- HP : 16 (big), 4 (small), 1 (tiny)
- Drops : 0-2 Slimeball (tiny uniquement)
- Spawn : sous Y=40 dans certains chunks, Swamp (nuit)
- Comportement : saute vers le joueur
- Split : big → 2-4 small → 2-4 tiny

WITCH
- HP : 26
- Dégâts : via potions (poison, harm, slowness, weakness)
- Drops : 0-6 items (redstone, glowstone, gunpowder, sugar, spider eye, glass bottle, stick)
- Comportement : jette des potions offensives, boit des potions défensives
- Spawn : Witch Hut (structure), rare natural spawn

GUARDIAN
- HP : 30
- Dégâts : 5 (laser), 2 (thorns)
- Drops : 0-2 Prismarine Shard, 0-1 Prismarine Crystal, Raw Fish
- Spawn : Ocean Monument
- Comportement : laser (charge 2s, puis dégât), nage

ELDER GUARDIAN
- HP : 80
- Drops : comme Guardian + Wet Sponge, rare : Prismarine Shard
- Spawn : 3 par Ocean Monument (ne respawnent pas)
- Mining Fatigue III infligé aux joueurs proches (toutes les 60s)
- Même comportement laser que Guardian mais plus puissant

SILVERFISH
- HP : 8
- Dégâts : 1
- Drops : rien
- Spawn : Monster Egg blocks (Extreme Hills, Strongholds)
- Quand attaqué : appelle les silverfish voisins cachés

ENDERMITE
- HP : 8
- Dégâts : 2
- Drops : rien
- Spawn : 5% chance quand Ender Pearl utilisée
- Durée de vie : 2 minutes
- Détesté par les Endermen
```

#### Mobs du Nether

```
ZOMBIE PIGMAN
- HP : 20
- Dégâts : 5-13 (gold sword)
- Drops : 0-1 Rotten Flesh, 0-1 Gold Nugget, rare : Gold Ingot, Gold Sword
- Comportement : NEUTRE, hostile si provoqué (tout le groupe attaque)
- Immunisé au feu

GHAST
- HP : 10
- Dégâts : variable (fireball, explosion power 1)
- Drops : 0-1 Ghast Tear, 0-2 Gunpowder
- Comportement : vole, tire des fireballs (réfléchissables par le joueur !)
- Grand hitbox (4×4×4)

BLAZE
- HP : 20
- Dégâts : 5 (melee) + fire, 5 (fireball)
- Drops : 0-1 Blaze Rod
- Spawn : Nether Fortress (spawners)
- Comportement : vole, tire 3 fireballs en rafale, corps enflammé
- Vulnérable aux snowballs (3 dégâts)

WITHER SKELETON
- HP : 20
- Dégâts : 5-10 + Wither effect (10s)
- Drops : 0-2 Bones, 0-1 Coal, rare : Stone Sword, SKULL (2.5%)
- Spawn : Nether Fortress
- 2.5 blocs de haut (plus grand que skeleton normal)
- Épée en pierre

MAGMA CUBE
- HP : 16/4/1 (comme slime, 3 tailles)
- Dégâts : 6/4/3 par taille
- Drops : 0-1 Magma Cream
- Spawn : Nether, toutes les zones
- Immunisé au feu
```

#### Boss Mobs

```
WITHER
- HP : 300 (150 cœurs)
- Invocation : 4 Soul Sand en T + 3 Wither Skeleton Skulls
- Phase 1 (>150HP) : vole, tire Wither Skulls (explosive, wither effect)
- Phase 2 (<150HP) : armure anti-projectile, charge, dash
- Drops : 1 Nether Star (pour Beacon)
- Détruit les blocs autour de lui (sauf bedrock, end portal, etc.)
- Boss bar affichée

ENDER DRAGON (détaillé section 14)
- HP : 200 (100 cœurs)
```

### 8.6 Modèles de Mobs

```
Système de modèle :
- Chaque mob est composé de "cuboids" (boîtes texturées)
- Chaque cuboid peut pivoter autour d'un point d'ancrage
- Animation par interpolation des rotations

Exemple modèle Zombie :
- Tête : 8×8×8, pivot au cou
- Corps : 8×12×4, pivot aux épaules
- Bras droit : 4×12×4, pivot épaule droite
- Bras gauche : 4×12×4, pivot épaule gauche
- Jambe droite : 4×12×4, pivot hanche droite
- Jambe gauche : 4×12×4, pivot hanche gauche
- Animation marche : jambes/bras oscillent en phase opposée
- Animation attaque : bras tendus vers l'avant (zombie-style)

Texture unwrap : comme Minecraft, 64×64 skin map
```

---

## 9. PHASE 6 — COMBAT (STYLE 1.8)

### 9.1 Système de Combat 1.8

```
DIFFÉRENCE CLÉ avec post-1.9 : PAS de cooldown d'attaque.
Le joueur peut spam-cliquer pour faire des dégâts maximum à chaque coup.
C'est LE point critique du PvE/PvP 1.8.

Dégâts d'attaque (main hand) :
| Arme | Dégâts |
|------|--------|
| Main nue | 1 |
| Épée bois | 5 |
| Épée pierre | 6 |
| Épée fer | 7 |
| Épée diamant | 8 |
| Épée or | 5 |
| Hache bois | 4 |
| Hache pierre | 5 |
| Hache fer | 6 |
| Hache diamant | 7 |
| Pioche bois | 3 |
| Pioche pierre | 4 |
| Pioche fer | 5 |
| Pioche diamant | 6 |

Knockback :
- De base : 0.4 blocs horizontalement + 0.36 verticalement
- Sprint hit : knockback ×1.5 (+ reset sprint)
- Enchant Knockback I : +0.5
- Enchant Knockback II : +1.0

Invulnerability frames : 0.5 secondes (10 ticks) après chaque hit
Si un second coup arrive pendant l'invulnérabilité :
- Si dégâts > précédent : applique la DIFFÉRENCE uniquement
- Sinon : aucun dégât

Blocking (épée) :
- Clic droit avec épée en main
- Réduit les dégâts de 50%
- Réduit la vitesse de mouvement
- Pas de shield en 1.8 !
```

### 9.2 Arc

```
Charge : maintenir clic droit (max 1 seconde pour charge complète)
Dégâts : 1-9 basé sur la charge + critical si charge max
Précision : léger random spread
Gravité : la flèche suit une trajectoire parabolique
Flèches : consommées depuis l'inventaire (sauf en Creative)
Enchantements : Power, Punch, Flame, Infinity
```

---

## 10. PHASE 7 — REDSTONE & MÉCANISMES

### 10.1 Signal Redstone

```
Signal : valeur 0-15
Sources de signal :
- Redstone Torch : 15 (toujours ON sauf si alimentée → OFF)
- Lever : 15 (toggle)
- Button : 15 pendant 1s (stone) ou 1.5s (wood)
- Pressure Plate : 15 quand entité dessus
- Tripwire : 15 quand traversé
- Daylight Sensor : 0-15 selon l'heure
- Redstone Block : 15 (permanent)
- Trapped Chest : 0-15 selon nb de joueurs
- Comparator output : variable

Propagation :
- Redstone dust : transmet avec -1 par bloc (15 max range)
- Connexion : les fils se connectent aux blocs adjacents qui sont des composants
- La poudre ne monte pas sur les blocs sauf si chemin d'escalier
```

### 10.2 Composants Redstone

```
REPEATER :
- Délai : 1-4 ticks (clic droit pour changer)
- Unidirectionnel (entrée arrière → sortie avant)
- Lock : un repeater alimenté sur le côté verrouille l'état
- Signal de sortie toujours 15

COMPARATOR :
- Mode Compare : output = back signal si back ≥ side, sinon 0
- Mode Subtract : output = max(0, back - side)
- Peut lire le contenu des containers (coffre, fourneau → signal proportionnel au remplissage)
- Output 0-15

PISTON :
- Pousse jusqu'à 12 blocs
- Ne peut pas pousser : obsidian, bedrock, extended piston, enchanting table
- Rétraction : 1 tick
- Extension : 0.5 tick (quasi-connectivity bug fidèle MC 1.8)

STICKY PISTON :
- Comme piston mais tire le bloc en se rétractant
- Ne tire PAS les blocs qui ne peuvent être poussés
- Slime block interaction : colle les blocs adjacents (tire/pousse en groupe)

DISPENSER :
- Activé par signal redstone (front montant)
- Distribue/utilise les items :
  - Flèche → tire
  - Snowball/Egg → lance
  - TNT → place et allume
  - Seau d'eau/lave → place le liquide
  - Seau vide → récupère le liquide
  - Feu d'artifice → lance
  - Bonemeal → fertilise
  - Armure → équipe sur joueur/armor stand devant
  - Autre → drop l'item

DROPPER :
- Comme dispenser mais TOUJOURS drop l'item (ne l'utilise jamais)
- Transfert dans un container adjacent si présent

HOPPER :
- Transfert items : container au-dessus → hopper → container en dessous/devant
- 2.5 items/seconde (1 item tous les 8 ticks)
- Désactivé par signal redstone

NOTE BLOCK :
- Clic droit : change la note (2 octaves, 25 notes)
- Instrument dépend du bloc en dessous (bois=basse, sable=caisse claire, etc.)
- Activé par redstone → joue la note
```

### 10.3 Portes, Trappes, Fence Gates

```
Tous activables par :
- Clic droit (toggle manuel)
- Signal redstone (ouvert si alimenté)
- Villagers peuvent ouvrir/fermer les portes en bois

Iron Door : uniquement par redstone
Wood Door : redstone + clic droit
```

### 10.4 TNT

```
Activation :
- Redstone signal
- Feu/lave
- Explosion adjacente
- Flèche enflammée

Fuse : 4 secondes (80 ticks)
Power : 4 (rayon destruction ~3-4 blocs)
Entité : la TNT amorcée est une entité avec physique (gravité, knockback d'explosion)

Explosion algorithm :
1. Tracer des rayons depuis le centre dans toutes les directions
2. Chaque rayon a une force initiale = power * (0.7 + random * 0.6)
3. Pour chaque bloc sur le rayon :
   a. Réduire la force de (blastResistance + 0.3) * 0.3
   b. Si force > 0 → détruire le bloc (30% chance de drop l'item)
4. Dégâts aux entités : basé sur la distance et l'exposition (raycasting)
5. Knockback : proportionnel aux dégâts
```

---

## 11. PHASE 8 — ENCHANTEMENT, BREWING & ANVIL

### 11.1 Table d'Enchantement

```
Interface :
┌──────────────────────────────┐
│ [Item] [Lapis]               │
│                              │
│ ▶ Enchantment I    (1 level) │
│ ▶ Enchantment II   (2 levels)│
│ ▶ Enchantment III  (3 levels)│
└──────────────────────────────┘

Mécanique :
- Étagères (bookshelves) : 0-15 autour de la table (max 30, mais 15 comptent)
- Distance : 1 bloc d'air entre la table et les étagères
- Chaque étagère augmente le max level des enchantments proposés
- 15 étagères = enchantments niveau max
- Coût : 1/2/3 niveaux d'XP + 1/2/3 lapis lazuli
- Le joueur voit un hint du premier enchantement (texte galactique + nom)
- Les enchantments sont générés aléatoirement (seed basé sur slot + XP seed)
- Après enchantement, les options se regénèrent
```

### 11.2 Enchantements Disponibles (1.8)

```
ARMES (Épée) :
| Enchantement | Niveaux | Effet |
|-------------|---------|-------|
| Sharpness | I-V | +1.25 dégâts par niveau |
| Smite | I-V | +2.5 vs undead par niveau |
| Bane of Arthropods | I-V | +2.5 vs arthropodes + slowness |
| Knockback | I-II | +knockback |
| Fire Aspect | I-II | Enflamme la cible (4s par niveau) |
| Looting | I-III | +1 max drop par niveau |
| Unbreaking | I-III | Chance de ne pas perdre durabilité |

ARC :
| Power | I-V | +25% dégâts par niveau |
| Punch | I-II | +knockback flèche |
| Flame | I | Flèches enflammées |
| Infinity | I | Pas de consommation de flèches (besoin 1 en inventaire) |
| Unbreaking | I-III | |

OUTILS (Pioche, Hache, Pelle) :
| Efficiency | I-V | +vitesse de minage |
| Silk Touch | I | Drop le bloc lui-même |
| Fortune | I-III | +drops (incompatible Silk Touch) |
| Unbreaking | I-III | |

ARMURE :
| Protection | I-IV | -4% dégâts par niveau |
| Fire Protection | I-IV | -8% dégâts feu |
| Blast Protection | I-IV | -8% dégâts explosion |
| Projectile Protection | I-IV | -8% dégâts projectile |
| Thorns | I-III | Renvoie 1-4 dégâts |
| Unbreaking | I-III | |

CASQUE :
| Respiration | I-III | +15s respiration sous l'eau par niveau |
| Aqua Affinity | I | Minage normal sous l'eau |

BOTTES :
| Feather Falling | I-IV | -12% dégâts chute par niveau |
| Depth Strider | I-III | Vitesse dans l'eau |

CANNE À PÊCHE :
| Luck of the Sea | I-III | Meilleur loot |
| Lure | I-III | Prise plus rapide |
| Unbreaking | I-III | |

TOUS :
| Unbreaking | I-III | applicable à tout |
```

### 11.3 Enclume (Anvil)

```
Fonctions :
1. Combiner 2 items identiques → somme durabilité + 12% bonus
2. Appliquer un livre enchanté sur un item
3. Combiner enchantements (même item + enchanted book)
4. Renommer un item

Coût en XP :
- Base : dépend des enchantements
- Prior Work Penalty : double à chaque utilisation de l'enclume (0, 1, 3, 7, 15, 31...)
- Max : 39 niveaux ("Too Expensive!" au-delà)
- Renommer : 1 niveau de base

Usure : L'enclume s'abîme à chaque utilisation (3 états → destruction)
```

### 11.4 Brewing (Potions)

```
Interface Alambic :
┌──────────────────────────┐
│      [Ingrédient]        │
│       ↓ ↓ ↓              │
│  [Potion1][Potion2][Potion3] │
│       [Blaze Powder]     │  ← Fuel (1.9+ seulement? Non, ajouté en 1.9)
└──────────────────────────┘

NOTE : En 1.8, le brewing stand n'a PAS besoin de Blaze Powder comme fuel.
Le fuel a été ajouté en 1.9. En 1.8, il suffit de mettre l'ingrédient + les bouteilles.

Processus :
1. Remplir des bouteilles d'eau (clic droit sur source d'eau)
2. Placer dans l'alambic
3. Ajouter ingrédient → 20 secondes de brassage
4. Résultat : potion

Arbre de brewing :
Water Bottle
├── + Nether Wart → Awkward Potion (base de presque tout)
│   ├── + Sugar → Speed
│   ├── + Rabbit Foot → Leaping
│   ├── + Blaze Powder → Strength
│   ├── + Glistering Melon → Healing (instant)
│   ├── + Spider Eye → Poison
│   ├── + Ghast Tear → Regeneration
│   ├── + Magma Cream → Fire Resistance
│   ├── + Golden Carrot → Night Vision
│   │   └── + Fermented Spider Eye → Invisibility
│   └── + Pufferfish → Water Breathing
├── + Fermented Spider Eye → Weakness
└── + Glowstone / Redstone → (enhanced / extended)

Modificateurs :
- Redstone Dust : durée prolongée (ex: 3min → 8min)
- Glowstone Dust : niveau augmenté (ex: Speed → Speed II, durée réduite)
- Fermented Spider Eye : corrompt l'effet (Healing → Harming, etc.)
- Gunpowder : transforme en Splash Potion (lançable)

Potions complètes :
| Potion | Effet | Durée base |
|--------|-------|-----------|
| Speed | +20% vitesse | 3:00 |
| Speed II | +40% vitesse | 1:30 |
| Slowness | -15% vitesse | 1:30 |
| Strength | +130% dégâts melee | 3:00 (note: en 1.8, +130%, pas +3 comme en 1.9+) |
| Strength II | +260% dégâts | 1:30 |
| Healing | +4 HP instant | instant |
| Healing II | +8 HP instant | instant |
| Harming | -6 HP instant | instant |
| Harming II | -12 HP instant | instant |
| Regeneration | +1 HP/2.5s | 0:45 |
| Poison | -1 HP/1.25s (min 1HP) | 0:45 |
| Fire Resistance | Immunité feu/lave | 3:00 |
| Night Vision | Vision parfaite | 3:00 |
| Invisibility | Invisible (mobs ne voient pas) | 3:00 |
| Water Breathing | Pas de noyade | 3:00 |
| Leaping | +jump boost | 3:00 |
| Weakness | -0.5 dégâts melee | 1:30 |
```

---

## 12. PHASE 9 — STRUCTURES & VILLAGES

### 12.1 Structures Générées

```
VILLAGE :
- Biomes : Plains, Desert, Savanna
- Composants : maisons, forge, église, bibliothèque, puits, fermes
- Villagers : 1-3 par bâtiment
- Forge contient coffre avec loot
- Chemins en gravel/grass path entre les bâtiments
- Taille : 5-25 bâtiments
- Iron Golem : spawn si 10+ villagers et 21+ portes

DESERT TEMPLE :
- Structure en sandstone
- 4 coffres au sous-sol
- TNT trap sous les coffres (pressure plate)
- Loot : or, diamant, fer, émeraude, os, rotten flesh, enchanted books

JUNGLE TEMPLE :
- Structure en cobblestone + mossy cobblestone
- 2 coffres
- Piège : dispensers avec flèches + tripwire
- Puzzle : 3 leviers (combinaison pour ouvrir le coffre caché)

WITCH HUT :
- Biome : Swamp
- Petite cabane en bois sur pilotis
- 1 Witch spawn
- Pot de fleur avec champignon rouge

OCEAN MONUMENT :
- Biome : Deep Ocean
- Structure : prismarine + sea lanterns
- 3 Elder Guardians (boss mini)
- Sponge rooms
- Gold block core (8 blocs d'or cachés)
- Guardians spawn à l'intérieur et autour

DUNGEON :
- Petite pièce en cobblestone (5×5 à 7×7)
- 1-2 coffres avec loot
- 1 spawner (zombie 50%, skeleton 25%, spider 25%)
- Génère n'importe où sous terre

MINESHAFT :
- Réseau de tunnels en bois (oak planks + fence)
- Rail + rail avec chest minecart
- Cave Spider spawners avec cobwebs
- Génère sous terre (tous biomes, en surface dans Mesa)

STRONGHOLD :
- 3 par monde (en anneau autour du spawn, ~1000-2000 blocs)
- Composants : couloirs, escaliers, bibliothèque, fontaine, prison
- End Portal Room : cadre de portail de l'End avec silverfish spawner
- Contient coffres avec loot varié
- Localisable avec Eye of Ender

NETHER FORTRESS :
- Structure en nether brick
- Couloirs, escaliers, balcons
- Blaze spawners
- Wither Skeleton spawn zone
- Coffres avec loot (diamant, or, armure, saddle, horse armor)
- Nether Wart farm (soul sand + nether wart)

END CITY : Non, ajouté en 1.9. EXCLU.
```

### 12.2 Loot Tables

```
Chaque coffre de structure a une loot table avec :
- Pool d'items possibles
- Poids (probabilité relative)
- Quantité min/max
- Enchantements aléatoires possibles

Exemple Dungeon Chest :
| Item | Poids | Quantité |
|------|-------|----------|
| Saddle | 20 | 1 |
| Iron Ingot | 10 | 1-4 |
| Bread | 20 | 1 |
| Wheat | 20 | 1-4 |
| Gunpowder | 10 | 1-4 |
| String | 10 | 1-4 |
| Bucket | 10 | 1 |
| Redstone | 10 | 1-4 |
| Music Disc | 5 | 1 |
| Golden Apple | 1 | 1 |
| Name Tag | 5 | 1 |
| Horse Armor | 5 | 1 |
| Enchanted Book | 10 | 1 |
```

---

## 13. PHASE 10 — LE NETHER

### 13.1 Portail du Nether

```
Construction :
- Cadre en Obsidian : minimum 4×5 (intérieur 2×3), max 23×23
- Activation : Flint & Steel sur l'intérieur
- Effet : texture violette animée + son ambiant + particules
- Téléportation : 4 secondes dans le portail → transfert

Conversion coordonnées :
- Nether X/Z = Overworld X/Z ÷ 8
- Y inchangé
- Le jeu cherche/crée un portail correspondant dans la dimension cible
```

### 13.2 Génération du Nether

```
Dimension :
- Hauteur : 0-128 (bedrock en haut et en bas)
- Bloc principal : Netherrack
- Plafond et sol en bedrock (irrégulier)
- Grandes cavernes intérieures (bruit 3D)

Caractéristiques :
- Océans de lave à Y=31
- Gravel (poches)
- Soul Sand (couches au sol, surtout dans les fortresses)
- Glowstone : clusters au plafond
- Nether Quartz Ore : comme les minerais de l'Overworld, dans le netherrack
- Nether Wart : dans les Nether Fortresses (soul sand)
- Mushrooms : sur netherrack (rare)
- Fire : feu éternel sur netherrack

Mobs :
- Zombie Pigman (omniprésent)
- Ghast (grands espaces)
- Blaze (Fortress)
- Wither Skeleton (Fortress)
- Magma Cube (partout, rare)
- Pas de spawn de mobs passifs
```

---

## 14. PHASE 11 — L'END & DRAGON

### 14.1 Accès à l'End

```
Étapes requises :
1. Obtenir des Ender Pearls (drop Enderman)
2. Obtenir du Blaze Powder (smelt Blaze Rod)
3. Crafter des Eyes of Ender (Pearl + Powder)
4. Utiliser Eye of Ender en l'air → vole vers le Stronghold le plus proche
5. Trouver la End Portal Room dans le Stronghold
6. Remplir les 12 cadres avec Eyes of Ender (certains déjà remplis aléatoirement, 10% chacun)
7. Le portail s'active → sauter dedans
```

### 14.2 Génération de l'End

```
Terrain :
- Île principale : grande plateforme de End Stone
- Void en dessous et autour (chute = mort)
- Bedrock pillar au centre (fontaine de retour, initialement inactif)
- Obsidian Pillars : 10 tours d'obsidian en cercle autour du centre
  - Hauteur variable (76-103)
  - Crystal d'End au sommet de chaque tour
  - Certaines tours ont des barres de fer autour du crystal (cage)

End Crystals :
- 10 au total
- Heal l'Ender Dragon continuellement (beam visible)
- Explosent quand détruits (dégâts aux entités proches)
- DOIVENT être détruits pour pouvoir tuer le Dragon
```

### 14.3 Combat de l'Ender Dragon

```
ENDER DRAGON
- HP : 200 (100 cœurs)
- Boss bar : affichée en haut de l'écran

Phases de vol :
1. HOLDING : Vole en cercle autour des tours d'obsidian
   - Altitude élevée
   - Descend parfois pour charger le joueur
   
2. STRAFING : Vole vers le joueur et crache des Dragon Fireballs
   - Dragon Breath : zone violette au sol (dégâts dans le temps)
   - Poison-like damage : 1 HP/seconde dans la zone
   
3. CHARGING : Fonce vers le joueur (dive bomb)
   - Dégâts de contact : 10 (5 cœurs)
   - Le joueur peut frapper le Dragon pendant la charge
   
4. PERCHING : Se pose sur la fontaine de bedrock
   - Le joueur peut attaquer la tête (dégâts ×4)
   - Le corps est résistant aux dégâts
   - Dure ~10 secondes
   - Dragon Breath particles au sol

Healing :
- Les End Crystals heal le Dragon à distance
- Beam visible entre le crystal et le Dragon
- Priorité : détruire les crystals d'abord
- Les crystals dans des cages nécessitent de casser les barres de fer

Immunités :
- Immune aux dégâts de flèches quand perché (rebondissent)
- Immune au feu, lave, noyade
- Seuls les coups melee et les flèches en vol font des dégâts

Mort :
1. Animation de mort : s'élève, commence à se désintégrer
2. Explosion de lumière
3. ~12000 XP orbs (niveau 0→78 en une fois)
4. Dragon Egg apparaît sur la fontaine de bedrock
5. Portail de retour (End Gateway) s'active
6. En entrant dans le portail → POÈME DE FIN
```

### 14.4 Poème de Fin (End Poem)

```
Après avoir sauté dans le portail de retour :
1. L'écran devient noir
2. Le texte du End Poem défile (dialogue entre deux entités)
3. Texte complet : ~9 minutes de défilement
4. Credits après le poème
5. Le joueur retourne à son spawn dans l'Overworld

Note légale : Le texte original du End Poem de Minecraft est sous copyright.
→ Écrire un poème de fin ORIGINAL inspiré du format
→ Même structure : dialogue philosophique, 2 voix, thèmes de création et de réalité
→ Durée similaire (~7-10 minutes de défilement)
→ Crédits du jeu (VoxelForge team) après le poème
```

### 14.5 Dragon Egg

```
- Apparaît au sommet de la fontaine de bedrock
- Clic dessus → se téléporte aléatoirement (rayon 15-20 blocs)
- Pour le récupérer : pousser avec piston OU placer torche en dessous et casser le bloc support (gravity)
- Bloc décoratif, aucun usage gameplay
- Unique dans le monde
```

---

## 15. PHASE 12 — AUDIO, PARTICULES & POLISH

### 15.1 Audio

```
Système : OpenAL avec audio positionnel 3D

Catégories de sons :
1. Ambiance musicale : tracks longues, jouées aléatoirement toutes les 10-30 min
   - Overworld day : 3-5 tracks calmes (piano, ambient)
   - Overworld night : 2-3 tracks plus sombres
   - Nether : 2-3 tracks oppressantes
   - End : 1-2 tracks éthérées
   - Combat boss : 1 track epic
   Note : COMPOSER DE LA MUSIQUE ORIGINALE (pas utiliser C418)

2. Sons de blocs :
   - Cassage : par matériau (stone, wood, sand, glass, cloth, grass, gravel, metal)
   - Placement : par matériau
   - Marche dessus : par matériau (pas du joueur)
   - Chaque matériau a 3-4 variantes

3. Sons de mobs :
   - Idle : son ambiant régulier
   - Hurt : quand touché
   - Death : quand tué
   - Special : creeper fuse, ghast scream, enderman scream, etc.

4. Sons d'ambiance :
   - Vent (cavernes)
   - Eau (rivières, océan)
   - Lave (bulles)
   - Pluie
   - Tonnerre
   - Nether ambiance (sons inquiétants random)

5. Sons UI :
   - Clic bouton
   - Ouverture inventaire/coffre
   - Level up XP (ding !)
   - Anvil use
   - Enchanting table
```

### 15.2 Particules

```
Types de particules :
- Block break : 10-20 petits cubes de la texture du bloc
- Critical hit : étoiles jaunes
- Enchant : glyphes flottants (autour enchanting table)
- Flame : petites flammes (torches, feu)
- Smoke : fumée (torches, feu)
- Water splash : gouttes d'eau
- Lava drip : gouttes de lave sous les blocs
- Rain : gouttes tombantes
- Snow : flocons
- Portal : particules violettes (portail nether)
- Redstone : particules rouges
- Explosion : grande bouffée blanche
- Heart : cœurs (breeding)
- Note : notes de musique (note block)
- Bubble : bulles sous l'eau
- Potion effect : spirales de la couleur de la potion
- Dragon Breath : particules violettes au sol
- End Rod particles : particules flottantes blanches (pas en 1.8, EXCLU)

Rendu : billboards (toujours face caméra), texture atlas de particules
Physique : gravité simple + friction + lifetime
```

### 15.3 Météo

```
Cycle :
- Beau temps → transition 0.5-1 jour → Pluie (durée 0.5-1 jour) → transition → Beau
- Orage : pendant la pluie, 5% chance par tick de devenir orage
- Neige : dans les biomes froids, remplace la pluie

Pluie :
- Particules de gouttes (depuis Y=256)
- Son ambient
- Assombrit le ciel (skylight réduit de 3)
- Éteint le feu
- Remplit les chaudrons (lentement)
- Les mobs undead ne brûlent pas

Orage :
- Comme la pluie + éclairs
- Lightning strike : random, peut :
  - Enflammer les blocs touchés
  - Transformer un cochon en zombie pigman
  - Transformer un creeper en charged creeper
  - Transformer un villager en witch
  - Dégâts aux entités (5 cœurs)
- Très sombre (mobs hostiles peuvent spawn même le jour)

Neige :
- Couche de neige sur les blocs (si biome froid)
- L'eau gèle dans les biomes froids
```

---

## 16. PHASE 13 — UI/UX & MENUS

### 16.1 HUD In-Game

```
┌─────────────────────────────────────────────┐
│                                             │
│                 [Crosshair +]               │
│                                             │
│                                             │
│                                             │
│  [XP Bar ================================] │
│  [♥♥♥♥♥♥♥♥♥♥]        [🍖🍖🍖🍖🍖🍖🍖🍖🍖🍖] │
│  [Armor: 🛡🛡🛡🛡🛡🛡🛡🛡🛡🛡]                │
│  [HB1][HB2][HB3][HB4][HB5][HB6][HB7][HB8][HB9] │
└─────────────────────────────────────────────┘

Éléments :
- Crosshair : + au centre (change quand on regarde une entité interactable)
- Hotbar : 9 slots, slot sélectionné surligné
- Hearts : 10 cœurs, animation de clignotement quand dégât, hearts dorés (absorption)
- Hunger : 10 jambons, tremblent quand faim basse
- Armor : 10 boucliers (affichés seulement si armure équipée)
- XP bar : barre verte avec numéro de niveau au milieu
- Air bubbles : 10 bulles (seulement sous l'eau, disparaissent au fur et à mesure)
- Boss bar : barre rose/violet en haut (Wither, Ender Dragon)
- Held item name : nom de l'item tenu (fade in/out)
- Action bar messages : messages temporaires
- Chat : en bas à gauche, messages du système/commandes
```

### 16.2 Menu Principal

```
┌─────────────────────────────────────┐
│                                     │
│           VOXELFORGE                │
│         [Panorama rotatif 3D]       │
│                                     │
│       [ Singleplayer ]              │
│       [ Options...   ]              │
│       [ Quit Game    ]              │
│                                     │
│  Splash text aléatoire !            │
│                                     │
│  VoxelForge v1.0   Copyright Paul   │
└─────────────────────────────────────┘

Le panorama de fond : 6 images (cube map) d'un monde rendu, qui tourne lentement.
Splash texts : liste de textes drôles affichés en jaune, inclinés.
```

### 16.3 Écrans de Jeu

```
Singleplayer :
- Liste des mondes sauvegardés
- Bouton "Create New World"
- Bouton "Delete" avec confirmation
- Info par monde : nom, seed, mode, taille, dernière date

Create World :
- Champ : World Name
- Champ : Seed (vide = aléatoire)
- Dropdown : Game Mode (Survival, Creative, Hardcore)
- Bonus Chest : toggle
- World Type : Default (seul type pour simplifier)
- Bouton "Create"

Options :
- FOV slider (30-110)
- Render distance slider (2-16 chunks)
- GUI scale (1-4, Auto)
- Fullscreen toggle
- VSync toggle
- Music volume (0-100%)
- Sound volume (0-100%)
- Mouse sensitivity (1-200%)
- Key bindings
- Difficulty (Peaceful, Easy, Normal, Hard)
- Language (pour simplifier : EN + FR)
- Brightness slider (0-100%)

Pause (Echap) :
- Back to Game
- Options...
- Save and Quit to Title

Death Screen :
- "You Died!"
- "Score: [xp total]"
- Bouton "Respawn"
- Bouton "Title Screen"
```

### 16.4 Debug Screen (F3)

```
Affichage F3 (indispensable pour le dev et les joueurs techniques) :
- FPS + chunk updates + entity count
- Coordonnées XYZ (avec decimals)
- Block position
- Chunk position
- Facing direction (N/S/E/W + angle)
- Biome
- Light level (sky + block)
- Day/game time (in ticks)
- Memory usage
- GPU info
- Targeted block info
- Entity count (loaded)
- Chunk count (loaded)
```

---

## 17. PHASE 14 — SAUVEGARDE & PACKAGING

### 17.1 Format de Sauvegarde

```
Structure fichiers :
saves/
└── [WorldName]/
    ├── level.dat              # Metadata monde (seed, time, player pos, gamemode, weather)
    ├── player.dat             # Inventaire, santé, XP, position
    ├── region/
    │   ├── r.0.0.dat          # Region file (32×32 chunks)
    │   ├── r.0.1.dat
    │   └── ...
    ├── nether/
    │   └── region/
    │       └── ...
    └── end/
        └── region/
            └── ...

Format Region (inspiré MCRegion/Anvil) :
- Header : 4096 bytes (1024 entrées × 4 bytes = offset+size pour chaque chunk)
- Chaque chunk : compressé avec zlib
- Données chunk : block IDs + metadata + light + heightmap + entities + tile entities

Format binaire simplifié (pas besoin d'être compatible MC réel) :
- level.dat : JSON ou format binaire custom
- player.dat : binaire (struct sérialisée)
- Chunks : palette-based compression
  - Palette locale par section (liste des block types présents)
  - Indices compactés (bits par entrée = ceil(log2(palette_size)))
```

### 17.2 Sauvegarde Automatique

```
- Autosave toutes les 5 minutes (configurable)
- Sauvegarde les chunks modifiés (dirty flag)
- Sauvegarde player data à chaque changement significatif
- Sauvegarde complète quand "Save and Quit"
- Loading : charger level.dat, puis charger les chunks autour du joueur
```

### 17.3 Packaging & Distribution

```
Build pipeline :
1. CMake → compilation C++ → exécutable
2. Copier assets/ à côté de l'exe
3. Emballer avec CPack ou NSIS

Structure distribuée :
VoxelForge/
├── VoxelForge.exe          # Exécutable principal
├── assets/                  # Textures, sons, shaders
│   └── ...
├── saves/                   # Créé au premier lancement
├── options.txt              # Settings utilisateur
└── logs/                    # Logs de debug

Ou en alternative : tout embarquer dans l'exe avec resource embedding.

Minimum specs cibles :
- OS : Windows 7+ (64-bit)
- CPU : Dual core 2 GHz
- RAM : 2 GB
- GPU : OpenGL 3.3 support (Intel HD 4000+, GTX 400+, Radeon HD 5000+)
- Stockage : 500 MB
```

---

## 18. REGISTRE COMPLET DES BLOCS

```
Note : Liste exhaustive des blocs de Minecraft 1.8.9 à implémenter.
ID = identifiant interne, data values pour variantes.

TERRAIN & NATUREL :
- Stone (0) + Granite, Polished Granite, Diorite, Polished Diorite, Andesite, Polished Andesite
- Grass Block (2)
- Dirt (3) + Coarse Dirt, Podzol
- Cobblestone (4)
- Bedrock (7)
- Sand (12) + Red Sand
- Gravel (13)
- Clay Block (82)
- Mycelium (110)
- Soul Sand (88)
- Netherrack (87)
- End Stone (121)
- Obsidian (49)
- Ice (79), Packed Ice (174)
- Snow Block (80), Snow Layer (78)

BOIS :
- Oak/Spruce/Birch/Jungle/Acacia/Dark Oak Log (17, 162)
- Oak/Spruce/Birch/Jungle/Acacia/Dark Oak Planks (5)
- Oak/Spruce/Birch/Jungle/Acacia/Dark Oak Leaves (18, 161)
- Oak/Spruce/Birch/Jungle/Acacia/Dark Oak Sapling (6)

MINERAIS :
- Coal Ore (16), Iron Ore (15), Gold Ore (14), Diamond Ore (56)
- Emerald Ore (129), Lapis Ore (21), Redstone Ore (73/74)
- Nether Quartz Ore (153)

BLOCS FABRIQUÉS :
- Planks (5, 6 variantes bois)
- Cobblestone (4)
- Stone Bricks (98) + Mossy, Cracked, Chiseled
- Bricks (45)
- Sandstone (24) + Chiseled, Smooth
- Red Sandstone (179) + Chiseled, Smooth
- Nether Brick (112)
- Quartz Block (155) + Chiseled, Pillar
- Prismarine (168) + Prismarine Bricks, Dark Prismarine
- Wool (35, 16 couleurs)
- Stained Clay/Terracotta (159, 16 couleurs)
- Hardened Clay (172)
- Stained Glass (95, 16 couleurs)
- Glass (20)
- Glass Pane (102)
- Stained Glass Pane (160, 16 couleurs)
- Glowstone (89)
- Sea Lantern (169)
- Bookshelf (47)
- TNT (46)
- Sponge (19) + Wet Sponge
- Hay Bale (170)
- Coal Block (173)
- Iron Block (42), Gold Block (41), Diamond Block (57)
- Emerald Block (133), Lapis Block (22), Redstone Block (152)
- Slime Block (165)

ESCALIERS (toutes variantes bois + stone) :
- Oak/Spruce/Birch/Jungle/Acacia/Dark Oak Stairs
- Cobblestone Stairs, Stone Brick Stairs, Brick Stairs
- Sandstone Stairs, Red Sandstone Stairs
- Nether Brick Stairs, Quartz Stairs

DALLES (toutes variantes) :
- Wood Slabs (6 types), Stone Slabs (stone, cobble, brick, stone brick, sandstone, nether brick, quartz, red sandstone)

PORTES & ACCÈS :
- Oak/Spruce/Birch/Jungle/Acacia/Dark Oak Door
- Iron Door
- Oak/Spruce/Birch/Jungle/Acacia/Dark Oak Fence
- Nether Brick Fence, Cobblestone Wall, Mossy Cobblestone Wall
- Fence Gates (6 types bois)
- Trapdoors (bois + fer)
- Ladder (65)

REDSTONE :
- Redstone Wire (55)
- Redstone Torch (75/76)
- Redstone Repeater (93/94)
- Redstone Comparator (149/150)
- Piston (33) + Sticky Piston (29)
- Dispenser (23)
- Dropper (158)
- Hopper (154)
- Lever (69)
- Stone/Wood Button (77/143)
- Stone/Wood/Iron/Gold Pressure Plate (70/72/147/148)
- Tripwire Hook (131) + Tripwire (132)
- Daylight Sensor (151)
- Note Block (25)
- Redstone Lamp (123/124)
- TNT (46)

CONTAINERS & UTILITAIRES :
- Chest (54), Trapped Chest (146), Ender Chest (130)
- Crafting Table (58)
- Furnace (61/62)
- Enchanting Table (116)
- Anvil (145, 3 états)
- Brewing Stand (117)
- Cauldron (118)
- Beacon (138)
- Jukebox (84)
- Bed (26, 1 couleur : rouge)

AGRICULTURE :
- Farmland (60)
- Wheat Crop (59, 8 stages)
- Carrot Crop (141, 8 stages)
- Potato Crop (142, 8 stages)
- Melon Stem/Block (103/106)
- Pumpkin Stem/Block (104/86)
- Cocoa (127, 3 stages)
- Sugar Cane (83)
- Cactus (81)
- Nether Wart (115, 4 stages)
- Vines (106)

PLANTES & FLEURS :
- Tall Grass (31) + Fern
- Dead Bush (32)
- Dandelion (37), Poppy (38), Blue Orchid, Allium, Azure Bluet
- Red/Orange/White/Pink Tulip
- Oxeye Daisy
- Sunflower, Lilac, Rose Bush, Peony (double tall plants, 175)
- Brown/Red Mushroom (39/40)
- Huge Mushroom blocks (99/100)
- Lily Pad (111)

RAILS :
- Rail (66)
- Powered Rail (27)
- Detector Rail (28)
- Activator Rail (157)

LUMIÈRE :
- Torch (50)
- Redstone Torch (75/76)
- Jack o'Lantern (91)
- Glowstone (89)
- Sea Lantern (169)
- Redstone Lamp (123/124)

DÉCO :
- Carpet (171, 16 couleurs)
- Banner (176/177, patterns complexes)
- Painting (entité, pas bloc)
- Item Frame (entité)
- Armor Stand (entité)
- Sign (63/68)
- Flower Pot (140)
- Mob Head (144, 5 types)
- Cake (92)

FLUIDES :
- Water (8/9, 8 niveaux)
- Lava (10/11, 4 niveaux — plus lent que l'eau)

TECHNIQUE :
- Piston Head (34)
- Moving Piston (36)
- Fire (51)
- Portal (90, Nether)
- End Portal (119)
- End Portal Frame (120)
- Dragon Egg (122)
- Barrier (166, invisible, creative only)
- Command Block : EXCLU
```

---

## 19. REGISTRE COMPLET DES ITEMS

```
OUTILS :
- Wooden/Stone/Iron/Gold/Diamond Sword
- Wooden/Stone/Iron/Gold/Diamond Pickaxe
- Wooden/Stone/Iron/Gold/Diamond Axe
- Wooden/Stone/Iron/Gold/Diamond Shovel
- Wooden/Stone/Iron/Gold/Diamond Hoe
- Fishing Rod
- Flint and Steel
- Shears
- Bow
- Carrot on a Stick
- Lead
- Name Tag
- Clock
- Compass
- Map + Empty Map

ARMURE :
- Leather/Chainmail/Iron/Gold/Diamond Helmet
- Leather/Chainmail/Iron/Gold/Diamond Chestplate
- Leather/Chainmail/Iron/Gold/Diamond Leggings
- Leather/Chainmail/Iron/Gold/Diamond Boots
- Horse Armor (Iron/Gold/Diamond)
- Saddle

MATÉRIAUX :
- Stick, Coal, Charcoal
- Iron Ingot, Gold Ingot, Diamond, Emerald
- Iron Nugget : Non, 1.11. EXCLU. Gold Nugget : oui.
- Flint, Feather, Leather, String, Bone
- Redstone Dust, Lapis Lazuli, Quartz
- Nether Star, Ghast Tear, Blaze Rod, Blaze Powder
- Ender Pearl, Eye of Ender
- Gunpowder, Slimeball, Magma Cream
- Clay Ball, Brick, Nether Brick (item)
- Prismarine Shard, Prismarine Crystal
- Rabbit Hide, Rabbit Foot
- Ink Sac, Dyes (16 couleurs)
- Book, Enchanted Book
- Paper, Sugar, Bowl
- Glass Bottle, Water Bottle
- Bucket (empty, water, lava, milk)
- Snowball, Egg

NOURRITURE :
(Voir section Faim pour les valeurs détaillées)
- Apple, Golden Apple, Enchanted Golden Apple
- Bread, Cookie, Cake
- Raw/Cooked Beef, Porkchop, Chicken, Mutton, Rabbit
- Raw/Cooked Fish, Salmon, Clownfish, Pufferfish
- Carrot, Golden Carrot, Potato, Baked Potato, Poisonous Potato
- Melon Slice, Pumpkin Pie
- Mushroom Stew, Rabbit Stew, Beetroot Soup (non, 1.9)
- Rotten Flesh, Spider Eye, Fermented Spider Eye
- Glistering Melon (non-comestible, brewing ingrédient)

POTIONS :
- Water Bottle
- Awkward Potion, Thick Potion, Mundane Potion
- Toutes les potions normales + extended + level II
- Splash variants de chaque potion

MUSIQUE :
- Music Disc (13, cat, blocks, chirp, far, mall, mellohi, stal, strad, ward, wait, 11)

DIVERS :
- Bone Meal (fertilisant)
- Fire Charge
- Firework Rocket + Firework Star
- Book and Quill + Written Book
- Banner (item)
- Armor Stand (item)
- Spawn Eggs (creative only, un par mob)
```

---

## 20. REGISTRE COMPLET DES RECETTES DE CRAFT

```
Ce registre contient TOUTES les recettes shaped et shapeless.
Format : [résultat] = pattern (où - = vide, X = matériau)

=== BLOCS BASIQUES ===
[4 Planks] = 1 Log (any type, shapeless, type matches)
[4 Sticks] = 2 Planks vertical
[1 Crafting Table] = 4 Planks (2×2)
[1 Chest] = 8 Planks (anneau 3×3)
[1 Furnace] = 8 Cobblestone (anneau 3×3)
[1 Torch ×4] = Coal/Charcoal + Stick vertical
[1 Ladder ×3] = Sticks en H (colonnes gauche+droite + rangée milieu)

=== OUTILS ===
[Wooden Pickaxe] = PPP / -S- / -S- (P=Plank, S=Stick)
[Stone Pickaxe] = CCC / -S- / -S- (C=Cobblestone)
[Iron Pickaxe] = III / -S- / -S- (I=Iron Ingot)
[Gold Pickaxe] = GGG / -S- / -S- (G=Gold Ingot)
[Diamond Pickaxe] = DDD / -S- / -S- (D=Diamond)

(Même pattern pour chaque tier, même pattern axe/pelle/houe/épée adaptés)

[Axe] = XX- / XS- / -S- (X=material)
[Shovel] = -X- / -S- / -S-
[Hoe] = XX- / -S- / -S-
[Sword] = -X- / -X- / -S-

[Bow] = -Sk / S-k / -Sk (Sk=Stick, k=String) mirrorable
[Arrow ×4] = Flint + Stick + Feather vertical
[Fishing Rod] = --S / -S# / S-# (S=Stick, #=String)
[Shears] = -I / I- (I=Iron Ingot)
[Flint and Steel] = I- / -F (I=Iron, F=Flint)

=== ARMURE ===
[Helmet] = XXX / X-X
[Chestplate] = X-X / XXX / XXX
[Leggings] = XXX / X-X / X-X
[Boots] = X-X / X-X

=== NOURRITURE ===
[Bread] = WWW (W=Wheat)
[Cake] = MMM / SES / WWW (M=Milk Bucket, S=Sugar, E=Egg, W=Wheat)
[Cookie ×8] = WCW (W=Wheat, C=Cocoa Beans)
[Pumpkin Pie] = Pumpkin + Sugar + Egg (shapeless)
[Golden Apple] = GGG / GAG / GGG (G=Gold Ingot, A=Apple)
[Enchanted G.Apple] = GGG / GAG / GGG (G=Gold BLOCK) — Note: recette supprimée en 1.9, mais PRÉSENTE en 1.8!
[Mushroom Stew] = RM / B (R=Red Mushroom, M=Brown Mushroom, B=Bowl, shapeless)
[Rabbit Stew] = -R- / CPM / -B- (R=Cooked Rabbit, C=Carrot, P=Baked Potato, M=Mushroom, B=Bowl)
[Fermented Spider Eye] = Spider Eye + Sugar + Brown Mushroom (shapeless)
[Golden Carrot] = GGG / GCG / GGG (G=Gold Nugget, C=Carrot)
[Glistering Melon] = GGG / GMG / GGG (G=Gold Nugget, M=Melon)

=== TRANSPORT ===
[Boat] = P-P / PPP (P=Plank)
[Minecart] = I-I / III (I=Iron Ingot)
[Chest Minecart] = Chest + Minecart (shapeless)
[Hopper Minecart] = Hopper + Minecart (shapeless)
[TNT Minecart] = TNT + Minecart (shapeless)
[Furnace Minecart] = Furnace + Minecart (shapeless)
[Rail ×16] = I-I / ISI / I-I (I=Iron, S=Stick)
[Powered Rail ×6] = G-G / GSG / GRG (G=Gold, S=Stick, R=Redstone)
[Detector Rail ×6] = I-I / ISI / IRI
[Activator Rail ×6] = ISI / IRI / ISI (I=Iron, S=Stick, R=Redstone Torch)
[Carrot on Stick] = Fishing Rod + Carrot (shapeless)
[Saddle] = Non craftable (loot uniquement)
[Lead ×2] = SS- / SR- / --S (S=String, R=Slimeball)
[Name Tag] = Non craftable (loot uniquement)

=== REDSTONE ===
[Redstone Torch] = R / S (R=Redstone, S=Stick)
[Repeater] = TRT / SSS (T=Redstone Torch, R=Redstone, S=Stone)
[Comparator] = -T- / TQT / SSS (T=Redstone Torch, Q=Quartz, S=Stone)
[Piston] = PPP / CIC / CRC (P=Plank, C=Cobble, I=Iron, R=Redstone)
[Sticky Piston] = Slimeball + Piston vertical
[Dispenser] = CCC / CBC / CRC (C=Cobble, B=Bow, R=Redstone)
[Dropper] = CCC / C-C / CRC
[Hopper] = I-I / ICI / -I- (I=Iron, C=Chest)
[Observer] : NON, 1.11. EXCLU.
[Daylight Sensor] = GGG / QQQ / SSS (G=Glass, Q=Quartz, S=Wood Slab)
[Tripwire Hook ×2] = I / S / P (I=Iron, S=Stick, P=Plank)
[Trapped Chest] = Tripwire Hook + Chest (shapeless)
[Note Block] = PPP / PRP / PPP (P=Plank, R=Redstone)
[Redstone Lamp] = -R- / RGR / -R- (R=Redstone, G=Glowstone)
[TNT] = G S G / S G S / G S G (G=Gunpowder, S=Sand)
[Lever] = S / C (S=Stick, C=Cobblestone)
[Stone Button] = S (1 Stone)
[Wood Button] = P (1 Plank)
[Stone Pressure Plate] = SS (2 Stone)
[Wood Pressure Plate] = PP (2 Planks)
[Iron/Gold Pressure Plate] : Weighted (I: II, G: GG)

=== ENCHANTEMENT & BREWING ===
[Enchanting Table] = -B- / DED / OOO (B=Book, D=Diamond, E=Emerald... NON : D=Diamond, O=Obsidian)
  CORRECTION : [Enchanting Table] = -B- / DOD / OOO (B=Book, D=Diamond, O=Obsidian)
[Bookshelf] = PPP / BBB / PPP (P=Plank, B=Book)
[Book] = Paper Paper / Paper Leather (shapeless)
[Paper ×3] = SSS (S=Sugar Cane)
[Anvil] = III / -i- / iii (I=Iron Block, i=Iron Ingot)
[Brewing Stand] = -B- / CCC (B=Blaze Rod, C=Cobblestone)
[Cauldron] = I-I / I-I / III (I=Iron Ingot)
[Glass Bottle ×3] = G-G / -G- (G=Glass)
[Blaze Powder ×2] = Blaze Rod (shapeless)
[Eye of Ender] = Blaze Powder + Ender Pearl (shapeless)
[Magma Cream] = Blaze Powder + Slimeball (shapeless)
[Fire Charge ×3] = Gunpowder + Blaze Powder + Coal/Charcoal (shapeless)

=== BLOCS DE STOCKAGE ===
[Iron Block] = 9 Iron Ingots (3×3)
[Gold Block] = 9 Gold Ingots
[Diamond Block] = 9 Diamonds
[Emerald Block] = 9 Emeralds
[Lapis Block] = 9 Lapis Lazuli
[Redstone Block] = 9 Redstone
[Coal Block] = 9 Coal
[Quartz Block] = 4 Quartz (2×2)
[Slime Block] = 9 Slimeballs
[Hay Bale] = 9 Wheat (3×3)

(Reverse : chaque bloc → 9 items, sauf Quartz Block et Hay Bale)
[9 Iron Ingots] = Iron Block (shapeless)
[Gold Nugget ×9] = Gold Ingot (shapeless)
[Gold Ingot] = 9 Gold Nuggets (3×3)

=== DÉCORATION ===
[Painting] = SSS / SWS / SSS (S=Stick, W=Wool)
[Item Frame] = SSS / SLS / SSS (S=Stick, L=Leather)
[Sign ×3] = PPP / PPP / -S- (P=Plank, S=Stick)
[Flower Pot] = B-B / -B- (B=Brick)
[Armor Stand] = SSS / -S- / SsS (S=Stick, s=Stone Slab)
[Carpet ×3] = WW (2 Wool, horizontal)
[Banner] = WWW / WWW / -S- (W=Wool, S=Stick)
[Stained Glass ×8] = GGG / GDG / GGG (G=Glass, D=Dye)
[Stained Clay ×8] = CCC / CDC / CCC (C=Hardened Clay, D=Dye)

=== ESCALIERS & DALLES ===
[Stairs ×4] = X-- / XX- / XXX (X=material)
[Slabs ×6] = XXX (X=material, rangée du bas)

=== PORTES & ACCÈS ===
[Door ×3] = PP / PP / PP (P=Plank or Iron)
[Trapdoor ×2] = PPP / PPP (P=Plank)
[Iron Trapdoor ×1] = II / II (I=Iron Ingot)
[Fence ×3] = PSP / PSP (P=Plank, S=Stick)
[Fence Gate] = SPS / SPS (S=Stick, P=Plank)
[Nether Brick Fence ×6] = NNN / NNN (N=Nether Brick)
[Cobblestone Wall ×6] = CCC / CCC (C=Cobblestone)
[Ladder ×3] = S-S / SSS / S-S (S=Stick)

=== TEINTURES (Dyes) ===
(16 couleurs, certaines craftées, d'autres par smelting ou drops)
- Bone → Bone Meal (white dye, shapeless)
- Ink Sac = squid drop (black)
- Cactus → Cactus Green (smelt)
- Rose → Rose Red (shapeless)
- Lapis = lapis ore (blue)
- Combinaisons : ex. Red + Yellow = Orange, Blue + White = Light Blue, etc.
- Cocoa Beans = jungle (brown)

=== BLOCS PRISMARINE ===
[Prismarine] = SS / SS (S=Prismarine Shard, 2×2)
[Prismarine Bricks] = SSS / SSS / SSS (S=Prismarine Shard)
[Dark Prismarine] = SSS / SDS / SSS (S=Prismarine Shard, D=Ink Sac)
[Sea Lantern] = SCS / CCC / SCS (S=Prismarine Shard, C=Prismarine Crystal)

=== FIREWORKS ===
[Firework Rocket] = Paper + Gunpowder (shapeless, 1-3 gunpowder = flight duration)
[Firework Star] = Gunpowder + Dye + optional (diamond=trail, glowstone=twinkle, fire charge=large ball, gold nugget=star, head=creeper face)

=== BEACON ===
[Beacon] = GGG / GSG / OOO (G=Glass, S=Nether Star, O=Obsidian)

Beacon pyramides :
- Tier 1 : 9 blocks (3×3) = Speed OR Haste
- Tier 2 : 34 blocks (5×5 + 3×3) = + Resistance OR Jump Boost
- Tier 3 : 83 blocks (7×7 + 5×5 + 3×3) = + Strength
- Tier 4 : 164 blocks (9×9 + 7×7 + 5×5 + 3×3) = + Regeneration (secondaire)
- Matériaux pyramide : Iron, Gold, Diamond, Emerald blocks

=== MAP & LIVRE ===
[Map] = PPP / PCP / PPP (P=Paper, C=Compass)
[Compass] = -I- / IRI / -I- (I=Iron, R=Redstone)
[Clock] = -G- / GRG / -G- (G=Gold, R=Redstone)
[Book and Quill] = Book + Ink Sac + Feather (shapeless)
```

---

## 21. REGISTRE COMPLET DES MOBS

(Voir section 8.5 pour le détail de chaque mob. Résumé ici.)

```
PASSIFS : Cow, Pig, Sheep, Chicken, Squid, Bat, Horse/Donkey/Mule, Rabbit, Ocelot/Cat, Mooshroom, Villager, Iron Golem (spawné), Snow Golem (craftable)
NEUTRES : Wolf, Zombie Pigman, Enderman, Spider (jour)
HOSTILES : Zombie, Skeleton, Creeper, Spider (nuit), Cave Spider, Witch, Slime, Silverfish, Endermite, Guardian, Elder Guardian, Ghast, Blaze, Wither Skeleton, Magma Cube
BOSS : Ender Dragon, Wither

GOLEMS :
- Iron Golem : 4 Iron Block + 1 Pumpkin (T shape + pumpkin head)
  - HP : 100, Dégâts : 7-21
  - Protège les villagers, attaque les hostiles
  - Drop : 3-5 Iron Ingot, 0-2 Poppy

- Snow Golem : 2 Snow Block + 1 Pumpkin (vertical)
  - HP : 4, Dégâts : 0 (knockback only, snowball)
  - Laisse une trace de neige
  - Meurt dans les biomes chauds et au contact de l'eau

Total : ~40 types d'entités vivantes
```

---

## 22. REGISTRE COMPLET DES BIOMES

```
| ID | Biome | Température | Pluie | Surface | Particularités |
|----|-------|-------------|-------|---------|----------------|
| 0 | Ocean | 0.5 | 0.5 | Gravel | Profond, monuments |
| 1 | Plains | 0.8 | 0.4 | Grass | Villages, chevaux |
| 2 | Desert | 2.0 | 0.0 | Sand | Temples, villages, puits |
| 3 | Extreme Hills | 0.2 | 0.3 | Grass/Stone | Emeralds, silverfish |
| 4 | Forest | 0.7 | 0.8 | Grass | Oak + Birch |
| 5 | Taiga | 0.25 | 0.8 | Grass | Spruce, loups |
| 6 | Swampland | 0.8 | 0.9 | Grass | Witch huts, slimes, lianes |
| 7 | River | 0.5 | 0.5 | Sand/Clay | Connecte biomes |
| 8 | Nether | 2.0 | 0.0 | Netherrack | Dimension séparée |
| 9 | End | 0.5 | 0.5 | End Stone | Dimension séparée |
| 10 | Frozen Ocean | 0.0 | 0.5 | Gravel+Ice | Glace en surface |
| 11 | Frozen River | 0.0 | 0.5 | Ice | |
| 12 | Ice Plains | 0.0 | 0.5 | Snow+Grass | Igloos (1.9, EXCLU) |
| 13 | Ice Mountains | 0.0 | 0.5 | Snow | |
| 14 | Mushroom Island | 0.9 | 1.0 | Mycelium | Mooshrooms, no hostile |
| 15 | Mushroom Shore | 0.9 | 1.0 | Mycelium | |
| 16 | Beach | 0.8 | 0.4 | Sand | |
| 17 | Desert Hills | 2.0 | 0.0 | Sand | |
| 18 | Forest Hills | 0.7 | 0.8 | Grass | |
| 19 | Taiga Hills | 0.25 | 0.8 | Grass | |
| 20 | Extreme Hills Edge | 0.2 | 0.3 | Grass | Transition |
| 21 | Jungle | 0.95 | 0.9 | Grass | Temples, melons |
| 22 | Jungle Hills | 0.95 | 0.9 | Grass | |
| 23 | Jungle Edge | 0.95 | 0.8 | Grass | |
| 24 | Deep Ocean | 0.5 | 0.5 | Gravel | Monuments |
| 25 | Stone Beach | 0.2 | 0.3 | Stone | Falaises |
| 26 | Cold Beach | 0.05 | 0.3 | Sand+Snow | |
| 27 | Birch Forest | 0.6 | 0.6 | Grass | Birch only |
| 28 | Birch Forest Hills | 0.6 | 0.6 | Grass | |
| 29 | Roofed Forest | 0.7 | 0.8 | Grass | Dark Oak, champignons |
| 30 | Cold Taiga | -0.5 | 0.4 | Snow | |
| 31 | Cold Taiga Hills | -0.5 | 0.4 | Snow | |
| 32 | Mega Taiga | 0.3 | 0.8 | Grass/Podzol | Mega Spruce 2×2 |
| 33 | Mega Taiga Hills | 0.3 | 0.8 | Grass/Podzol | |
| 34 | Extreme Hills+ | 0.2 | 0.3 | Grass/Gravel | Plus hauts |
| 35 | Savanna | 1.2 | 0.0 | Grass | Acacia, villages |
| 36 | Savanna Plateau | 1.0 | 0.0 | Grass | Plat en altitude |
| 37 | Mesa | 2.0 | 0.0 | Red Sand/Terracotta | Bandes colorées |
| 38 | Mesa Plateau F | 2.0 | 0.0 | Terracotta | Avec forêt sur plateau |
| 39 | Mesa Plateau | 2.0 | 0.0 | Terracotta | |

+ variantes "M" (mutated) rares pour plusieurs biomes
```

---

## 23. REGISTRE COMPLET DES ENCHANTEMENTS

(Détaillé en section 11.2, résumé avec les IDs ici)

```
| ID | Nom | Max Level | Applicable à |
|----|-----|-----------|-------------|
| 0 | Protection | IV | Armure |
| 1 | Fire Protection | IV | Armure |
| 2 | Feather Falling | IV | Bottes |
| 3 | Blast Protection | IV | Armure |
| 4 | Projectile Protection | IV | Armure |
| 5 | Respiration | III | Casque |
| 6 | Aqua Affinity | I | Casque |
| 7 | Thorns | III | Plastron |
| 8 | Depth Strider | III | Bottes |
| 16 | Sharpness | V | Épée |
| 17 | Smite | V | Épée |
| 18 | Bane of Arthropods | V | Épée |
| 19 | Knockback | II | Épée |
| 20 | Fire Aspect | II | Épée |
| 21 | Looting | III | Épée |
| 32 | Efficiency | V | Outils |
| 33 | Silk Touch | I | Outils |
| 34 | Unbreaking | III | Tout |
| 35 | Fortune | III | Outils |
| 48 | Power | V | Arc |
| 49 | Punch | II | Arc |
| 50 | Flame | I | Arc |
| 51 | Infinity | I | Arc |
| 61 | Luck of the Sea | III | Canne |
| 62 | Lure | III | Canne |

Incompatibilités :
- Silk Touch vs Fortune
- Sharpness vs Smite vs Bane of Arthropods
- Protection vs Fire/Blast/Projectile Protection (mutuellement exclusifs en 1.8)
- Infinity vs ... (pas de restriction en 1.8, Infinity+Mending conflict = 1.9+)
```

---

## 24. REGISTRE COMPLET DES POTIONS

(Voir section 11.4 pour les recettes de brewing)

```
| Potion | ID | Durée std | Durée ext | Niveau II |
|--------|----|-----------|-----------|-----------|
| Speed | speed | 3:00 | 8:00 | 1:30 |
| Slowness | slowness | 1:30 | 4:00 | - |
| Strength | strength | 3:00 | 8:00 | 1:30 |
| Weakness | weakness | 1:30 | 4:00 | - |
| Healing | healing | instant | - | instant |
| Harming | harming | instant | - | instant |
| Regeneration | regeneration | 0:45 | 2:00 | 0:22 |
| Poison | poison | 0:45 | 2:00 | 0:22 |
| Fire Resistance | fire_resistance | 3:00 | 8:00 | - |
| Night Vision | night_vision | 3:00 | 8:00 | - |
| Invisibility | invisibility | 3:00 | 8:00 | - |
| Water Breathing | water_breathing | 3:00 | 8:00 | - |
| Leaping | leaping | 3:00 | 8:00 | 1:30 |

Chaque potion existe en : Normal, Extended (+Redstone), Upgraded (+Glowstone), Splash (+Gunpowder)
= ~50+ variantes au total
```

---

## 25. REGISTRE COMPLET DES STRUCTURES

(Résumé section 12.1, ajout de détails de génération)

```
| Structure | Biome | Fréquence | Taille | Composants clés |
|-----------|-------|-----------|--------|-----------------|
| Village | Plains, Desert, Savanna | 1/32 chunks | 5-25 bâtiments | Forge, église, ferme |
| Desert Temple | Desert | 1/64 chunks | 21×21 | 4 coffres, TNT trap |
| Jungle Temple | Jungle | 1/64 chunks | 12×15 | 2 coffres, pièges flèches |
| Witch Hut | Swamp | 1/48 chunks | 7×9 | 1 witch |
| Ocean Monument | Deep Ocean | 1/32 chunks | 58×58 | 3 Elder Guardians |
| Dungeon | Partout (underground) | 8/chunk | 5-7×5-7 | Spawner, coffres |
| Mineshaft | Partout (underground) | | Variable | Rails, spawners cave spider |
| Stronghold | Partout (deep) | 3 par monde | Variable | End Portal |
| Nether Fortress | Nether | | Variable | Blazes, Wither Skeletons |
| Desert Well | Desert | Rare | 5×5 | Sandstone + eau |
```

---

## 26. SPÉCIFICATIONS DE LA GÉNÉRATION DE MONDE

```
Ordre de génération d'un chunk :

1. Biome map : déterminer le biome de chaque colonne 1×1 dans le chunk
2. Base terrain : heightmap via noise, remplissage stone/dirt/sand
3. Bedrock layer : Y=0 solide, Y 1-4 aléatoire
4. Water : remplir air sous Y=63 avec eau (si dans un ocean/river/lac)
5. Caves : Perlin worm carving
6. Ravines : grands carving (rare)
7. Ore generation : placer tous les filons de minerai
8. Surface decoration : herbe, fleurs, arbres (dépend du biome)
9. Structures : vérifier si une structure doit commencer/continuer ici
10. Snow & Ice : placer neige/glace dans les biomes froids (au-dessus de Y seuil)
11. Lighting : calculer le skylight et blocklight
12. Heightmap : calculer la colonne la plus haute (pour le skylight rapide)

Seed consistency :
- Chaque étape utilise le seed du monde + position chunk
- Deux mondes avec le même seed produisent le même résultat
- Les structures utilisent un grid-based check avec le seed
```

---

## 27. SPÉCIFICATIONS TECHNIQUES DU RENDU

```
=== SHADERS ===

block.vert :
- Input : position (vec3), UV (vec2), normal (int), light (int), AO (float)
- Uniform : model-view-projection matrix
- Output : fragment position, UV, light, AO, fog factor

block.frag :
- Input : UV, light, AO, fog factor
- Uniform : texture atlas sampler, sky color, time
- Output : final color = texture * light * AO, blended with fog

water.frag :
- Comme block.frag mais avec :
  - Transparence (alpha = 0.6)
  - Léger tint bleu
  - Animation UV (scroll les UVs lentement)

entity.vert / entity.frag :
- Modèles 3D avec texture skin
- Joints animés (rotation par os)
- Éclairage basé sur la position dans le monde

sky.vert / sky.frag :
- Dome coloré avec gradient
- Soleil/lune comme quads
- Étoiles comme points

particle.vert / particle.frag :
- Billboard particles
- Texture atlas particules
- Fade out sur lifetime

ui.vert / ui.frag :
- Orthographic projection
- Texture GUI elements
- Text rendering

=== RENDER PASSES ===
1. Sky pass (désactiver depth write)
2. Opaque blocks pass (depth test + write, face culling)
3. Entity pass (depth test, alpha test pour les parties transparentes)
4. Transparent blocks pass (depth test, pas depth write, alpha blending, tri par distance)
5. Particle pass (depth test, pas depth write, additive ou alpha blending)
6. UI pass (pas de depth test, orthographic)

=== OPTIMISATIONS ===
- Frustum culling (skip chunks hors vue)
- Face culling (pas de mesh pour les faces cachées entre blocs)
- Greedy meshing (réduire le nombre de quads)
- Chunk-level LOD (optionnel : simplifier les chunks lointains)
- VBO batching (un VBO par chunk, ou grouper les chunks)
- Texture atlas (1 seul bind pour tous les blocs)
- Occlusion culling (optionnel : skip chunks derrière des montagnes)
- Multi-threaded chunk generation et meshing
```

---

## 28. ROADMAP & MILESTONES

### Phase 0 — Fondations (Semaines 1-3)
```
□ Setup projet CMake + dépendances (GLFW, GLAD, GLM, stb)
□ Fenêtre + contexte OpenGL + input
□ Caméra FPS (mouvement, rotation souris)
□ Rendu d'un cube texturé
□ Texture atlas basique (10 blocs)
□ Chunk data structure (16×256×16)
□ Mesh builder simple (toutes faces, pas de culling)
□ Rendu d'un chunk unique
✓ MILESTONE : Se déplacer dans un chunk de stone/dirt/grass
```

### Phase 1 — Monde de base (Semaines 4-6)
```
□ Face culling (ne pas mesh les faces entre blocs adjacents)
□ Greedy meshing
□ Multi-chunk (loading/unloading, render distance)
□ Frustum culling
□ Perlin noise (heightmap basique)
□ Biome map simple (3-4 biomes)
□ Génération de caves basique
□ Minerais
□ Arbres (oak + birch)
□ Skybox + cycle jour/nuit
□ Brouillard
✓ MILESTONE : Explorer un monde infini avec terrain réaliste
```

### Phase 2 — Joueur & Blocs (Semaines 7-9)
```
□ Physique joueur (gravité, collision, saut, sprint)
□ Raycasting (cibler un bloc)
□ Block breaking (avec animation craquelure)
□ Block placing
□ Éclairage (skylight + blocklight, propagation)
□ Ambient Occlusion
□ Eau (rendu semi-transparent + physique fluide spread)
□ Lave
□ Sable/Gravier (gravité)
□ 50+ types de blocs avec propriétés
□ Blocs non-cubiques (escaliers, dalles, torches, portes)
✓ MILESTONE : Casser/poser des blocs, lumière dynamique, eau qui coule
```

### Phase 3 — Survie de base (Semaines 10-12)
```
□ Inventaire joueur (36 slots + armure)
□ HUD (cœurs, faim, hotbar, XP bar)
□ Système de crafting (2×2 + table 3×3)
□ Toutes les recettes de base (outils, blocs)
□ Durabilité des outils
□ Fourneau (smelting)
□ Système de faim + nourriture
□ Santé + dégâts (chute, feu, noyade)
□ Drops d'items au sol
□ XP orbs
□ Coffres
□ Lit (skip nuit, set spawn)
✓ MILESTONE : Boucle de survie complète (récolter, crafter, manger, dormir)
```

### Phase 4 — Mobs (Semaines 13-17)
```
□ Système d'entités
□ Modèles de mobs (cuboids animés)
□ Pathfinding A*
□ Système IA (goals + tasks)
□ Mobs passifs : Cow, Pig, Sheep, Chicken
□ Spawn rules (passifs)
□ Mob drops + loot
□ Mobs hostiles : Zombie, Skeleton, Creeper, Spider
□ Spawn rules (hostiles, light-based)
□ Combat basique (hit, knockback, invuln frames)
□ Brûlage au soleil (zombie, skeleton)
□ Enderman (complet avec téléportation)
□ Tous les mobs restants (voir registre section 21)
□ Iron Golem + Snow Golem
□ Breeding
□ Taming (wolf, ocelot, horse)
□ Villager + trading
✓ MILESTONE : Monde vivant avec tous les mobs, combat fonctionnel
```

### Phase 5 — Redstone & Mécanismes (Semaines 18-20)
```
□ Redstone wire (propagation 0-15)
□ Redstone torch (inverseur)
□ Lever, button, pressure plate
□ Repeater (délai, lock)
□ Comparator (compare, subtract)
□ Piston + sticky piston
□ Dispenser + dropper
□ Hopper
□ Redstone lamp
□ TNT (explosion complète)
□ Note block
□ Tripwire
□ Daylight sensor
□ Rails (normal, powered, detector, activator)
□ Minecarts + variantes
□ Portes, trapdoors, fence gates (redstone-compatible)
✓ MILESTONE : Circuits redstone fonctionnels, pistons, TNT
```

### Phase 6 — Enchantement & Brewing (Semaines 21-22)
```
□ Table d'enchantement (bookshelves, UI)
□ Système d'enchantement (randomisation, niveaux)
□ Tous les enchantements
□ Enclume (combinaison, renommage)
□ Brewing stand (UI + toutes les recettes)
□ Toutes les potions (normales + splash)
□ Effets de status (particules, timer)
□ Beacon (pyramide, effets)
□ Ender Chest (inventaire partagé)
□ Jukebox + music discs
✓ MILESTONE : Enchanter des items, brewer des potions, beacon fonctionnel
```

### Phase 7 — Structures & Monde complet (Semaines 23-25)
```
□ Villages (5 variantes, villagers, forge)
□ Desert Temple + Jungle Temple
□ Witch Hut
□ Dungeons (spawners)
□ Mineshafts
□ Strongholds (+ End Portal)
□ Ocean Monuments (+ Guardians)
□ Tous les biomes restants
□ Décoration complète par biome
□ Météo (pluie, orage, neige, lightning)
□ Eye of Ender (localisation stronghold)
□ Loot tables pour tous les coffres
✓ MILESTONE : Monde complet avec toutes les structures, prêt pour le Nether
```

### Phase 8 — Nether (Semaine 26)
```
□ Portail du Nether (construction, activation, téléportation)
□ Génération du Nether (terrain, lave, netherrack)
□ Nether Fortress
□ Mobs Nether (Zombie Pigman, Ghast, Blaze, Wither Skeleton, Magma Cube)
□ Nether Wart farming
□ Glowstone, Quartz mining
□ Wither boss (invocation, combat)
✓ MILESTONE : Nether complet, Wither tuable, Blaze rods récoltables
```

### Phase 9 — The End & Dragon (Semaines 27-28)
```
□ End Portal activation (12 Eyes of Ender)
□ Génération de l'End (île, tours d'obsidian)
□ End Crystals (heal beam, explosion)
□ Ender Dragon AI complète (holding, strafing, charging, perching)
□ Combat complet (détruire crystals, frapper dragon)
□ Mort du Dragon (animation, XP, portail retour)
□ Dragon Egg
□ Poème de fin (texte original + crédits)
□ Retour au spawn après le poème
✓ MILESTONE : GAME COMPLETE — Du spawn au dragon kill
```

### Phase 10 — Audio & Polish (Semaines 29-31)
```
□ Intégration OpenAL
□ Sons de blocs (break, place, step)
□ Sons de mobs (idle, hurt, death)
□ Sons d'ambiance (cave, nether, pluie)
□ Musique originale (5-10 tracks)
□ Particules complètes
□ Animations de mobs polish
□ Transitions d'écran (fade)
□ Splash texts (menu principal)
□ Panorama fond menu
✓ MILESTONE : Expérience audio-visuelle complète
```

### Phase 11 — UI & Commandes (Semaine 32)
```
□ Menu principal complet
□ Écran sélection/création de monde
□ Options complètes (video, audio, controls)
□ F3 debug screen
□ Chat + commandes (/gamemode, /tp, /give, /time, /weather, etc.)
□ Écran de mort
□ Toutes les GUIs (coffre, fourneau, enchanting, anvil, brewing, etc.)
□ Inventaire créatif (tabs, search)
□ Recette book : NON, 1.12+. EXCLU.
✓ MILESTONE : Interface utilisateur complète et polie
```

### Phase 12 — Sauvegarde & Release (Semaines 33-34)
```
□ Sauvegarde monde (region files)
□ Chargement monde
□ Autosave
□ Gestion multiple mondes
□ Optimisation performance (profiling, bottlenecks)
□ Memory leaks fix
□ Build release (CMake → .exe)
□ Packaging (dossier distributable)
□ Test complet : nouveau monde → kill dragon
□ README + instructions
✓ MILESTONE : v1.0 RELEASE — .exe jouable et complet
```

---

## ANNEXE A — CONSTANTES IMPORTANTES

```
TICKS_PER_SECOND = 20
TICK_DURATION_MS = 50
DAY_LENGTH_TICKS = 24000
CHUNK_WIDTH = 16
CHUNK_HEIGHT = 256
CHUNK_DEPTH = 16
SEA_LEVEL = 63
BEDROCK_MAX_Y = 4
MAX_BUILD_HEIGHT = 256
PLAYER_REACH = 5.0
PLAYER_HEIGHT = 1.8
PLAYER_WIDTH = 0.6
PLAYER_EYE_HEIGHT = 1.62
WALK_SPEED = 4.317
SPRINT_SPEED = 5.612
GRAVITY = 0.08  // blocs/tick²
TERMINAL_VELOCITY = 3.92 // blocs/tick
MAX_HEALTH = 20
MAX_HUNGER = 20
MAX_LIGHT_LEVEL = 15
RANDOM_TICK_SPEED = 3
MAX_STACK_SIZE = 64
TOOL_STACK_SIZE = 1
ENDER_PEARL_STACK = 16
SNOWBALL_STACK = 16
SIGN_STACK = 16
EGG_STACK = 16
```

## ANNEXE B — KEYBINDINGS PAR DÉFAUT

```
| Touche | Action |
|--------|--------|
| W/Z | Avancer |
| S | Reculer |
| A/Q | Gauche |
| D | Droite |
| Space | Sauter / Nager vers le haut |
| Shift | S'accroupir |
| Ctrl | Sprinter (double W aussi) |
| E | Ouvrir inventaire |
| Escape | Menu pause |
| 1-9 | Sélectionner hotbar |
| Scroll | Changer slot hotbar |
| LMB | Attaquer / Casser bloc |
| RMB | Utiliser / Poser bloc |
| MMB | Pick block (creative) |
| Q | Drop item |
| F | Swap main hand : NON, 1.9. EXCLU |
| T | Ouvrir chat |
| / | Ouvrir chat avec / |
| F1 | Toggle HUD |
| F2 | Screenshot |
| F3 | Debug screen |
| F5 | Toggle 3ème personne (1st → 3rd back → 3rd front) |
| F11 | Fullscreen |
| Tab | Player list : NON, singleplayer. EXCLU. |
```

## ANNEXE C — FORMULES UTILES

```
// Temps de minage d'un bloc
float getMiningTime(Block block, Tool tool, Player player) {
    float base = block.hardness;
    if (base < 0) return INFINITY; // Bedrock, etc.
    
    float speedMultiplier = 1.0;
    if (tool.type == block.preferredTool) {
        speedMultiplier = tool.materialSpeed; // wood=2, stone=4, iron=6, diamond=8, gold=12
        if (tool.hasEfficiency) speedMultiplier += efficiency_level² + 1;
    }
    
    if (player.isInWater && !player.hasAquaAffinity) speedMultiplier /= 5.0;
    if (!player.isOnGround) speedMultiplier /= 5.0;
    
    float damage = speedMultiplier / base;
    if (canHarvest(tool, block)) {
        damage /= 30.0;
    } else {
        damage /= 100.0;
    }
    
    // Instant break check
    if (damage >= 1.0) return 0; // 1 tick
    
    return ceil(1.0 / damage); // ticks
}

// Explosion ray
void explode(vec3 center, float power) {
    for (ray in 1352_uniform_directions) {
        float intensity = power * (0.7 + random() * 0.6);
        vec3 pos = center;
        while (intensity > 0) {
            Block block = getBlock(pos);
            intensity -= (block.blastResistance / 5.0 + 0.3) * 0.3;
            if (intensity > 0 && block != AIR) {
                destroyBlock(pos, random() < 1.0/power); // drop chance
            }
            pos += ray_direction * 0.3;
            intensity -= 0.225; // distance decay
        }
    }
    // Entity damage
    for (entity in getEntitiesInRadius(center, power * 2)) {
        float dist = distance(center, entity.pos);
        float exposure = calculateExposure(center, entity); // raycast visibility
        float impact = (1.0 - dist / (power * 2)) * exposure;
        entity.damage(impact * impact * 7 * power + 1);
        entity.knockback(directionFromCenter * impact);
    }
}

// Enchantment level generation
int getEnchantmentLevel(int slot, int bookshelves) {
    // slot = 0, 1, 2 (top, middle, bottom)
    int base = random(1, 8) + (bookshelves / 2) + random(0, bookshelves);
    float modifier = [0.5, 0.66, 1.0][slot]; // approx
    return max(1, round(base * modifier));
}
```

---

**FIN DU PRD — VoxelForge v1.0**

Ce document constitue la spécification complète pour reproduire l'expérience Minecraft 1.8.9 en singleplayer. Chaque système décrit est nécessaire pour atteindre l'objectif final : un joueur qui spawne, survit, explore les 3 dimensions, et tue l'Ender Dragon.

Estimation totale : ~34 semaines de développement intensif pour un développeur expérimenté.
Lines of code estimées : 50,000 - 80,000 lignes C++.

**Let's build.** 🏗️