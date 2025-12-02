# **THE RAVEN GAME - Complete Development Plan**

## **Game Overview**

**Title:** The Raven (working title)
**Genre:** First-person atmospheric exploration/narrative adventure
**Inspiration:** Edgar Allan Poe's "The Raven" + Sky: Children of the Light aesthetic
**Platform:** PC (raylib, no engine)
**Length:** 2-3 hours
**Art Style:** Mid-poly, gray-blue dawn aesthetic, atmospheric fog
**Core Loop:** Explore → Discover memories → Progress narrative → Unlock new areas

---

## **Technical Specifications**

**Engine:** Custom (raylib C/C++)
**Terrain:** Procedurally generated (infinite, already implemented)
**World Size:** 1500x1500 unit play area (750 radius from center)
**Player Speed:** 4 units/sec walk, 8 units/sec sprint
**Camera:** First-person
**Target Performance:** 60 FPS
**Architecture:** Simple procedural/OOP hybrid (NO ECS)

---

## **Narrative Structure**

### **Setup**
Player is the narrator from the poem—a grieving man who moved to an isolated cabin in the forest after losing his love, Lenore. It's been months, maybe years. He lives in perpetual dawn, trapped in the moment between night (despair) and day (hope).

### **Inciting Incident**
A raven arrives at his cabin, speaking only one word: "Nevermore." The raven leads him into the forest, forcing him to confront his grief through symbolic locations.

### **Journey**
Through 5 major areas, the player collects "Memory Embers"—fragments of his past with Lenore. Each area represents a stage of grief:
1. **Whispering Woods** - Denial (searching for echoes of her voice)
2. **Ashen Grove** - Anger (burned forest, destruction)
3. **Stone Circle** - Bargaining (ancient shrine, seeking answers)
4. **Pallid Shore** - Depression (boundary of the world, emptiness)
5. **Echoing Glade** - Acceptance (returning memories, peace)

### **Resolution**
Return to cabin. Final confrontation with raven. Player makes implicit choice through exploration:
- **Ending A (Acceptance):** Let go of Lenore's belongings, raven flies away, dawn breaks into full morning
- **Ending B (Obsession):** Keep everything, raven remains perched, trapped in eternal dawn

---

## **Core Gameplay Mechanics**

### **Movement**
- WASD walk (4 u/s)
- Shift sprint (8 u/s)
- Mouse look (smooth first-person camera)
- No jumping (grounded, contemplative)
- Footstep sounds (terrain-specific)

### **Interaction**
- E key to interact
- Prompt appears when near interactable objects
- Types of interactions:
  - Read notes/letters
  - Collect memory embers
  - Examine objects (triggers narration)
  - Open/close doors

### **Memory Embers**
- Glowing orange orbs scattered in each area
- 5-7 per major area (~30 total)
- Collecting one triggers:
  - Voiceover narration (memory of Lenore)
  - Visual effect (brief flash of color in gray world)
  - Journal entry unlocked
  - Quest progression

### **The Raven**
- Always visible somewhere in world
- Flies to next location when player completes objective
- Perches and watches player
- Says "Nevermore" when approached (text appears)
- Blocks paths player shouldn't go yet
- Acts as quest marker and antagonist/guide

### **Quest System**
- Simple linear progression
- Active quest shown in UI
- Example: "Follow the raven into the Whispering Woods"
- "Find 5 Memory Embers in the Ashen Grove"
- No quest log clutter—one objective at a time

### **Journal**
- ESC menu → Journal
- Contains:
  - Collected memories (readable text)
  - Map showing discovered areas
  - Poem excerpts unlocked per area

---

## **World Design**

### **Layout**
```
                    [FOG BOUNDARY - 750 radius]
                           |
              [Whispering Woods - North]
                   300 unit radius
                        ●
                        |
    [Stone Circle]  [CABIN]  [Ashen Grove]
      West 180r   ----●----    East 200r
                      |
              [Pallid Shore - South]
                   250 unit radius
                        ●
                        |
                [Echoing Glade - Southeast]
                    120 unit radius
```

### **Area Specifications**

#### **1. The Cabin (Center - Spawn Point)**
**Size:** 75 unit radius clearing
**Purpose:** Hub, safe space, bookends the game
**Atmosphere:** Warm amber light inside, cold blue from windows, melancholic

**Interior Layout:**
- Main room: Fireplace (dying embers), armchair, bookshelves
- Desk with papers, quill, inkwell
- Window with purple curtains (animated rustling)
- Door (to outside)
- Bust of Pallas above door frame
- Lenore's belongings: portrait, hairbrush, letters, jewelry box

**Exterior:**
- Small clearing, flat terrain
- Chopped wood pile
- Stone path to door
- Sparse trees at edge
- Fog begins outside clearing

**Interactions:**
- 5 readable letters from Lenore
- Examine portrait (triggers narration)
- Books (flavor text about forgotten lore)
- Fireplace (add wood to brighten room—optional)

**Progression:**
- Tutorial happens here
- Player can return anytime
- Changes based on memory collection:
  - More light if accepting grief
  - Darker if obsessing
- Final choice happens here

---

#### **2. The Whispering Woods (North - First Area)**
**Size:** 300 unit radius
**Theme:** Denial—searching for Lenore's voice
**Atmosphere:** Dense fog, distant whispers, disorienting

**Layout:**
- Dense tree coverage (70% density)
- Winding paths marked by ember trails
- No clear sightlines (easy to get lost)
- Occasional clearings with landmarks

**Landmarks:**
- Old well (can hear echoes)
- Broken fence (overgrown)
- Weathered shrine
- Hollow tree (memory ember inside)

**Memory Embers:** 5 total
- Each triggers memory of Lenore's voice/laughter
- Locations: well, shrine, hollow tree, clearing center, forest edge

**Gameplay:**
- Follow raven into woods
- Teach exploration mechanics
- Introduce memory collection
- Whispers lead player to embers (audio cue)

**Raven Behavior:**
- Flies between trees
- Perches on branches
- Leads to first ember
- Waits at area exit

**Exit Trigger:**
- Collect all 5 embers
- Raven flies toward next area
- Narration: "The voices fade... but the ache remains"

---

#### **3. The Ashen Grove (East - Second Area)**
**Size:** 200 unit radius
**Theme:** Anger—destruction and loss
**Atmosphere:** Burned trees, ash particles, darker lighting

**Layout:**
- Moderate tree coverage (40% density)
- All trees charred/dead (dark gray)
- Ash particles floating
- Ground covered in soot
- Occasional smoldering embers (environmental, not collectible)

