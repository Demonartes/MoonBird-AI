# 🌙 MoonBird-AI

A 2D arcade game inspired by Flappy Bird, developed in **C++** using the **EasyX** graphics library.

Unlike a traditional Flappy Bird clone, MoonBird-AI introduces a lunar environment, custom artwork, sound effects, and an experimental **Q-Learning AI** capable of learning how to play the game autonomously.

---

## ✨ Features

- Smooth 2D gameplay
- Lunar-themed graphics
- PNG sprites with alpha transparency
- Sound effects
- Dynamic difficulty
- Score and high-score system
- Main menu and Game Over screen
- Experimental Q-Learning AI
- Persistent AI training (Q-table)

---

## 🛠 Technologies

- C++
- EasyX
- Windows API
- Q-Learning
- Visual Studio 2022

---

## 🎮 Controls

| Key | Action |
|------|--------|
| Space | Jump |
| R | Restart |
| Esc | Return to menu |

---

## 🤖 AI Mode

MoonBird-AI includes an experimental implementation of the **Q-Learning** reinforcement learning algorithm.

The AI observes the game state, selects actions using an exploration/exploitation strategy, receives rewards based on its performance, and continuously updates its Q-table. Over multiple training sessions, it gradually improves its ability to navigate through obstacles.

---

## 📁 Project Structure

```text
MoonBird-AI
│
├── assets
│   ├── images
│   └── sounds
│
├── screenshots
│
├── src
│
├── README.md
├── LICENSE
├── .gitignore
└── MoonBird-AI.sln
```

---

## 🚀 Getting Started

1. Open the solution in Visual Studio 2022.
2. Build the project.
3. Run the executable.
4. Press **Space** to start playing.

---

## 📈 Future Improvements

- Animated sprites
- Multiple difficulty levels
- Better reinforcement learning model
- Additional obstacles
- Background animations
- Cross-platform support

---

## 📄 License

This project is released under the MIT License.