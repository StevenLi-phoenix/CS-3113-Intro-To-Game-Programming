<h2 align=center>第01周</h2>

<h1 align=center>你好，Raylib！</h1>

<h3 align=center>V 弓月，帝国年 MMXXV</h3>

<p align=center><strong><em>每日之歌</strong>: <a href="https://youtu.be/Soy4jGPHr3g?si=3OzzGXAaF8yi1u8q"><strong><u>テトリス (Tetris feat. 重音テト [Kasane Teto])</u></strong></a> by 柊マグネタイト (Hiiragi Magnetite) (2024).</em></p>

---

## 章节

1. [**什么是游戏编程？**](#1)
2. [**为什么选择raylib？**](#2)
3. [**你好，Raylib！**](#3)
    1. [**`main`**](#3-1)
    2. [**`initialise`**](#3-2)
    3. [**`processInput`**](#3-3)
    4. [**`update`**](#3-4)
    5. [**`render`**](#3-5)
    6. [**`shutdown`**](#3-6)

---

<a id="1"></a>

## 什么是游戏编程？

让我们首先解决一个我在任何人问我关于这门课时最常得到的问题：为什么我们不使用Unity / Unreal / 字面上任何其他免费提供给我们的行业标准游戏引擎？那不是启动游戏职业生涯的最佳方式吗？

是也不是。确实，很多视频游戏开发者通过预构建引擎开始他们的职业生涯。使用它们的全部吸引力在于您不必担心在屏幕上渲染像素并使其交互的细节。冒着听起来有点居高临下的风险（这不是我的本意——我在Unity中编程过很多），您可以称这种学习方式为"带训练轮的游戏开发"。这没有什么错！很多时候，开发者有强烈的愿景，无论哪种工具能帮助他们最有效地实现那个目标，都将是最明显的选择。老实说，只要我们能从中得到好游戏，谁在乎呢？

那么，为什么_我们_不那样做呢？

考虑这门课程的名称："游戏编程入门"。您如何定义游戏编程？或者，换个说法，游戏开发者负责什么？这不是一个有直接答案的直接问题，特别是对于Unity和Unreal这样的工具。如果我要冒险给出一个答案，那将是以下内容：

1. 游戏交互性的实现。
2. 游戏图形的实现。
3. 游戏物理的实现（如果有的话）。

在我看来，后两者特别重要，而且比您想象的要复杂得多。作为21世纪的公民，我们往往认为图形是理所当然的，因为我们随时随地都能看到它们。我们习惯了与图像交互的拖放模型，在这种模型中，我们甚至不需要考虑我们屏幕下的液晶所需的资源，就能在我们想要的地方显示我们想要的内容。考虑到在幕后，所有这些最终都会被分解为机器代码，这个过程中有相当多我们（有意地）根本没有考虑的内容。

物理也是一个比表面看起来更复杂的话题。当我们说一个游戏有非常沉浸式的移动或控制时，我们_确切地_在说什么？一般来说，当某些东西感觉_真实_时，我们会这样说——也就是说，类似于我们在现实世界中与之交互的物理定律。它对我们来说很熟悉，所以它让我们忘记，暂时地，我们正在玩游戏。唤起这种感觉并不容易，因为机制是极其复杂的。模拟一个物体在空间中的移动方式，包括它对外部刺激（如空气阻力等）的反应，真的很困难！我们不会想到它，因为我们真实地生活在其中，但当轮到告诉数字的东西以同样的方式行动时，我们突然在谈论复杂程度高出几个数量级的东西。

我选择在这门课中不使用预构建引擎的原因是，它们往往从开发者那里夺走了对这两件事的责任——或者至少将它们抽象化到琐碎的程度。当然，这并不是说您_不能_在预构建引擎中要求更精细的控制——您可以（特别是在Unreal中），但在那种情况下，几乎就像您在"逆着"这些软件的方向工作。意思是：如果您只是要重新编程Unity处理事物的方式，为什么还要使用Unity？这种事情。

那么，这意味着什么？通过选择不使用预构建引擎，我们实际上是在选择_自己构建引擎_。没错：从头开始，我们将使用一个可爱的小库叫**raylib**来构建我们自己的定制游戏引擎，能够按照_我们_想要的方式渲染图形，按照_我们_想要的方式处理用户输入，并按照_我们_想要的方式模拟物理。在我看来，这种方法的好处远远超过我们不使用预构建引擎可能有的任何"劣势"：

1. 您在做一些您可以称为自己的事情时获得大量编程练习。
2. 您可以立即用您本学期将编程的任何游戏扩展您的简历，（正确地）声明它是在您自己的定制引擎中完成的。
3. 大多数[**专业开发者**](https://gist.github.com/raysan5/909dc6cf33ed40223eb0dfe625c0de74)使用他们自己的定制引擎，当然，他们自己构建这些引擎。
4. 在这门课之后，掌握Unity和/或Unreal将是小菜一碟。（精通它们？那是另一回事。）

如果您同意我的推理，或者至少如果您不会在今天之后退课，那么我欢迎您。我迫不及待地想看到您制作什么。

<br>

<a id="2"></a>

## 为什么选择raylib？

您们中更了解情况的人可能知道，我过去使用一个[**叫做OpenGL的旧的、粗糙的库版本**](https://github.com/sebastianromerocruz/CS3113-intro-to-game-programming/tree/main?tab=readme-ov-file#cs3113-introduction-to-game-programming)来教授这门课程。实际上我们仍然会使用OpenGL，但以一种稍微不同、更受控制的方式。[**OpenGL**](https://www.opengl.org/)是一个相当著名的库，它直接与您的显卡交互，以创建高保真图形，开销比Unity甚至Unreal都要小得多。我从那种模式中切换出来的具体原因并不重要，但要点是，随着学生开始使用更新的设备，兼容性问题基本上成为了常态，这真的干扰了课程的重点。基本上，更多的时间被花在弄清楚为什么设置不工作上，而不是实际工作在游戏本身上。

所以，一个前学生推荐了[**raylib**](#https://www.raylib.com/index.html)，用他们自己的话说：

> _...是一个享受视频游戏编程的编程库；没有花哨的界面，没有视觉助手，没有图形界面工具或编辑器...只是以纯斯巴达程序员的方式编码。_
> 
> — 官方raylib网站。

<a id="fg-1"></a>

<p align=center>
    <img src="https://raw.githubusercontent.com/raysan5/raylib.com/master/images/raylib_architecture_v5.5.png">
    </img>
</p>

<p align=center>
    <sub>
        <strong>图I</strong>: raylib的整个架构 (<a href="https://github.com/raysan5/raylib/wiki/raylib-architecture"><strong>源</strong></a>)。
    </sub>
</p>

如您所见，raylib本身构建在OpenGL之上，所以我们仍然对我们的机器有相当精细的控制级别，只是有更友好的程序接口。我喜欢它的其他事情是：

- 设置起来轻而易举。
- 它是[**高度模块化**](https://github.com/raysan5/raylib/wiki/raylib-architecture)的。我们喜欢这一点。
- 它是开源的。我们_爱_这一点。
- 它被移植到[**阳光下的每种语言**](https://github.com/raysan5/raylib/blob/master/BINDINGS.md#raylib-bindings-and-wrappers)。我们真的只使用C版本，因为C/C++往往是行业的标准。

总的来说，虽然有一些我希望它做得不同/更接近原始OpenGL做法的事情，但它似乎是一个真正小而强大的库——对于我们本学期将要涵盖的内容来说绰绰有余。

<br>

<a id="3"></a>

## 你好，Raylib！

要开始使用这个库，让我们分析使用raylib显示一个米白色屏幕所需的代码，并逐部分分解它：

<a id="cb-1"></a>

```cpp
#include "raylib.h"

// Enums
enum AppStatus { TERMINATED, RUNNING };

// Global Constants
constexpr int SCREEN_WIDTH  = 800,
              SCREEN_HEIGHT = 450,
              FPS           = 60;

// Global Variables
AppStatus gAppStatus   = RUNNING;

// Function Declarations
void initialise();
void processInput();
void update();
void render();
void shutdown();

// Function Definitions
void initialise()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Hello raylib!");

    SetTargetFPS(FPS);
}

void processInput() 
{
    if (WindowShouldClose()) gAppStatus = TERMINATED;
}

void update() {}

void render()
{
    BeginDrawing();

    ClearBackground(RAYWHITE);

    EndDrawing();
}

void shutdown() 
{ 
    CloseWindow(); // Close window and OpenGL context
}

int main(void)
{
    initialise();

    while (gAppStatus == RUNNING)
    {
        processInput();
        update();
        render();
    }

    shutdown();

    return 0;
}
```

<p align=center>
    <sub>
        <strong>Code Block I</strong>: The contents of this week's <a href="main.cpp"><strong>source code</strong></a>.
    </sub>
</p>

To run the following code (assuming you have its [**corresponding `makefile`**](makefile) in the same folder), simply run the following command in your terminal/command line:

```bash
make run
```

And voilà!

<a id="fg-2"></a>

<p align=center>
    <img src="assets/hello-raylib.png">
    </img>
</p>

<p align=center>
    <sub>
        <strong>Figure II</strong>: Fascinating, I know.
    </sub>
</p>

<a id="3-1"></a>

### `main`

As with any program, we should look at its driver function if we want to get a good idea of how it works on a high-level. In the case of video games, this tends to take the shape of something called a **game loop**:

<a id="cb-2"></a>

```cpp
int main(void)
{
    initialise();

    while (gAppStatus == RUNNING)
    {
        processInput();
        update();
        render();
    }

    return 0;
}
```

<p align=center>
    <sub>
        <strong>Code Block II</strong>: Our invariable `main` function. With maybe a single exception this semester, this function should/will never change.
    </sub>
</p>

[**Micheal Stutz**](https://en.wikipedia.org/wiki/Video_game_programming#Game_structure) gives us a slightly more specific, pseudo-code definition:

```
while (user does not exit)
    check for user input
    run AI
    move enemies
    resolve collisions
    draw graphics
    play sounds
end while
```

Essentially, what we're seeing here is that, in order to run a game, you must:
1. **Initialise it**, i.e., setting up the screen, clearing the memory buffers, etc..
2. Until the user quits, **run it**. This involves:
    1. **Getting input from the user**, which may come from a keyboard, a mouse, a controller, etc.. We then _record_ this input somewhere to use it later (if there was any input at all).
    2. **Process the game's current frame**, which is a catch-all way of saying: "make everything do what it's supposed to do under the current circumstances". The player pressed `A` to jump in step 2.1? It's time to make the character jump. Did two enemies collide, causing them to walk in opposite directions? Time to change their direction vectors. So on and so forth. In essence, most of your game logic will go in here.
    3. Whatever it is that happened this frame **render/draw it on the screen**. This usually involves clearing the previous frame and asking the GPU to draw the new one, complete with whichever new positions/rotations/scales we ended up with after step 2.2.

This is important, if only because you will see this pattern again and again in every game development environment you're in. What's even more important right now, though, is to remember to _not mix things up_. In other words, make sure that _all_ user input is handled by the `processInput` function, _all_ state updates happen in `update`, and _all_ drawing happens in `render`. This way we can keep everything nice a modularised while keeping up with the industry's best practices.

If you've never seen an [**enum**](https://en.wikipedia.org/wiki/Enumerated_type) before, the following might be a little confusing:

```c++
enum AppStatus { TERMINATED, RUNNING };

AppStatus gAppStatus = RUNNING;

void main()
{
    while (gAppStatus == RUNNING) 
    {
        // ...
    }
}
```

Behind the hood, the values of enums are actually integers, starting from 0 and incrementing by a step of 1. This means that if I defined the following enum:

```c++
enum Day { MON, TUES, WED, THUR, FRI, SAT, SUN };
```

`MON` actually has a value of `0`, `TUES` has a value of `1`, `WED` of `2`, and so on. You can see this in the following screenshot:

<a id="fg-3"></a>

<p align=center>
    <img src="assets/enums.png">
    </img>
</p>

<p align=center>
    <sub>
        <strong>Figure III</strong>: Enumerators under the hood.
    </sub>
</p>

In general, they are considered to be a better way of handling logic than booleans are, as they are easily readable and are not limited to only two values.

<a id="3-2"></a>

### `initialise`

The first function we find in our `main` is that of `initialise`. As previously alluded to, this function takes care of a lot of the "housecleaning" necessary for your game to run smoothly. In general, this will involve the following:

1. **Initialising a window and OpenGL context**. I mean this literally, too. Computers don't just generate windows just because we click on icons. We have to _tell_ our operating system, and our graphics card by extension, to dedicate a portion our the screen to what we want to display. In raylib, this is relatively simple, using the following line:
    ```cpp
    InitWindow(
        SCREEN_WIDTH,   // in pixels
        SCREEN_HEIGHT,  // in pixels
        "Hello raylib!" // text shown on the title of the window
    );
    ```
    Keep in mind that [**1 pixel is equivalent to about 26.5 mm**](https://www.unitconverters.net/typography/pixel-x-to-centimeter.htm).
    - You can think of an **OpenGL context** as a structure that stores the state, functions, and resources needed for rendering graphics using OpenGL.
2. **Set the game's parameters**. This can be a number of things, but for now, we're only setting the game's frame rate. We'll explore frame rates, what they actually do and how they work later in the semester, but for now, all you need to know is that this is how you set it:
    ```cpp
    SetTargetFPS(FPS);
    ```
3. **Initialising the game's camera**. We won't do that for a while, but this is certainly when we would do it.
4. **Initialising any game objects necessary for the scene**. Loading screens are basically there to give your game time to put everything that is supposed to be in your game where and how its supposed to be. Before any updates happen, we first have to make sure that they exist.

<a id="3-3"></a>

### `processInput`

Our game is pretty empty at the moment, so all we have in terms of interactivity is the ability to close the window. That is precisely what this selection statement is checking for:

```cpp
void processInput() 
{
    if (WindowShouldClose()) gAppStatus = TERMINATED;
}
```

According to the official [**raylib docs**](https://www.raylib.com/cheatsheet/cheatsheet.html), `WindowShouldClose` will...

```c
// Check if application should close (KEY_ESCAPE pressed or windows close icon clicked)
bool WindowShouldClose(void);
```

Now, remember that `processInput` is being checked _every single frame_, so this function is constantly _listening_ for the user to perform either of those actions. In general, that will be the case for all user input functions, though there are some important distinctions we will have to consider when we get to that point in the course.

Notice, too, that _if_ the player does close the window, **`processInput` does not close the game itself**—it only marks it "eventual" termination. Why? Remember—`processInput` must do only that: register a user's input and keep track of it. It'll up to other functions within your game to do something with that input.

<a id="3-4"></a>

### `update`

Since there is no actual game to speak of in this current application of ours, it makes sense for `update` to be empty; this function is the one to update all game logic given the game's current state and the user's input. We have neither, so we can't do much.

<a id="3-5"></a>

### `render`

`render`, on the other hand, still has some tasks to perform, in spite there not being much to actually draw. Remember that, fundamentally, raylib and OpenGL are _computer graphics libraries_. Their sole job is to take pixel data and run it through your GPU in order to display it on the screen. That means that even a simple job like a screen with a solid colour has to be explicitly requested.

In general, we will always begin our `render` function with the following line:

```C
void BeginDrawing(void);  // Setup canvas (framebuffer) to start drawing
```

And end it with the following line:

```C
void EndDrawing(void);  // End canvas drawing and swap buffers (double buffering)
```

What exactly does this mean? Consider the following question: what is the most effective way to draw data onto the screen?

1. Bit-by-bit? or...
2. By chunks?

Loading and rendering data is a fairly expensive operation, so your computer will usually load a sizable chunk from the source to render instead of pulling a individual bits. A **buffer** is essentially that chunk of data, is reserved by your computer to do basically anything—play music, videos, read files, you name it. In terms of computer graphics, what our computers do is that they _load the entire image to be drawn into a buffer (the `framebuffer` in our case), and **only then**` does it draw it on screen_. [**Gulraiz Noor Bari**](https://medium.com/@gulraiznoorbari/buffer-swapping-v-sync-explained-a-beginners-journey-into-the-c-big-boys-league-0a5b53920152) on Medium gives a really good analogy to what happens next:

> Imagine you’re flipping through a sketchpad to show someone your animation. Rather than drawing on the same page they’re looking at (which would look messy), you draw on the next page while they view the current one. Once you’re done, you flip the page — smooth transition, no mess. That’s double buffering.
>
> When rendering graphics (especially with OpenGL), you’re not drawing directly to the screen. Instead, you use two frame buffers:
>
> - **Back Buffer**
> - **Front Buffer**

The buffer that we are filling up during the course of `render` is the **back buffer**, and the frame being currently shown on screen is the **front buffer**. What we meant by _buffer swapping_ is literally flipping them like you would between pages of an animation book:

> 1. The newly drawn frame (in the back buffer) becomes visible.
> 2. The old frame (in the front buffer) is now free to be drawn on for the next update.

<a id="3-6"></a>

### `shutdown()`

There won't be much here for a while besides the following line:

```cpp
void shutdown() 
{ 
    CloseWindow(); // Close window and OpenGL context
}
```

Though eventually we will be releasing some manually allocated data as well.