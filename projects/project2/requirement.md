NYU Tandon School of Engineering
CS-UY 3113 Spring 2025
Project 2: Pong Clone!
Due: 11:59pm, Monday, Oct 13th 2025
Submission instructions
You must use delta time in your animations. Do not need to use the Entity class in this assignment, but you may if you'd like.
You should submit the link to your repo as a comment on Brightspace.
You should also push that same version to your GitHub account. Note that any commits done after the deadline will be ignored.
Do not use any functionality that we have not learned in class unless I have explicitly approved of it.
The main.cpp file you submit should contain a header comment block as follows:
/**
* Author: [Your name here]
* Assignment: Pong Clone
* Date due: 2025-10-13, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/
No late submissions will be accepted.

Requirement 1: Paddles / Players / Walls (30%)
There needs to be a paddle on each side that can move independently to the other.
The ball needs to bounce off of the paddles.
The paddles should not be able to go off the top or bottom of the screen.
Both players should be allowed to use the keyboard. For instance, player 1 could use the W and S keys, while player 2 would use the Up and Down arrows.
All objects in your scene must be textured.
The ball needs to bounce off the top and bottom of the screen.
Requirement 2: Single-Player Switch (25%)
If the player presses the t-key, one of the paddles should switch from player-controlled to a simple up-and-down motion. In other words, we are simulating the player switching from 2-player mode to 1-player mode. Make sure that the user can't use their keys to move the paddle when in 1-player mode.
Requirement 3: Have an option to choose number of balls (30%)
Allow the player to press number keys from 1-3 and generate that number of balls on the screen (do NOT generate more than 3 balls). Player should be able to change the number of balls any time during the game.
HINT: Generate every object that could be on the screen at any given time, but any object that isn't currently being used should not be rendered nor updated.
Requirement 4: Game Over (15%)
The game should stop when someone wins or loses (i.e. when the ball hits a wall on the left or right).
Tips
To keep things simple, I'd recommend using box-to-box collision detection only.
Work object by object, mechanic by mechanic, and test often. This goes for all programming, but if you try finishing all objects in one go, you are bound to miss something that have a hard time finding it.
Extra Credit
Have fun with it and have a theme. Instead of square paddles and a ball, have other kinds of objects/images with the same mechanics. It doesn't need to be super polished; anything that looks fun is welcome.

Have an endgame UI message. When someone wins, you can show an image of text saying who won.

Submit
Push your code to GitHub repo and submit the link to your GitHub repo. Please make sure that it is a public repo and clearly identify which is your project folder and which is your project 2.
Due on Oct 13, 2025 11:59 PM
Available on Sep 26, 2025 12:01 AM. Access restricted before availability starts.

---

Implementation Notes
- `main.cpp` includes the mandated header comment block and uses delta time for all physics updates.
- Paddles, balls, background, and UI elements are textured; assets live under `assets/`.
- Pressing `T` switches the right paddle into AI control; additional keys (`Y`, `U`) change difficulty presets while disabling manual movement.
- Number keys `1`–`3` update the active ball count and enforce the three-ball cap; `Q`/`E` shortcuts respect the same bounds.
- When any ball exits a goal wall, the game transitions to the game-over state and stops play until restarted.
