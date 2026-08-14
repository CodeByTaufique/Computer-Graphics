# 🎨 Computer Graphics

<p align="center">
  <img src="https://img.shields.io/badge/C%2B%2B-00599C?style=for-the-badge&logo=cplusplus&logoColor=white" alt="C++">
  <img src="https://img.shields.io/badge/OpenGL-5586A4?style=for-the-badge&logo=opengl&logoColor=white" alt="OpenGL">
  <img src="https://img.shields.io/badge/GLUT-Computer%20Graphics-blue?style=for-the-badge" alt="GLUT">
  <img src="https://img.shields.io/badge/CMake-Build%20System-064F8C?style=for-the-badge&logo=cmake&logoColor=white" alt="CMake">
</p>

<p align="center">
  <b>A collection of Computer Graphics implementations, laboratory exercises, transformations, animations, and OpenGL projects.</b>
</p>

---

## 📌 About

This repository contains my **Computer Graphics course implementations and projects**, developed primarily using **C++ and OpenGL**.

The repository covers fundamental computer graphics algorithms as well as practical OpenGL programming, including:

* 🖊️ Line and circle drawing algorithms
* 🔄 2D transformations
* 🎬 2D animation
* 🌸 Shape and object rendering
* 🚗 Interactive graphical scenes
* 🎮 Basic game-oriented graphics
* 🚆 Metro station simulation
* 🧱 OpenGL-based object construction
* 📐 Geometric transformations
* 🖥️ Interactive graphics using keyboard input

The goal of this repository is to document my learning journey throughout the **Computer Graphics course**, from fundamental rasterization algorithms to larger interactive OpenGL applications.

---

## 📚 Topics Covered

### 🖊️ Computer Graphics Algorithms

| Topic                     | Implementation                 |
| ------------------------- | ------------------------------ |
| DDA Line Algorithm        | `DDA.cpp`                      |
| Bresenham Line Algorithm  | `Bresenham.cpp`                |
| Midpoint Circle Algorithm | `midCircle.cpp`                |
| 2D Transformations        | `2dTransform.cpp`              |
| Object Translation        | Multiple implementations       |
| Rotation                  | Transformation examples        |
| Scaling                   | Transformation examples        |
| Animation                 | `Animation.cpp`, `Animate.cpp` |

---

### 🎨 OpenGL Rendering

The repository also contains several OpenGL-based graphical scenes and object implementations.

Examples include:

* 🏠 House rendering
* 🌺 Flower rendering
* 🚗 Car rendering
* 🚩 Flag rendering
* ⭐ Star rendering
* 🚤 Boat rendering
* 🎮 Interactive game scenes
* 🚆 Metro station simulation

---

## 🚆 Major Project — Smart Metro Station Simulation

One of the major projects in this repository is an interactive **Smart Metro Station Simulation** developed using C++ and OpenGL.

The project demonstrates how computer graphics concepts can be combined to create a dynamic real-world environment.

### ✨ Features

* 🚆 Animated metro train
* 🚉 Metro station environment
* 🚪 Train door animation
* 🚦 Signal system
* ☁️ Moving clouds
* 🐦 Animated birds
* 🌧️ Rain effect
* 🌙 Day/Night environment
* ⏰ Digital clock
* 🎮 Keyboard-controlled interactions

### 🎮 Controls

| Key | Action            |
| --- | ----------------- |
| `A` | Train Arrival     |
| `D` | Train Departure   |
| `N` | Toggle Night Mode |
| `R` | Toggle Rain       |

The project demonstrates practical applications of **translation, rotation, scaling, animation, object composition, keyboard interaction, and OpenGL rendering**.

---

## 🗂️ Repository Structure