**Landmarks:**
- Burned cabin ruins (not player's cabin)
- Charred statue
- Circle of dead trees
- Collapsed bridge

**Memory Embers:** 6 total
- Each triggers angry/painful memories
- Locations: ruins, statue, tree circle, bridge, two in forest

**Gameplay:**
- Player questions why this happened
- Environmental storytelling (other losses in the world)
- Darker tone than previous area

**Raven Behavior:**
- Perches on burned branches
- Caws more frequently (aggressive)
- Appears when player examines ruins

**Exit Trigger:**
- Collect all 6 embers
- Raven flies to Stone Circle
- Narration: "Destruction offers no answers"

---

#### **4. The Stone Circle (West - Third Area)**
**Size:** 180 unit radius
**Theme:** Bargaining—seeking supernatural answers
**Atmosphere:** Ancient, mysterious, spiritual

**Layout:**
- Large clearing (minimal trees)
- Central stone circle (12 standing stones, 30 unit diameter)
- Sparse grass coverage
- Misty but not dense fog
- Ancient feeling

**Landmarks:**
- Stone circle (main feature)
- Altar in center
- Carved symbols on stones
- Old ritual site
- Overgrown path

**Memory Embers:** 5 total
- Each shows player trying to bring Lenore back
- Locations: each direction stone, altar

**Gameplay:**
- Player seeks supernatural solution
- Examine stones (text about ancient rituals)
- Raven responds "Nevermore" to prayers
- Realization: no magic will help

**Raven Behavior:**
- Perches on central altar
- Immovable until all embers collected
- Says "Nevermore" to each stone interaction

**Exit Trigger:**
- Collect all 5 embers
- Attempt to interact with altar
- Raven flies away
- Narration: "No balm in Gilead... no salvation here"

---

#### **5. The Pallid Shore (South - Fourth Area)**
**Size:** 250 unit radius
**Theme:** Depression—emptiness and boundary
**Atmosphere:** Open, empty, end of the world feeling

**Layout:**
- Long shoreline (250 x 100 units)
- Flat water plane (no swimming)
- Very sparse trees (20% density)
- Open sky visible
- Still, mirror-like water

**Landmarks:**
- Wooden dock (extends into water)
- Abandoned rowboat
- Smooth stones on beach
- Single dead tree in water
- Distant fog wall

**Memory Embers:** 7 total
- Each shows depths of depression
- Locations: dock end, boat, along shoreline, tree

**Gameplay:**
- Longest walk of any area
- Minimal distractions
- Contemplative atmosphere
- Player reaches edge of playable world

**Raven Behavior:**
- Flies over water (unreachable)
- Silent—no cawing
- Appears distant, observing

**Exit Trigger:**
- Collect all 7 embers
- Walk to dock end
- Raven flies back toward cabin
- Narration: "Perhaps... it's time to return"

---

#### **6. The Echoing Glade (Southeast - Fifth Area)**
**Size:** 120 unit radius
**Theme:** Acceptance—peaceful memories
**Atmosphere:** Softer lighting, gentle ambience, beauty in sorrow

**Layout:**
- Small, intimate clearing
- Moderate tree coverage (50% density)
- Flowers/grass (only colorful area—subtle)
- Gentle sunbeams through fog
- Peaceful feeling

**Landmarks:**
- Memorial stone (blank, for player to project)
- Flowering tree (only living, healthy tree)
- Bench
- Small stream

**Memory Embers:** 6 total
- Each shows happy memories with Lenore
- Pure, untainted by grief
- Locations: memorial, tree, bench, stream, two in glade

**Gameplay:**
- Shortest area
- Emotional climax
- Player processes all collected memories
- Raven is almost absent

**Raven Behavior:**
- Perches silently on flowering tree
- Doesn't say "Nevermore"
- Seems to watch peacefully

**Exit Trigger:**
- Collect all 6 embers
- Sit on bench (optional interaction)
- Raven flies back to cabin
- Narration: "I remember... and I understand"

---

### **The Final Return**

**Back to Cabin:**
- Player walks back to cabin (raven leads)
- World feels different (same but perceived differently)
- Cabin door is open

**Inside Cabin:**
- Fireplace relit or fully dead (based on player pacing)
- Lenore's belongings remain
- Raven perched on bust of Pallas

**The Choice (Implicit):**
Player can interact with Lenore's belongings:
- **Acceptance Path:** Examine each item, then close jewelry box, cover portrait
  - Raven flies away through window
  - Dawn breaks into morning (lighting shifts warm)
  - Narration: "And my soul... shall be lifted"
  - Camera pans to sky, credits
  
- **Obsession Path:** Refuse to interact, or interact endlessly
  - Raven remains perched
  - Dawn never breaks (stuck in gray-blue)
  - Narration: "And the raven, never flitting, still is sitting..."
  - Camera focuses on raven's eyes, fade to black, credits

---

## **Audio Design**

### **Music**
- Minimal ambient soundtrack
- Layered atmospheric drones
- Area-specific tones:
  - Woods: whispers, wind through trees
  - Ashen: crackling, distant rumble
  - Stone: deep hums, echoes
  - Shore: water lapping, emptiness
  - Glade: gentle, almost hopeful

### **Sound Effects**
- **Player:**
  - Footsteps (grass, dirt, wood, stone—4 types)
  - Breathing (subtle, increases when sprinting)
  - Cloth rustling (movement)

- **Environment:**
  - Wind (constant, varies per area)
  - Tree rustling
  - Distant bird calls (not cheerful)
  - Water sounds at shore
  - Fire crackling in cabin
  - Door creaks
  - Curtain rustling

- **Raven:**
  - Wing flaps (when flying)
  - Landing/perching sound
  - Caw (3 variations, used sparingly)
  - Breathing/presence (when close)

- **Interactions:**
  - Paper rustling (letters)
  - Object pickup sound
  - Memory ember collection (soft chime)
  - UI sounds (menu, journal)

### **Voiceover**
- **Narrator (Player Character):**
  - Male voice, melancholic but not monotone
  - ~40-50 lines total
  - Triggered by:
    - Story beats
    - Memory ember collection
    - Area transitions
    - Examining key objects
  - Should sound like internal monologue

- **Lenore (Memories):**
  - Female voice, warm, loving
  - ~30 lines (one per memory ember)
  - Brief snippets (5-10 seconds each)
  - Not dialogue—remembered phrases

- **"Nevermore":**
  - Deep, resonant, slightly distorted
  - Single word, multiple deliveries
  - Used ~10 times throughout game

---

## **Visual Design**

### **Color Palette**
**Primary:**
- Gray-blue (sky, fog, shadows): #8B9DC3
- Dark blue-gray (terrain, trees): #4A5568
- Cool white (highlights, mist): #E2E8F0

**Accents:**
- Warm amber (cabin interior, embers): #FFA500
- Soft orange (memory embers): #FF8C42
- Purple (curtains, shadows): #6B5B95
- Black (raven, deep shadows): #1A1A1A

**Mood:**
- Desaturated overall
- High contrast between light/dark
- Fog softens everything
- Only memory embers are vibrant

### **Lighting**
**Global:**
- Directional light (sun, just below horizon)
- Ambient light (cool blue-gray, low intensity)
- Fog scattering (volumetric if possible)

**Dynamic:**
- Fireplace (point light, orange, flickering)
- Memory embers (point lights, orange, pulsing)
- Player lantern (optional—point light, amber)

**Post-Processing:**
- Subtle vignette
- Color grading (cool tones)
- Distance fog (exponential)
- Optional: light bloom on embers

### **Models (Primitive Placeholders First)**

**Phase 1 - Primitives:**
- Cabin: Cube + pyramid roof
- Trees: Cylinder trunk + cone/cylinder crown
- Raven: Sphere body + cone beak + flat spheres for wings
- Stones: Stretched cubes
- Props: Basic geometric shapes
- Memory embers: Glowing spheres

**Phase 2 - Final Models:**
- Cabin: Detailed log cabin, interior furnished
- Trees: 1-2 variations, mid-poly, dead/bare
- Raven: Detailed bird, animated wings
- Character hands/arms (if holding items)
- Props: Rocks, logs, posts, shrine, statues, dock, boat
- Particles: Mist, ash, embers, dust

### **UI Design**
**HUD (Minimal):**
- Interaction prompt: "Press E to examine" (bottom center, fades in/out)
- Quest objective: Top left, small text, fades when not updated
- Optional: Stamina bar (if sprint is limited)

**Menus:**
- Main menu: Title, "Begin," "Continue," "Settings," "Quit"
- Pause menu: "Resume," "Journal," "Settings," "Main Menu"
- Journal: Tabs for "Memories," "Areas," "Poem"
- Settings: Audio sliders, controls, graphics options

**Typography:**
- Serif font for narration/poem (like Crimson Text)
- Sans-serif for UI (like Inter)
- Readable sizes, high contrast

---

## **Technical Implementation Details**

### **Project Structure**
```
/raven_game
├── /src
│   └── /shaders               # Custom shaders
│   └─   # other code
├── /assets
│   ├── /models                # 3D models (.obj, .gltf)
│   ├── /textures              # Texture files
│   ├── /audio
│   │   ├── /music             # Ambient tracks
│   │   ├── /sfx               # Sound effects
│   │   └── /voice             # Voiceover lines
│   ├── /fonts                 # TTF fonts
├── /build                     # Compiled output
└── README.md
```

### **Core Systems to Implement**

#### **1. Player Controller**
**Features:**
- Smooth WASD movement
- Mouse look with sensitivity
- Sprint toggle
- Collision with terrain
- Head bob (subtle, optional)
- Footstep audio sync

---

#### **2. Terrain System**
**Features:**
- Infinite generation (you have this)
- LOD (optional, for performance)
- Height queries for collision
- Texture based on height/slope

---

#### **3. Raven AI**
**Behaviors:**
- Pathfind to perch points (simple waypoint system)
- Smooth flight (lerp position)
- Face direction of movement
- Play animations (wings flap during flight)
- Trigger "Nevermore" text when player approaches
- Block player at boundaries

---

#### **4. Quest System**
**Features:**
- One active quest at a time
- Simple progress tracking
- UI updates on progress
- Trigger next quest on completion

---

#### **5. Interactable Objects**
**Features:**
- Distance-based prompt (show "Press E" within 2-3 units)
- Trigger audio/narration on interact
- Collect memory embers
- Read notes (pause game, show text overlay)

---

#### **6. Audio Manager**
**Features:**
- 3D positional audio (raven caw, footsteps)
- Area-based ambient music
- Voiceover queue (don't overlap lines)
- Volume controls
- Crossfade between area tracks

---

#### **7. UI System**
**Features:**
- Minimal HUD
- Quest display (fades after 3 seconds if no update)
- Interaction prompts
- Journal with tabs
- Settings menu

---

#### **8. Narrative System**
**Features:**
- Trigger voiceover at specific points
- Subtitle display (bottom of screen)
- Don't repeat lines unless scripted
- Queue system (wait for current line to finish)

---

#### **9. Fog System**
**Features:**
- Distance-based fog (built into raylib)
- Area-specific fog density
- Boundary fog (gets thicker outside play area)
- Color tint (blue-gray)

---

#### **10. Particle System**
**Features:**
- Memory ember glow (particles orbit ember)
- Ash falling in burned area
- Ground mist layers
- Dust motes in cabin

---

#### **11. Save System**
**Features:**
- Auto-save on quest completion
- Manual save in pause menu
- Single save slot (or multiple if desired)
- Store quest state, collected items, discovered areas

---

## **Complete Development Checklist**

### **PHASE 0: Pre-Production (1 week)**

**Setup:**
- [x] Set up project structure (folders, source files)
- [x] Initialize raylib project
- [x] Set up version control (git repository)
- [x] Create build system (Makefile or CMake)
- [x] Test compilation and basic window creation

**Design:**
- [ ] Write complete narrative script (all voiceover lines)
- [ ] Create area design documents (layout, landmarks, progression)
- [ ] Define all quest objectives
- [ ] List all interactable objects needed
- [ ] Sketch UI mockups

**Assets Planning:**
- [ ] List all 3D models needed (primitives vs. final)
- [ ] List all sound effects needed
- [ ] List all voiceover lines needed
- [ ] List all music/ambient tracks needed
- [ ] Create asset naming convention

---

### **PHASE 1: Foundation (3-4 weeks)**

#### **Week 1: Core Player Systems**

**Player Movement:**
- [x] Impxement basic first-person camera
- [x] Add WASD movement (constant speed)
- [x] Add mouse look (pitch/yaw control)
- [x] Implement sprint (Shift key, 2x speed)
- [x] Add basic collision with terrain
- [x] Test movement feel (adjust speeds if needed)

**Camera:**
- [x] Smooth camera rotation
- [x] Clamp vertical look angle (-89° to 89°)
- [x] Add mouse sensitivity setting
- [x] Optional: subtle head bob when walking

**Terrain Integration:**
- [x] Query terrain height at player position
- [x] Keep player grounded (position.y = terrain height)
- [x] Test on your procedurally generated terrain
- [x] Ensure no falling through terrain

**Debug Tools:**
- [x] Display player position on screen
- [x] Display FPS counter
- [ ] Toggle wireframe mode
- [ ] Toggle collision visualization

---

#### **Week 2: World Foundation**

**Terrain System:**
- [ ] Refine terrain generation for Dawn aesthetic
- [x] Implement terrain chunking (if not already done)
- [x] Add basic texture (gray-blue gradient by height)
- [x] Optimize terrain rendering

**Lighting:**
- [x] Set up directional light (dawn angle, ~15° above horizon)
- [x] Set ambient light (cool blue-gray, low intensity)
- [x] Test lighting across terrain
- [x] Adjust colors to match gray-blue palette

**Fog:**
- [ ] Implement distance fog (raylib built-in)
- [ ] Set fog color (light blue-gray)
- [ ] Set fog start/end distances
- [ ] Test fog density—should limit visibility to ~200 units
- [ ] Adjust for atmospheric feeling

**World Boundaries:**
- [ ] Define world center (0, 0, 0)
- [ ] Define world radius (750 units)
- [ ] Implement soft boundary system:
    - [ ] Increase fog density beyond radius
    - [ ] Add gentle physics push toward center
    - [ ] If distance is more than 100 units across border, strong push them back
    - [ ] Display warning text when far from center

**Debug Tools:**
- [ ] Draw world boundary circle
- [ ] Draw player distance from center
- [ ] Toggle area boundary visualization

---

#### **Week 3: Basic Interaction**

**Interaction System:**
- [x] Create WorldObject type API
- [x] Add distance checking (player within 3 or 5 units)
- [x] Interaction prompt event handler
- [x] Trigger prompt event action (print for now)

**Object Placement:**
- [ ] Manually place 5-10 test objects in world
- [ ] Test interaction from different angles
- [ ] Ensure prompt appears/disappears correctly
- [ ] Test interaction while moving

**UI Framework:**
- [x] Set up basic UI rendering (raylib text)
- [ ] Create DrawText helper with custom font
- [ ] Implement interaction prompt display (bottom center)
- [ ] Fade in/out animation for prompt
- [ ] Test readability against various backgrounds

---

#### **Week 4: Audio Foundation**

**Audio Manager:**
- [ ] Initialize raylib audio system
- [ ] Create AudioManager struct
- [ ] Implement PlaySound() function
- [ ] Implement PlayMusic() function
- [ ] Test basic sound playback

**Footsteps:**
- [ ] Record/download 4 footstep sounds (grass, dirt, wood, stone)
- [ ] Implement footstep timing based on movement
- [ ] Play footstep sound every ~0.5 seconds while moving
- [ ] Test footstep sync with movement speed
- [ ] Stop footsteps when player stops

**Ambient Audio:**
- [ ] Find/create basic wind sound loop
- [ ] Play wind as ambient background
- [ ] Set volume (subtle, ~30%)
- [ ] Test audio levels

**3D Audio:**
- [ ] Implement basic 3D audio (volume based on distance)
- [ ] Test with placed objects
- [ ] Adjust falloff distances

---

### **PHASE 2: The Cabin - Vertical Slice (3-4 weeks)**

**Goal:** Complete the cabin interior/exterior as a polished proof-of-concept.

#### **Week 5: Cabin Exterior**

**Modeling (Primitive):**
- [ ] Create cabin primitive:
  - [ ] Box for walls (8w x 6h x 10d units)
  - [ ] Pyramid for roof
  - [ ] Combine into single model
- [ ] Position cabin at world center (0, height, 0)
- [ ] Create door primitive (thin box)

**Environment:**
- [ ] Flatten terrain in 75-unit radius around cabin
- [ ] Clear trees in cabin clearing
- [ ] Place 3-5 tree primitives at clearing edge:
  - [ ] Cylinder trunk (0.5 radius, 4 height)
  - [ ] Cone top (2 radius, 3 height)
- [ ] Place primitive props:
  - [ ] Wood pile (stacked boxes)
  - [ ] Stumps (short cylinders)
  - [ ] Path stones (flat boxes leading to door)

**Lighting:**
- [ ] Add warm point light inside cabin (amber, visible through windows)
- [ ] Adjust exterior lighting for contrast

**Testing:**
- [ ] Walk around cabin
- [ ] Test visibility from distance
- [ ] Ensure clearing feels right size
- [ ] Adjust tree placement if needed

---

#### **Week 6: Cabin Interior**

**Interior Modeling (Primitive):**
- [ ] Create interior space (remove back faces from walls)
- [ ] Add floor plane
- [ ] Create window openings (2 windows with purple curtains)
  - [ ] Curtains = flat planes with semi-transparency
- [ ] Add door frame above door
- [ ] Model bust of Pallas (sphere on cylinder on box)

**Furniture (Primitive):**
- [ ] Fireplace (box + smaller box for mantle)
- [ ] Armchair (boxes for seat/back/arms)
- [ ] Desk (box + thin boxes for legs)
- [ ] Bookshelf (box with horizontal dividers)
- [ ] Side table

**Props:**
- [ ] Books (tiny boxes on shelf)
- [ ] Papers on desk
- [ ] Lenore's portrait (flat plane with texture)
- [ ] Jewelry box
- [ ] Hairbrush
- [ ] Letters (flat planes on desk)
- [ ] Quill and inkwell

**Lighting:**
- [ ] Fireplace point light (orange, flickering)
  - [ ] Implement flicker (random intensity 0.8-1.2)
- [ ] Window light (blue from outside)
- [ ] Adjust indoor/outdoor light contrast

**Interactables:**
- [ ] Make 5 letters interactable
- [ ] Make portrait interactable
- [ ] Make jewelry box interactable
- [ ] Make each book interactable (3-5 books)
- [ ] Make door interactable (open/close)
- [ ] Make fireplace interactable (examine embers)

---

#### **Week 7: Cabin Gameplay**

**Tutorial Sequence:**
- [ ] Player spawns inside cabin, near armchair
- [ ] Display "WASD to move, Mouse to look" for 3 seconds
- [ ] After 5 seconds, play narrator line: "Once upon a midnight dreary..."
- [ ] Prompt player to examine letters/books
- [ ] After first interaction, play next narrator line

**Tapping Sequence:**
- [ ] After 30 seconds in cabin, play tapping sound (3D audio from door)
- [ ] Display objective: "Investigate the sound"
- [ ] When player approaches door, play narrator line: "'Tis some visitor..."
- [ ] When player opens door, show only darkness and fog
- [ ] After 3 seconds, close door automatically (or let player close)
- [ ] Play narrator line: "Darkness there and nothing more"

**Window Sequence:**
- [ ] 10 seconds after door, play tapping at window
- [ ] Display objective: "Investigate the window"
- [ ] When player approaches window, play louder tapping
- [ ] Trigger raven entrance (next phase)

**Journal System:**
- [ ] Implement basic journal (ESC → Journal)
- [ ] Add "Memories" tab (empty for now)
- [ ] Add "Notes" tab with collected letters
- [ ] When player reads a letter, add to journal

**Voiceover:**
- [ ] Record/source temp narrator voiceover (5-7 lines for cabin)
- [ ] Trigger lines based on player actions
- [ ] Display subtitles (bottom of screen, white text, black shadow)
- [ ] Test subtitle timing and readability

---

#### **Week 8: Polish & Refinement**

**Particle Effects:**
- [ ] Implement ember particle system
- [ ] Add embers in fireplace (orange particles rising)
- [ ] Add dust motes in light beams (subtle)

**Animation:**
- [ ] Animate curtains (subtle sway)
  - [ ] Shader or vertex animation
- [ ] Flicker fireplace light

**Audio:**
- [ ] Add cabin ambience (wind outside, creaking wood)
- [ ] Add fire crackling sound (3D audio from fireplace)
- [ ] Add door creak sound (open/close)
- [ ] Add paper rustling sound (when reading letters)
- [ ] Adjust all audio levels

**UI Polish:**
- [ ] Smooth fade in/out for interaction prompts
- [ ] Smooth fade for subtitles
- [ ] Add vignette effect (subtle darkening at screen edges)
- [ ] Test UI readability in all lighting conditions

**Testing:**
- [ ] Full playthrough of cabin sequence (10-15 min)
- [ ] Test all interactions
- [ ] Check for bugs (falling through floor, missing audio, etc.)
- [ ] Get feedback from someone else if possible
- [ ] Fix any issues

**Save/Load:**
- [ ] Implement basic save system
- [ ] Auto-save when exiting cabin
- [ ] Test save/load functionality

---

### **PHASE 3: The Raven (2 weeks)**

#### **Week 9: Raven Implementation**

**Modeling:**
- [ ] Create raven primitive:
  - [ ] Body (sphere, 0.5 radius)
  - [ ] Head (smaller sphere)
  - [ ] Beak (cone)
  - [ ] Wings (two flattened spheres on sides)
  - [ ] Tail (flat triangle)
- [ ] Color all black
- [ ] Position raven outside cabin window

**Basic AI:**
- [ ] Create Raven struct (position, target, state)
- [ ] Implement state machine:
  - [ ] IDLE: stationary, occasional head turn
  - [ ] FLYING: move toward target
  - [ ] PERCHED: sitting on object, watching player
  - [ ] BLOCKING: preventing player progress

**Flying:**
- [ ] Implement smooth flight (lerp position to target)
- [ ] Set flight speed (6 units/sec, faster than player walk)
- [ ] Add simple wing flap animation (scale wings up/down)
- [ ] Face direction of movement
- [ ] Add takeoff/landing animations (optional, can be instant)

**Perching:**
- [ ] Define perch points (Vector3 positions + normals)
- [ ] Snap raven to perch point when reached
- [ ] Rotate raven to align with perch surface
- [ ] Add idle animation (subtle head turns every 2-3 sec)

**Audio:**
- [ ] Find/create raven caw sound (3-4 variations)
- [ ] Implement 3D audio for caw (distance-based volume)
- [ ] Play caw occasionally while perched (every 10-20 sec)
- [ ] Play wing flap sound when taking off

---

#### **Week 10: Raven Behavior**

**Entrance Sequence:**
- [ ] When player approaches window, trigger raven entrance
- [ ] Raven flies toward window (from outside)
- [ ] Raven breaks through window (glass break sound, optional)
- [ ] Raven flies around cabin interior (scripted path)
- [ ] Raven perches on bust of Pallas above door
- [ ] Play narrator line: "In there stepped a stately Raven..."
- [ ] Display "Nevermore" text when player approaches raven

**"Nevermore" System:**
- [ ] When player within 3 units of raven, display "Nevermore"
- [ ] Text appears above raven (world-space text)
- [ ] Play "Nevermore" voiceover (deep, resonant)
- [ ] Fade out after 2 seconds
- [ ] Don't repeat if player stays near (only on approach)

**Following Mechanic:**
- [ ] After player examines all cabin items, raven flies out window
- [ ] Raven perches on tree at clearing edge
- [ ] Display objective: "Follow the raven"
- [ ] When player approaches, raven flies to next perch
- [ ] Lead player toward first forest area

**Blocking Behavior:**
- [ ] When player goes wrong direction, raven flies in front
- [ ] Raven perches mid-air (or on invisible perch)
- [ ] Says "Nevermore"
- [ ] Player can't pass through raven (collision)
- [ ] After 5 seconds, raven returns to correct path

**Observing:**
- [ ] When player explores area, raven perches nearby
- [ ] Occasionally changes perch points
- [ ] Watches player (rotate head to face player)
- [ ] Caws when player finds important objects

**Testing:**
- [ ] Test all raven states
- [ ] Ensure smooth transitions between states
- [ ] Test following behavior
- [ ] Test blocking behavior
- [ ] Adjust flight speed if too fast/slow

---

### **PHASE 4: First Exterior Area - Whispering Woods (3 weeks)**

#### **Week 11: Woods Layout**

**Terrain Modification:**
- [ ] Define Whispering Woods center (0, 0, -400)
- [ ] Define area radius (200 units)
- [ ] Slightly modify terrain height in this area (subtle hills)

**Tree Population:**
- [ ] Implement tree placement algorithm:
  - [ ] Randomly place trees in radius
  - [ ] Check for spacing (min 5 units apart)
  - [ ] Avoid placing on steep slopes
- [ ] Populate woods with 150-200 tree primitives
- [ ] Vary tree scale (0.8 - 1.2x)
- [ ] Create dense feeling (70% coverage)

**Landmarks:**
- [ ] Create old well (cylinder with stone texture)
  - [ ] Position: (-50, 0, -400)
- [ ] Create broken fence (cylinders + horizontal planks)
  - [ ] Position: (30, 0, -450)
- [ ] Create weathered shrine (stacked boxes)
  - [ ] Position: (80, 0, -380)
- [ ] Create hollow tree (larger cylinder, hollow inside)
  - [ ] Position: (-40, 0, -350)

**Paths:**
- [ ] Create winding path from cabin to woods entrance
- [ ] Mark path with small stones (primitive cubes)
- [ ] Path width: 3-4 units
- [ ] Path should be subtle, not obvious

**Fog:**
- [ ] Increase fog density in woods (0.7-0.8)
- [ ] Reduce visibility to ~150 units
- [ ] Test readability (player can still navigate)

---

#### **Week 12: Woods Gameplay**

**Memory Embers:**
- [ ] Create memory ember primitive (glowing sphere, orange)
- [ ] Add pulsing animation (scale 0.9-1.1 over 2 seconds)
- [ ] Add particle effect (orbiting particles)
- [ ] Place 5 embers in woods:
  1. [ ] At old well
  2. [ ] At broken fence
  3. [ ] Inside hollow tree
  4. [ ] Near weathered shrine
  5. [ ] In clearing (random location)

**Ember Collection:**
- [ ] Make embers interactable (3 unit radius)
- [ ] On collect:
  - [ ] Play collection sound (soft chime)
  - [ ] Play voiceover (memory of Lenore)
  - [ ] Display subtitle
  - [ ] Remove ember from world
  - [ ] Update quest progress
  - [ ] Add memory to journal

**Whisper System:**
- [ ] Implement 3D audio sources for whispers
- [ ] Place whisper sources near each ember (5-10 units away)
- [ ] Play whisper sound (faint voice saying "Lenore")
- [ ] Audio should guide player toward embers
- [ ] Test audio falloff (audible from ~20 units)

**Quest Implementation:**
- [ ] Quest: "Enter the Whispering Woods"
  - [ ] Triggers when player leaves cabin clearing
  - [ ] Completes when player enters woods radius
- [ ] Quest: "Find 5 Memory Embers"
  - [ ] Track collected embers (0/5)
  - [ ] Update UI on each collection
  - [ ] Complete when all 5 collected

**Raven Behavior:**
- [ ] Raven leads player from cabin to woods entrance
- [ ] Raven perches at woods entrance
- [ ] When player enters, raven flies deeper into woods
- [ ] Raven perches near first ember
- [ ] After collection, raven flies to next ember
- [ ] After all embers, raven flies to woods exit

---

#### **Week 13: Woods Polish**

**Audio:**
- [ ] Add woods ambience (wind through trees, distant birds)
- [ ] Add leaf rustling sounds (random 3D sources)
- [ ] Add player footsteps on forest floor (different from cabin)
- [ ] Record/source 5 voiceover lines for memories
- [ ] Test audio mix (whispers audible but not overwhelming)

**Lighting:**
- [ ] Add light shafts through trees (god rays, if possible)
- [ ] Adjust ambient lighting (darker under canopy)
- [ ] Test visibility (not too dark)

**Particles:**
- [ ] Add falling leaves (rare, subtle)
- [ ] Add ground mist (particle emitters at ground level)

**Landmarks Interactivity:**
- [ ] Make well interactable (hear echoes)
- [ ] Make shrine interactable (examine inscription)
- [ ] Make hollow tree interactable (examine interior)

**Navigation:**
- [ ] Ensure player can find all embers
- [ ] Add subtle hints (whispers, raven, visual cues)
- [ ] Test for getting lost (is it frustrating or atmospheric?)

**Exit:**
- [ ] When all embers collected, display objective: "Follow the raven"
- [ ] Raven flies toward next area
- [ ] Play narrator line: "The voices fade, but the ache remains"
- [ ] Fade to black (brief)
- [ ] Fade in at woods exit

**Testing:**
- [ ] Full playthrough (15-20 min)
- [ ] Test all ember locations
- [ ] Test quest progression
- [ ] Check for navigation issues
- [ ] Fix bugs

---

### **PHASE 5: Core Systems Completion (2 weeks)**

#### **Week 14: Quest & Journal Systems**

**Quest Manager:**
- [ ] Create QuestManager struct
- [ ] Track active quest
- [ ] Track quest history
- [ ] Implement quest completion rewards
- [ ] Add quest notifications (on-screen popup)

**Journal UI:**
- [ ] Design journal layout (3 tabs)
- [ ] Tab 1: Memories
  - [ ] List all collected memory embers
  - [ ] Display text for each memory
  - [ ] Show which area it's from
- [ ] Tab 2: Areas
  - [ ] List discovered areas
  - [ ] Show brief description
  - [ ] Show collection progress (X/Y embers)
- [ ] Tab 3: Poem
  - [ ] Display poem excerpts unlocked per area
  - [ ] Format in stanzas

**Journal Functionality:**
- [ ] Open journal with TAB or J key
- [ ] Navigate tabs with arrow keys or mouse
- [ ] Scroll content if it overflows
- [ ] Close journal with ESC or same key
- [ ] Pause game when journal is open

**Testing:**
- [ ] Test journal with collected content
- [ ] Ensure UI is readable and intuitive
- [ ] Test keyboard and mouse navigation

---

#### **Week 15: Save/Load & Settings**

**Save System:**
- [ ] Implement comprehensive save data:
  - [ ] Player position
  - [ ] Quest progress
  - [ ] Collected embers (bool array)
  - [ ] Discovered areas
  - [ ] Playtime
  - [ ] Journal entries
- [ ] Save to file (binary or JSON)
- [ ] Auto-save on quest completion
- [ ] Manual save in pause menu

**Load System:**
- [ ] Load save file on game start (if exists)
- [ ] Restore all player data
- [ ] Restore world state (remove collected embers, etc.)
- [ ] Test save/load thoroughly

**Main Menu:**
- [ ] Create main menu scene
- [ ] "New Game" button (starts fresh)
- [ ] "Continue" button (loads save, grayed if no save exists)
- [ ] "Settings" button
- [ ] "Quit" button
- [ ] Background: Still image of cabin at dawn

**Pause Menu:**
- [ ] Pause game with ESC
- [ ] "Resume" button
- [ ] "Journal" button
- [ ] "Settings" button
- [ ] "Main Menu" button (with confirmation)

**Settings Menu:**
- [ ] Audio settings:
  - [ ] Master volume slider
  - [ ] Music volume slider
  - [ ] SFX volume slider
  - [ ] Voice volume slider
- [ ] Graphics settings:
  - [ ] Resolution dropdown (if relevant)
  - [ ] Fullscreen toggle
  - [ ] VSync toggle
  - [ ] Fog quality (high/medium/low)
- [ ] Controls settings:
  - [ ] Mouse sensitivity slider
  - [ ] Key rebinding (optional, can use defaults)
- [ ] Accessibility:
  - [ ] Subtitle toggle
  - [ ] Subtitle size
  - [ ] Colorblind mode (optional)

**Testing:**
- [ ] Test save/load multiple times
- [ ] Test all menu navigation
- [ ] Test all settings (ensure they apply)
- [ ] Test pause/resume

---

### **PHASE 6: Remaining Areas (5-6 weeks)**

**Goal:** Implement Ashen Grove, Stone Circle, Pallid Shore, and Echoing Glade.

*(Each area follows similar structure to Whispering Woods)*

---

#### **Week 16-17: Ashen Grove (East)**

**Layout:**
- [ ] Define area center (450, 0, 0)
- [ ] Define radius (200 units)
- [ ] Populate with burned trees (40% density)
- [ ] Color trees dark gray/black
- [ ] Add charred texture (or tint)

**Landmarks:**
- [ ] Burned cabin ruins (collapsed walls, charred wood)
- [ ] Charred statue (humanoid figure, blackened)
- [ ] Circle of dead trees (arranged in circle)
- [ ] Collapsed bridge (over a dry creek bed)

**Memory Embers:**
- [ ] Place 6 embers at landmarks and scattered
- [ ] Each triggers angry/painful memory voiceover

**Atmosphere:**
- [ ] Add ash particle system (falling from sky)
- [ ] Darken ambient lighting
- [ ] Add smoldering ember effects (ground-level, rare)
- [ ] Add crackling sounds (distant fires)

**Raven Behavior:**
- [ ] Raven caws more frequently (aggressive)
- [ ] Perches on charred branches
- [ ] Observes player examining ruins

**Quest:**
- [ ] "Explore the Ashen Grove"
- [ ] "Collect 6 Memory Embers"

**Narration:**
- [ ] Record 6 voiceover lines (painful memories)
- [ ] Exit line: "Destruction offers no answers"

**Testing:**
- [ ] Full playthrough
- [ ] Ensure atmosphere feels distinct from Woods
- [ ] Test ember placement

---

#### **Week 18: Stone Circle (West)**

**Layout:**
- [ ] Define area center (-350, 0, 0)
- [ ] Define radius (180 units)
- [ ] Clear trees in central area (large clearing)
- [ ] Flatten terrain

**Landmarks:**
- [ ] Central stone circle (12 standing stones, 30 unit diameter)
  - [ ] Each stone: tall box (1w x 4h x 0.8d)
  - [ ] Arrange in circle
- [ ] Central altar (stacked boxes, 2h)
- [ ] Carved symbols on stones (texture or decals)
- [ ] Ancient path leading to circle

**Memory Embers:**
- [ ] Place 5 embers, one near each cardinal direction stone + center
- [ ] Each triggers bargaining memory voiceover

**Atmosphere:**
- [ ] Light mist (ground level)
- [ ] Mysterious ambience (deep hums, echoes)
- [ ] Brighter lighting than other areas (open sky)
- [ ] Wind sounds

**Raven Behavior:**
- [ ] Raven perches on central altar
- [ ] Immovable until all embers collected
- [ ] Says "Nevermore" when player interacts with each stone
- [ ] Flies away only after quest complete

**Quest:**
- [ ] "Enter the Stone Circle"
- [ ] "Seek answers from the stones" (collect 5 embers)
- [ ] "Approach the altar" (triggers completion)

**Narration:**
- [ ] Record 5 voiceover lines (bargaining, seeking supernatural help)
- [ ] Exit line: "No balm in Gilead... no salvation here"

**Interactions:**
- [ ] Make each stone interactable (examine carvings)
- [ ] Make altar interactable (attempt ritual, raven responds)

**Testing:**
- [ ] Full playthrough
- [ ] Test raven interaction with altar
- [ ] Ensure open atmosphere feels distinct

---

#### **Week 19-20: Pallid Shore (South)**

**Layout:**
- [ ] Define area center (0, 0, 450)
- [ ] Define radius (250 units, elongated 250w x 100d)
- [ ] Create flat water plane (just below terrain level)
- [ ] Flatten shoreline terrain

**Landmarks:**
- [ ] Wooden dock (extends 30 units into water)
  - [ ] Planks (thin boxes)
  - [ ] Posts (cylinders)
- [ ] Abandoned rowboat (box hull, cylinder seats)
- [ ] Smooth stones along beach (scattered)
- [ ] Single dead tree standing in shallow water
- [ ] Distant fog wall (marks world boundary)

**Memory Embers:**
- [ ] Place 7 embers along shoreline and on dock
- [ ] Each triggers depressive memory voiceover

**Atmosphere:**
- [ ] Still, mirror-like water (reflective shader, optional)
- [ ] Very sparse trees (20% density, only at edges)
- [ ] Open sky feeling
- [ ] Gentle water lapping sounds
- [ ] Subtle wind
- [ ] Cooler color temperature

**Raven Behavior:**
- [ ] Raven flies over water (unreachable)
- [ ] Occasionally perches on dead tree in water
- [ ] Silent—no cawing
- [ ] Distant, observing from afar

**Quest:**
- [ ] "Walk to the Pallid Shore"
- [ ] "Follow the shoreline" (collect 7 embers)
- [ ] "Reach the end of the dock"

**Narration:**
- [ ] Record 7 voiceover lines (depths of depression, emptiness)
- [ ] Exit line: "Perhaps... it's time to return"

**Interactions:**
- [ ] Make boat interactable (examine)
- [ ] Make dead tree interactable (examine)
- [ ] Walking to end of dock triggers contemplation scene

**Testing:**
- [ ] Full playthrough (longest area, ~20 min)
- [ ] Ensure openness doesn't feel empty
- [ ] Test water rendering
- [ ] Adjust ember spacing if area feels too big

---

#### **Week 21: Echoing Glade (Southeast)**

**Layout:**
- [ ] Define area center (300, 0, 300)
- [ ] Define radius (120 units, smallest area)
- [ ] Moderate tree coverage (50% density)
- [ ] Gentle terrain (small hill in center)

**Landmarks:**
- [ ] Memorial stone (blank, smooth, upright)
- [ ] Flowering tree (only tree with color—subtle pink/white blossoms)
- [ ] Wooden bench (simple, facing memorial)
- [ ] Small stream (narrow, shallow, flowing water)

**Memory Embers:**
- [ ] Place 6 embers around glade
- [ ] Each triggers happy, pure memory of Lenore

**Atmosphere:**
- [ ] Softer lighting (god rays through canopy)
- [ ] Gentle ambience (birds, stream, wind)
- [ ] Subtle color (flowers, slight grass tint—only area with color)
- [ ] Peaceful, contemplative mood

**Raven Behavior:**
- [ ] Raven perches silently on flowering tree
- [ ] Doesn't say "Nevermore" in this area
- [ ] Watches peacefully
- [ ] Almost seems sympathetic

**Quest:**
- [ ] "Enter the Echoing Glade"
- [ ] "Remember" (collect 6 embers)
- [ ] "Sit and reflect" (optional interaction with bench)

**Narration:**
- [ ] Record 6 voiceover lines (happiest memories, untainted)
- [ ] Exit line: "I remember... and I understand"

**Interactions:**
- [ ] Make memorial stone interactable (moment of silence)
- [ ] Make bench interactable (sit, optional, triggers reflection)
- [ ] Make flowering tree interactable (examine blossoms)

**Testing:**
- [ ] Full playthrough
- [ ] Ensure emotional impact (should feel cathartic)
- [ ] Test color visibility (subtle, not jarring)

---

### **PHASE 7: Ending & Polish (2-3 weeks)**

#### **Week 22: The Final Return**

**Journey Back:**
- [ ] After Echoing Glade, quest: "Return to the cabin"
- [ ] Raven flies back toward cabin
- [ ] Player walks back (no fast travel, time to reflect)
- [ ] World feels different (same geometry, different perception)
  - [ ] Slightly warmer lighting
  - [ ] Less fog
  - [ ] Subtle changes to convey internal shift

**Cabin Changes:**
- [ ] Cabin door is open (was closed)
- [ ] Interior feels different based on player's journey:
  - [ ] If player rushed: darker, cold
  - [ ] If player took time: warmer, softer
- [ ] Raven is perched on bust of Pallas

**The Choice:**
- [ ] Player can interact with Lenore's belongings
- [ ] No explicit "Choose Ending A or B" prompt
- [ ] Ending determined by player's actions

**Acceptance Ending:**
- [ ] If player examines each of Lenore's items (portrait, letters, jewelry)
- [ ] Option appears: "Put away" or "Let go"
- [ ] Choosing "Let go" for each item:
  - [ ] Cover portrait with cloth
  - [ ] Close jewelry box gently
  - [ ] Place letters in drawer
- [ ] After last item, trigger ending sequence:
  - [ ] Raven flies away through window
  - [ ] Dawn light intensifies (golden hour)
  - [ ] Warm lighting fills cabin
  - [ ] Play narrator line: "And my soul... shall be lifted"
  - [ ] Camera slowly pans to window, watching raven fly away
  - [ ] Fade to white
  - [ ] Credits

**Obsession Ending:**
- [ ] If player refuses to interact with items, or interacts but doesn't "let go"
- [ ] After standing in cabin for ~60 seconds without choosing:
  - [ ] Raven remains perched, staring
  - [ ] Dawn never progresses (stuck in gray-blue)
  - [ ] Cabin gets darker
  - [ ] Play narrator line: "And the raven, never flitting, still is sitting..."
  - [ ] Camera slowly zooms on raven's eyes
  - [ ] Raven's eyes glow faintly
  - [ ] Fade to black
  - [ ] Credits

**Testing:**
- [ ] Test both endings
- [ ] Ensure triggers work correctly
- [ ] Test edge cases (what if player leaves cabin again?)
- [ ] Time the ending sequences (not too fast, not too slow)

---

#### **Week 23-24: Final Polish**

**Visual Polish:**
- [ ] Color grading pass (ensure consistent palette)
- [ ] Lighting pass (adjust all areas)
- [ ] Add subtle vignette effect
- [ ] Add optional light bloom on embers
- [ ] Screen space effects (fog, depth of field—optional)

**Audio Polish:**
- [ ] Final audio mix (balance all volumes)
- [ ] Ensure no audio pops or clicks
- [ ] Add ambient layers to all areas
- [ ] Crossfade music between areas (if using music tracks)
- [ ] Master all audio to consistent levels

**Particle Polish:**
- [ ] Refine all particle systems
- [ ] Ensure particles don't tank performance
- [ ] Add variety (different particle sizes, speeds)

**UI Polish:**
- [ ] Consistent font usage
- [ ] Smooth transitions (fades, wipes)
- [ ] Readable in all lighting conditions
- [ ] Add UI sounds (menu clicks, hovers)

**Performance Optimization:**
- [ ] Profile frame rate in all areas
- [ ] Optimize draw calls (batch similar objects)
- [ ] LOD for distant objects (if needed)
- [ ] Cull objects outside view frustum
- [ ] Optimize particle counts
- [ ] Target: 60 FPS on mid-range hardware

**Bug Fixing:**
- [ ] Full playthrough, note all bugs
- [ ] Fix collision issues
- [ ] Fix audio bugs
- [ ] Fix quest progression bugs
- [ ] Fix save/load bugs
- [ ] Fix UI bugs

**Playtesting:**
- [ ] Get 3-5 people to playtest
- [ ] Watch them play (don't give hints)
- [ ] Note where they get stuck
- [ ] Note where they lose interest
- [ ] Gather feedback
- [ ] Iterate based on feedback

---

### **PHASE 8: Asset Replacement (2-3 weeks, optional)**

**Goal:** Replace primitive placeholders with final 3D models.

**Priority Order:**

1. **The Raven** (most important)
   - [ ] Commission or model detailed raven
   - [ ] Rig and animate (perch, fly, idle)
   - [ ] Import and integrate
   - [ ] Test all behaviors with new model

2. **The Cabin**
   - [ ] Model detailed log cabin exterior
   - [ ] Model interior (walls, floor, ceiling)
   - [ ] Model furniture (fireplace, armchair, desk, shelves)
   - [ ] Import and replace primitives
   - [ ] Adjust lighting for new geometry

3. **Trees**
   - [ ] Model 1-2 tree variations (dead, bare branches)
   - [ ] Create LOD versions (optional)
   - [ ] Replace all tree primitives
   - [ ] Test performance

4. **Props & Landmarks**
   - [ ] Model key props (well, shrine, stones, dock, boat, etc.)
   - [ ] Model smaller props (books, letters, jewelry box, etc.)
   - [ ] Import and replace
   - [ ] Adjust scales and positions

5. **Environmental Details**
   - [ ] Model rocks, logs, stumps
   - [ ] Add variety
   - [ ] Scatter procedurally

**Testing:**
- [ ] Full playthrough with new assets
- [ ] Ensure no visual bugs (z-fighting, missing textures)
- [ ] Test performance (models should be optimized)
- [ ] Adjust lighting if needed

---

### **PHASE 9: Final Testing & Release Prep (1-2 weeks)**

**Comprehensive Testing:**
- [ ] Full playthrough from start to finish
- [ ] Test both endings
- [ ] Test all interactions
- [ ] Test save/load at various points
- [ ] Test settings (ensure they persist)
- [ ] Test on different hardware (if possible)
- [ ] Test at different resolutions

**Bug Bash:**
- [ ] Create bug list
- [ ] Prioritize bugs (critical, major, minor)
- [ ] Fix all critical bugs
- [ ] Fix as many major bugs as possible
- [ ] Accept minor bugs if time constrained

**Performance Testing:**
- [ ] Profile on minimum spec hardware
- [ ] Optimize bottlenecks
- [ ] Ensure stable frame rate

**Accessibility:**
- [ ] Ensure subtitles work correctly
- [ ] Test colorblind mode (if implemented)
- [ ] Ensure UI is readable at different resolutions
- [ ] Test with keyboard only, mouse only

**Release Build:**
- [ ] Create release build configuration
- [ ] Strip debug symbols
- [ ] Optimize compiler flags
- [ ] Test release build thoroughly

**Documentation:**
- [ ] Write README (how to run, controls, credits)
- [ ] Create control reference (in-game or separate doc)
- [ ] Write credits (list all asset sources, tools used)

**Marketing Materials (if releasing publicly):**
- [ ] Take screenshots (5-10 beautiful shots)
- [ ] Record trailer (1-2 min, show atmosphere, gameplay, don't spoil ending)
- [ ] Write description (1-2 paragraphs)
- [ ] Create itch.io page (or Steam, etc.)

**Launch:**
- [ ] Upload build to distribution platform
- [ ] Set pricing (or free)
- [ ] Publish
- [ ] Share with community

---

## **Summary Timeline**

**Total Development Time: ~5-7 months**

- Phase 0: Pre-Production - 1 week
- Phase 1: Foundation - 4 weeks
- Phase 2: Cabin Vertical Slice - 4 weeks
- Phase 3: Raven System - 2 weeks
- Phase 4: Whispering Woods - 3 weeks
- Phase 5: Core Systems - 2 weeks
- Phase 6: Remaining Areas - 6 weeks
- Phase 7: Ending & Polish - 3 weeks
- Phase 8: Asset Replacement - 3 weeks (optional, can do last)
- Phase 9: Final Testing - 2 weeks

---

## **Critical Success Factors**

**Scope Management:**
- Stick to the plan
- Don't add features mid-development
- Finish one thing before starting another
- Primitives first, polish later

**Playtesting:**
- Get feedback early and often
- Watch people play without helping
- Listen to what they struggle with

**Atmosphere is King:**
- Lighting, fog, and audio are 80% of the experience
- Spend extra time on these
- Simple geometry + great atmosphere > complex geometry + poor atmosphere

**Performance:**
- Test on lower-end hardware
- Optimize as you go, don't save it all for the end
- Consistent frame rate > high frame rate with drops

**Narrative:**
- Voiceover quality matters—invest here
- Subtitles are non-negotiable
- Show, don't tell (environmental storytelling)

**Backup Everything:**
- Use version control (git)
- Commit often
- Back up to cloud (GitHub, GitLab, etc.)

---

## **Potential Pitfalls & Solutions**

**"The raven AI is too complex"**
- Solution: Use simple waypoint system, not full pathfinding
- Scripted sequences are fine

**"Areas feel empty"**
- Solution: Increase fog density, add more ambient audio
- Loneliness is the theme—embrace it

**"Players get lost"**
- Solution: Raven guides, whispers guide, add subtle visual cues
- Test with people unfamiliar with the game

**"Performance tanks with too many trees"**
- Solution: Use instancing, reduce tree count, add LOD

**"Voiceover is expensive"**
- Solution: Start with text-to-speech, replace later
- Or reduce line count, make each line more impactful

**"Running out of time"**
- Solution: Cut the 5th area (Echoing Glade), go straight to ending
- Or reduce embers per area from 5-7 to 3-4

---

## **Post-Launch Considerations**

**Potential Updates:**
- Add New Game+ with alternate narration
- Add photo mode
- Add accessibility options (text size, UI scale)
- Add achievements (if on Steam)
- Fix bugs reported by players

**Community:**
- Engage with players on itch.io/Steam forums
- Share development insights
- Thank players for feedback

