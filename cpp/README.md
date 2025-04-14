# 🎮 BreakOut Game

<div align="center">

[![C++](https://img.shields.io/badge/C%2B%2B-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white)](https://en.cppreference.com/)
[![SDL2](https://img.shields.io/badge/SDL2-FFD700?style=for-the-badge&logo=SDL&logoColor=black)](https://www.libsdl.org/)
[![CMake](https://img.shields.io/badge/CMake-064F8C?style=for-the-badge&logo=cmake&logoColor=white)](https://cmake.org/)
</div>

## 📝 Description

A classic arcade-style BreakOut game built with modern C++ and SDL2. Break all the bricks with your ball while controlling the paddle to prevent the ball from falling! Features multiple brick types, progressive difficulty, and high score tracking.

## 🖼️ Game Screens

<div align="center">

### 🎮 Start Screen
![Start Screen](./screenshots/start_screen.png)

### 🎲 Playing Screen
![Playing Screen](./screenshots/playing_screen.png)

### 🏆 Win Screen
![Win Screen](./screenshots/win_screen.png)

### ❌ Game Over Screen
![Game Over Screen](./screenshots/game_over_screen.png)

</div>

## 🎯 Game Features

- 🏓 Smooth paddle controls
- 🔴 Multiple brick types with different hit points
- ⚡ Dynamic ball physics with speed progression
- 🎨 Clean visual design with animations
- 🏆 Score system and high score tracking
- 🎮 Multiple game states (Title, Playing, Win, Game Over)
- 💾 Persistent high score storage

## 🎮 Controls

| Key | Action |
|-----|--------|
| ⬅️ Left Arrow | Move paddle left |
| ➡️ Right Arrow | Move paddle right |
| ⏎ Space | Start/Restart game |
| ❌ Escape | Exit game |

## 🧱 Brick Types

| Color | Hits Required | Points |
|-------|---------------|--------|
| 🟢 Green | 1 | 100 |
| 🟠 Orange | 2 | 200 |
| 🔴 Red | 3 | 300 |

## 🎯 Game Mechanics

- Ball speed increases as bricks are destroyed
- Different brick types require multiple hits
- Score based on brick type and destruction
- High score persistence between sessions
- Dynamic paddle collision angles

## 🔄 Game States

```mermaid
graph TD
    A[Title Screen] -->|Space| B[Playing]
    B -->|All Bricks Destroyed| C[Win Screen]
    B -->|Ball Lost| D[Game Over]
    C -->|Space| A
    D -->|Space| A
    
    subgraph Title Screen
    A1[Show High Score]
    A2[Animated Title]
    A3[Press Space Prompt]
    end
    
    subgraph Playing
    B1[Move Paddle]
    B2[Ball Physics]
    B3[Break Bricks]
    B4[Update Score]
    B5[Increase Ball Speed]
    end
    
    subgraph Win Screen
    C1[Show Final Score]
    C2[Update High Score]
    C3[Victory Message]
    end
    
    subgraph Game Over
    D1[Show Final Score]
    D2[Game Over Message]
    end
```

## 📁 Project Structure

### 🎯 Core Files
- `main.cpp` - 🎮 Game loop and core logic
- `window.cpp/h` - 🖼️ SDL window management
- `entity.cpp/h` - 🎲 Game objects (paddle, ball, bricks)

### 🛠️ Support Files
- `vector2.cpp/h` - ➡️ 2D vector mathematics
- `rectangle.cpp/h` - 📦 Collision shapes
- `colour.cpp/h` - 🎨 Color management
- `key_event.h` - ⌨️ Input handling

## 🚀 Building the Game

### Prerequisites
- 📌 C++20 compiler
- 📌 SDL2 library
- 📌 CMake 3.10+

### Build Steps

```bash
# 1. Clone the repository
git clone https://github.com/Prince-Patel84/asm_c_cpp/tree/develop/cpp

# 2. Create build directory
mkdir build
cd build

# 3. Generate build files
cmake ..

# 4. Build the game
cmake --build .
```

## 🎮 How to Play

1. 🚀 Launch the game
2. 🏓 Use left and right arrows to move the paddle
3. 🎯 Bounce the ball to break all bricks
4. 🏆 Try to clear all bricks without losing the ball
5. 📊 Score points based on brick type
6. 💾 Beat your high score!

## 🔧 Technical Implementation

### Physics System 🎯
- Accurate ball bouncing mechanics
- Precise collision detection
- Dynamic paddle reflection angles
- Progressive ball speed system

### Rendering Engine 🎨
- Hardware-accelerated graphics
- Smooth animations
- Efficient frame rendering
- Dynamic color transitions

### Game State Management 🎮
- Title screen with animations
- Playing state with score display
- Win condition with final score
- Game over state with restart option

### Score System 📊
- Points based on brick type
- High score tracking
- Persistent storage
- Visual score display

## 🛠️ Future Enhancements

- [ ] 🔊 Sound effects
- [ ] 🎯 Multiple levels
- [ ] ⚡ Power-ups
- [ ] 💖 Lives system
- [ ] 🎨 Custom themes

## 👥 Team Motion Minds

<div align="center">

### 🎮 Game Development Team

| Name | Student ID |
|------|------------|
| Prince Patel | 202401151 |
| Vishwa Prajapati | 202401163 |
| Dhruv Patel | 202401142 |

Built with 💖 and lots of 🎮
</div>

---

<div align="center">

### 🌟 Star this repository if you find it helpful!

</div>