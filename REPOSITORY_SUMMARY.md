# Repository Summary: CS-3113 Introduction to Game Programming

## Overview
This repository contains course materials for **CS-3113 Introduction to Game Programming** at **New York University Tandon School of Engineering**, taught by Professor Sebastián Romero Cruz. The course focuses on building 2D games from scratch using C++ and the raylib library, emphasizing understanding core game programming concepts rather than using pre-built engines.

## Course Philosophy
Rather than using industry-standard engines like Unity or Unreal, this course teaches students to build their own custom game engine. This approach provides:
- Deep understanding of graphics rendering and game loops
- Comprehensive knowledge of physics simulation
- Strong foundation in game programming fundamentals
- Portfolio projects built with custom technology
- Easier transition to pre-built engines later

## Technical Stack
- **Language**: C++
- **Graphics Library**: raylib (OpenGL wrapper)
- **IDE**: Visual Studio Code (recommended)
- **Platforms**: macOS, Windows, Linux
- **Prerequisites**: CS-UY 2124 Object-Oriented Programming (C- or better)

## Course Structure

### Grading Breakdown
- **Projects** (90%): 5 programming assignments
  - Project 1: Draw a Simple 2D Scene (10%)
  - Project 2: Pong (10%)
  - Project 3: Lunar Lander (15%)
  - Project 4: Rise of The AI (25%)
  - Project 5: Students' Choice (30%)
- **Classwork** (10%): 5-7 group assignments

### Project Schedule
- Projects due every ~2 weeks on Saturdays at 11:59 PM
- Late penalty: 10 points per day, max 2 days late
- Extension policy varies by project
- Extra credit available (5% final grade bonus for completing 3+ extra credit portions)

### Topics Covered (Planned)
1. **Week 1**: The Basics - Hello, Raylib!
2. **Week 2**: Moving in video games (Matrices, Transformations)
3. **Week 3**: Art integration (Delta Time, Textures)
4. **Week 4**: User input and collision detection
5. **Week 5**: Sprite animation, text, and entity classes
6. **Week 6**: Physics (Gravity, entity collisions)
7. **Week 7**: Audio and basic AI
8. **Week 8**: Map building and scenes
9. **Week 9**: Special effects and shaders
10. **Week 10**: Playtesting and publishing

## Current Repository Contents

### Documentation
- **README.md**: Complete course syllabus and policies
- **SET_UP.md**: Installation instructions for raylib on macOS and Windows
- **LICENSE**: MIT License

### Code Examples
- **lectures/01-introduction/**: First week's materials
  - Basic raylib "Hello World" program
  - Cross-platform Makefile
  - Game loop structure demonstration

### Key Programming Concepts Introduced

#### Game Loop Structure
The fundamental pattern used throughout the course:
```cpp
int main(void) {
    initialise();
    while (gAppStatus == RUNNING) {
        processInput();  // Handle user input
        update();        // Process game logic
        render();        // Draw to screen
    }
    shutdown();
    return 0;
}
```

#### Core Functions
- **initialise()**: Set up window, OpenGL context, game parameters
- **processInput()**: Handle keyboard, mouse, and other input
- **update()**: Update game state, physics, AI
- **render()**: Draw graphics using double buffering
- **shutdown()**: Clean up resources

## Setup Requirements

### macOS
```bash
brew install cmake raylib pkg-config
```

### Windows (MinGW)
1. Download raylib precompiled binaries
2. Extract to `C:\raylib\`
3. Install MinGW/MSYS2 for `g++` and `make`

### Project Structure
```
project-name/
├── main.cpp
└── Makefile
```

## Academic Policies

### Academic Integrity
- No plagiarism tolerance (0 on project + academic report)
- Limited generative AI use (must ask permission and cite)
- Independent work required on all projects

### Accommodations
- Moses Center for Students with Disabilities support available
- Emphasis on accessibility importance for this course

### Communication
- Slack server for quick questions and course discussion
- Office hours available via Calendly scheduling
- Professional environment expectations

## Learning Objectives
Students will master:
- Vector mathematics and coordinate systems
- Sprite-based graphics and animation
- Collision detection algorithms
- Physics simulation (gravity, momentum)
- Basic AI for enemy behavior
- Audio integration
- User input handling
- Map building and level design
- Game state management

## Repository Structure
```
CS-3113-Intro-To-Game-Programming/
├── README.md                 # Course syllabus and information
├── LICENSE                   # MIT License
├── .gitignore               # Build artifacts exclusions
├── lectures/
│   └── 01-introduction/     # Week 1 materials
│       ├── README.md        # Lecture notes
│       ├── main.cpp         # Basic raylib example
│       ├── makefile         # Cross-platform build script
│       └── assets/          # Screenshots and images
└── resources/
    ├── SET_UP.md           # Installation guide
    └── sampleProject.zip   # Template project
```

## Next Steps for Students
1. Follow setup instructions in `resources/SET_UP.md`
2. Review Week 1 lecture materials in `lectures/01-introduction/`
3. Build and run the "Hello, Raylib!" example
4. Prepare for Project 1: Draw a Simple 2D Scene
5. Join course Slack server for communication

## Contact Information
- **Instructor**: Sebastián Romero Cruz (sebastian.romerocruz@nyu.edu)
- **Office Hours**: Available via Calendly
- **Course Assistant**: TBD

This repository serves as the foundation for a comprehensive journey into game programming, providing students with the knowledge and tools to create their own 2D games while understanding the underlying technology that makes modern games possible.