```text
Computer-Graphics/
│
├── 2dTransform.cpp
├── Animate.cpp
├── Animation.cpp
├── Bresenham.cpp
├── Car.cpp
├── DDA.cpp
├── Flower.cpp
├── Game.cpp
│
├── House2.0.cpp
├── Lab1.cpp
├── Lab2.cpp
├── Lab3.cpp
├── Lab4.cpp
├── Lab5.cpp
├── Lab6.cpp
├── Lab7.cpp
├── Lab8.cpp
├── Lab9.cpp
│
├── Metro.cpp
├── Metro2.cpp
│
├── boat.cpp
├── check.cpp
├── flag.cpp
├── fourStars.cpp
├── house.cpp
├── main.cpp
├── midCircle.cpp
│
├── p.cpp
├── try.cpp
├── try1.cpp
├── try6.cpp
│
├── CMakeLists.txt
└── README.md
```

---

## 🛠️ Technologies Used

### Programming Language

**C++**

Used for implementing algorithms, graphical objects, animation logic, and interactive applications.

### Graphics Library

**OpenGL**

Used for rendering graphical objects and creating interactive 2D/3D scenes.

### Utility Toolkit

**GLUT / FreeGLUT**

Used for:

* Window creation
* Keyboard input
* Mouse interaction
* OpenGL context management
* Animation callbacks

### Build System

**CMake**

Used for managing and building compatible OpenGL projects.

---

## ⚙️ Getting Started

### 1. Clone the Repository

```bash
git clone https://github.com/CodeByTaufique/Computer-Graphics.git
```

```bash
cd Computer-Graphics
```

### 2. Install Requirements

Make sure the following are available on your system:

* C++
* OpenGL
* GLUT / FreeGLUT
* CMake
* A compatible C++ compiler

### 3. Build

If the project uses CMake:

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

You can also open individual `.cpp` files in an IDE such as:

* CLion
* Visual Studio
* Code::Blocks
* VS Code

---

## 💻 Platform

The programs can be configured for different platforms depending on the available OpenGL and GLUT libraries.

Tested/developed primarily with:

* 🍎 macOS
* 🪟 Windows
* 🐧 Linux

> OpenGL/GLUT configuration may differ between operating systems.

---

## 🧠 Learning Outcomes

Through these implementations, I practiced:

* Understanding the graphics pipeline
* Rasterization algorithms
* Pixel-based line generation
* Circle generation
* Coordinate systems
* 2D geometric transformations
* Translation
* Rotation
* Scaling
* Object composition
* OpenGL primitives
* Animation techniques
* Keyboard interaction
* Scene design
* Real-time graphical updates

---

## 🎯 Course Progression

```text
Basic Drawing
      │
      ▼
Line Algorithms
(DDA / Bresenham)
      │
      ▼
Circle Algorithms
(Midpoint Circle)
      │
      ▼
2D Transformations
      │
      ▼
Object Rendering
      │
      ▼
Animation
      │
      ▼
Interactive Scenes
      │
      ▼
OpenGL Projects
      │
      ▼
🚆 Smart Metro Station Simulation
```

---

## 📸 Project Showcase

You can add screenshots or GIFs of the projects here.

### Metro Station

<p align="center">
  <i>Smart Metro Station Simulation</i>
</p>

<!-- Add screenshot here -->

### Computer Graphics Algorithms

<p align="center">
  <i>DDA • Bresenham • Midpoint Circle • Transformations</i>
</p>

<!-- Add screenshots here -->

---

## 📖 References

Some of the concepts implemented in this repository are based on standard Computer Graphics learning materials and documentation.

* OpenGL Programming Guide
* OpenGL Documentation
* Computer Graphics course materials
* GeeksforGeeks Computer Graphics resources
* Stack Overflow discussions and programming examples

---

## 👨‍💻 Author

### Taufique

**Computer Science & Engineering Student**

Interested in:

* 💻 Software Development
* 🤖 Artificial Intelligence
* 🔐 Cybersecurity
* 🎨 Computer Graphics
* 🧠 Algorithms & Data Structures
* 🚀 Building practical projects

<p align="center">
  <a href="https://github.com/CodeByTaufique">
    <img src="https://img.shields.io/badge/GitHub-CodeByTaufique-181717?style=for-the-badge&logo=github" alt="GitHub">
  </a>
</p>

---

## ⭐ Support

If you find this repository useful for learning Computer Graphics, consider giving it a ⭐.

---

<p align="center">
  <b>Built with C++ & OpenGL ❤️</b>
</p>
