# NYU Tandon School of Engineering

## CS-UY 3113 Spring 2025

### Project 3: Lunar Lander!

**Due:** 11:59pm, Saturday, Oct 25th 2025

---

### Submission instructions

- You must use delta time, the Entity class, and fixed time steps in this assignment.
- You should submit the link to your repo as a comment on Brightspace.
- You should also push that same version to your GitHub account. Note that any commits done after the deadline will be ignored.
- Do not use any functionality that we have not learned in class unless I have explicitly approved of it.

The `main.cpp` file you submit should contain a header comment block as follows:

```cpp
/**
* Author: [Your name here]
* Assignment: Pong Clone
* Date due: 2025-10-13, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/
```

No late submissions will be accepted.

---

### Requirement 1: Player Falls With Gravity (25%)

The player should fall according to an acceleration of gravity that you choose. Traditional Lunar Lander games use a small gravity value to simulate outer space, but you may pick any value that fits your theme.

### Requirement 2: Moving With Acceleration (25%)

- Moving left or right should change the player's acceleration, not velocity.
- When the player releases the movement key, the ship should drift briefly before coming to a stop.
- Do not directly update `m_movement` (or your equivalent variable) ⚠️
- You should only modify the values in `m_acceleration`.

### Requirement 3: Mission Failed / Mission Accomplished (25%)

- If the player touches any forbidden area of the map → display a "Mission Failed" message and end the game.
- If the player successfully lands on a winning platform → display a "Mission Accomplished" message and end the game.
- Include at least one moving platform that the player can interact with to either win or lose.

### Requirement 4: Fuel Mechanic (25%)

- Implement a fuel system.
- Each time the player presses a key to change acceleration, fuel should be consumed.
- When the fuel runs out, acceleration keys should stop working.
- Include a UI element that displays the remaining fuel.

---

### Tips

- Focus first on getting your ship's movement to work correctly because this is the most challenging part of the project.
- Once movement works smoothly, start building out the rest of the world (collision, platforms, etc.).
- Check out the original Lunar Lander for inspiration and a better sense of gameplay feel.
- Observe how its UI displays movement, velocity, and acceleration to understand its physics system.

### Extra Credit

Add an animation using a sprite sheet for your falling object (e.g. flame exhaust, ship engine flicker, or crash explosion).

---

### Submission

Push your code to GitHub repo and submit the link to your GitHub repo as a comment. Please make sure that it is a public repo and clearly identify which is your Project 3: Lunar Lander folder and which is your project 2. Make sure to submit all the files that is necessary to compile and run your program.

**Due on:** Oct 27, 2025 11:59 PM

Available on Oct 10, 2025 12:01 AM. Access restricted before availability starts.
