<h1 align=center>CS3113 <em>游戏编程入门</em></h1>

<h2 align=center>纽约大学坦登工程学院</h2>

<p align=center>
    <a href="https://www.learncpp.com/"><img src="https://img.shields.io/badge/Language-C%2B%2B-yellow"></img></a>
    <a href="https://code.visualstudio.com/"><img src="https://img.shields.io/badge/IDE-Visual%20Studio%20Code-blue"></img></a>
    <a href="https://www.opengl.org/"><img src="https://img.shields.io/badge/Library-raylib-white"></img></a>
</p>

---

### _章节_

1.  [**课程笔记**](#1)
2.  [**讲师和课程助教**](#2)
3.  [**课程信息**](#3)
4.  [**课程描述**](#4)
5.  [**学习目标**](#5)
6.  [**计算机/软件要求**](#6)
7.  [**评分方案**](#7)
8.  [**截止日期、延期和迟交政策**](#8)
    - [**项目时间表**](#8-1)
9. [**获取帮助和Slack**](#9)
10. [**Moses中心残疾声明**](#10)
11. [**学术诚信**](#11)
    - [**生成式AI**](#11-1)
    - [**处罚措施**](#11-2)

---

<a id="1"></a>

### _课程笔记_

1. [**基础知识: _你好，Raylib!_**](lectures/01-introduction)

<!-- 1. **Week 1**: The basics
    1.  [**Introduction**](lectures/introduction/)
    2.  [**Triangles**](lectures/triangle/)
2. **Week 2**: Moving in video games
    1.  [**You're getting old, Matrix**](lectures/matrices/)
    2.  [**Transformations**](lectures/transformations/)
3. **Week 3**: Applying art to our games
    1.  [**Delta Time**](lectures/delta-time/)
    2.  [**Textures**](lectures/textures/)
4. **Week 4**: Player input and basic collision detection
    1.  [**Player Input**](lectures/player-input/)
    2.  [**Collision Detection**](lectures/collision-detection/) 
5. **Week 5** Sprite Animation and Text and The Entity Class: 
    1.  [**Sprite Animation and Text**](lectures/sprites-and-text/)
    2.  [**Entities**](lectures/entities/)
6. **Week 6**: Physics 1 and Physics 2
    1. [**Physics: Gravity**](lectures/physics_1/)
    2. [**Physics: Colliding with Different Kinds of Entities**](lectures/physics_2/) 
7. **Week 7**: Music, sound effects, and basic enemy AI
    1. [**Music and Sound Effects**](lectures/sound-fx/)
    2. [**Basic Enemy AI**](lectures/enemy-ai/)
8. **Week 8**: Map and stage building
    1. [**Tilesets and Tilemaps**](lectures/maps/)
    2. [**Scenes**](lectures/scenes/)
9. **Weeks 9**: Special effects
    1. [**Effects**](lectures/fx/)
    2. [**Shaders**](lectures/shaders/)
10. **Week 10**: [**Playtesting and publishing your game**](lectures/playtesting/) -->

<sub>每周五课前发布。</sub>

<br>

<a id="2"></a>

### _讲师和课程助教_

[**Sebastián Romero Cruz**](https://github.com/sebastianromerocruz)  _(They / Them)_

| **邮箱**                                  | **办公时间**                                                                                                     |
|---------------------------------------------|----------------------------------------------------------------------------------------------------------------------|
| [**sebastian.romerocruz@nyu.edu**](mailto:src402@nyu.edu) | [**calendly**](https://calendly.com/profromerocruz) |

[**待定**]()

| **邮箱**                                  | **办公时间**                                                                                                     |
|---------------------------------------------|----------------------------------------------------------------------------------------------------------------------|
| [**待定**](mailto:) | 待定 |


<br>

<a id="3"></a>

### _课程信息_

- **先修课程**: [_**CS-UY 2124 面向对象编程（事实上的）**_](http://bulletin.engineering.nyu.edu/preview_course_nopop.php?catoid=15&coid=36541)（C-或更好成绩）
- **学分**: 3

<a id="4"></a>

### _课程描述_

一门以编程为重点的计算机游戏创作入门课程。主要使用二维精灵（sprite）编程，我们将研究和实验动画、物理、人工智能和音频。此外，本课程还探索变换的数学（二维和三维）以及它们的表示方法。

<br>

<a id="5"></a>

### _学习目标_

您将学习向量、坐标系统、精灵、动画、碰撞、物理、地图构建、简单敌人AI、音频和处理用户输入。这些内容将通过C++编程实现，并利用[**raylib**](https://www.raylib.com/)库，它本身是一个跨平台的OpenGL包装器。您将能够在您的引擎中创建简单的2D游戏，并接触3D游戏编程。

<br>

<a id="6"></a>

### _计算机/软件要求_

您需要使用Mac、Linux或Windows计算机。您不需要功能强大的机器，因为我们不会实现任何超级复杂的内容。虽然我推荐使用[**Visual Studio Code (VSCode)**](https://code.visualstudio.com/)，但您可以使用您最熟悉的编辑器。

您还需要一个[**Github**](https://github.com/)账户！

[**项目设置说明**](resources/SET_UP.md)

<br>

<a id="7"></a>

### _评分方案_

| **项目**                                                                | **百分比** |
|-------------------------------------------------------------------------|----------------|
| **_项目1_**: **绘制简单2D场景** | 10%            |
| **_项目2_**: **乒乓球**                   | 10%            |
| **_项目3_**: **月球登陆器**           | 15%            |
| **_项目4_**: **AI的崛起**         | 25%            |
| **_项目5_**: **学生自选**                               | 30%            |
| **_课堂作业_**                                                         | 10%            |

- **项目**（90%）
    - 大约每两周分配一次，这些是基于课程材料的编程项目，需要_独立完成_。
    - 如您所见，这门课程以项目为主。这意味着，虽然没有考试或测验，但每个项目的成绩对您的总体课程成绩有相对较大的影响。
    - 本课程假设您对C++语言有扎实的理解，包括语法、控制流、函数分解和面向对象编程。因此，我们不会根据代码质量给您评分。
    - 但是，由于raylib库相当广泛，而且有多种方法来使用它，您只能_使用我们在课堂上学过的结构_。例如，如果我们在讲座中定义了一个特定的类，并且在项目中要求使用它，**您必须使用它才能获得满分**。
    - 当然，您可以自由地进行自己的修改，并创建任何辅助函数/类/库，以最适合您的编程风格。如果您不确定是否可以在作业项目中使用某些内容，请先询问我们。
    - 每个作业都会有一个"额外学分"部分。如果您完成**至少3个项目**的额外学分部分，这将在学期末**为您的最终成绩加5%**。虽然5%可能看起来不多，但它可能意味着两个字母等级之间的差别。这在学期后期特别有帮助，因为届时项目会变得更长更复杂。

- **课堂作业**（10%）基于5-7个小组作业，顾名思义，需要在课堂上完成。为了获得学分，您需要：
    1. 到场。
    2. 在课程结束_之前_与您的团队完成作业。
    3. 仅使用课堂上学过的结构完成作业。
    4. 确保您的团队中至少有一人向教授展示完成的解决方案。

至于您的最终字母等级，将使用以下评分标准：

| **字母等级** | **A**  | **A-** | **B+** | **B** | **B-** | **C+** | **C** | **C-** | **D+** | **D** | **F** |
|------------------|--------|--------|--------|-------|--------|--------|-------|--------|--------|-------|-------|
| **分数范围**        | 93-100 | 90-92  | 87-89  | 83-86 | 80-82  | 77-79  | 73-76 | 70-72  | 67-69  | 60-66 | 0-60  |

**本课程没有额外的加分**，成绩**不调整曲线**。

<br>

<a id="8"></a>

### _截止日期、延期和迟交政策_

- 所有项目都在**周六晚上11:59**截止。
    - 迟交项目将**每天扣10分**。
    - 迟交项目在**2天**后将不被接受。
    - 晚上11:59截止意味着您的项目必须在那个时间_之前_成功推送到GitHub。这意味着要评分的版本必须在那个时间同时上传到Brightspace和GitHub。请至少在截止时间前一小时开始推送您的项目。
    - **晚1分钟**收到的项目被视为**迟交一天**。

- 如果上传项目时遇到任何问题，您必须在**截止日期前24小时**发邮件给我。
    - 虽然我经常查看邮件，但不要期望在周末或临近截止日期时收到回复。
    - 您的代码必须能够编译。无法编译的代码将得到0分。

- 您可以为**5个项目中的任意2个**申请延期。各项目的延期政策不同：
    - **项目1-3**：您可以通过在截止日期前至少72小时（3天）向Romero Cruz教授申请非正式延期。延期的确切长度将根据具体情况分配。
    - **项目4**：您_必须_联系[**学生权益办公室**]()申请此项目的延期，因为其截止日期与您开始最终项目的时间冲突。如果他们批准，那么我将毫无问题地批准。
    - **项目5**：您的最终项目在_最后一天课程_期间评分，届时您将向全班展示。这意味着理论上不可能延期。但是，如果出现紧急情况，我可以暂时给您一个未完成的成绩（`I`）并给您短期延期。这确实是最后的选择，所以请尽量避免。

<a id="8-1"></a>

#### 项目时间表

| 项目                 | 发布日期 | 截止日期      |
|-------------------------|--------------|---------------|
| **简单场景**        | 待定          | 待定           |
| **乒乓球**                | 待定          | 待定           |
| **月球登陆器**        | 待定          | 待定           |
| **平台游戏**          | 待定          | 待定           |
| **学生自选**    | 待定          | 待定           |

<br>

<a id="9"></a>

### _Getting Help and Slack_

If you are emailing me for help with your projects, upload your entire project to github and email me with the link (I need to see everything so I can help you). Do not email screenshots of your code. 

**Slack Server**: We will be using Slack to answer quick questions that you may have about the course throughout the semester; the join link will be provided during the first day of class. While I’m usually pretty lax in terms of behavior in our server, this server is still a university environment and should be treated as such. Be respectful to me, your course assistant, and to your fellow students. Please adhere to the following rules:

- Do not post your homework assignment code, or anybody else's, on this server. Doing so will have you automatically banned and flagged for plagiarism. You may, however, share small code blocks that don’t give away your implementation in order to ask questions.

- Please use your real first and last name as your name for easy identification.

- While we aim to be as active as possible on this server, we may not always have time to respond to a question. Please respect the team’s time as you wait for somebody to answer your question. As a student of this class, you should aim to try to answer your classmates' questions as well, instead of waiting for me to answer them every single time.

- This is a productivity server. While we encourage a relaxed atmosphere, let's stay on topic. For absolutely necessary off-topic content (it happens), post in `#off-topic`.

- Use `#concepts-help` to ask questions that pertain to the lecture material in general (i.e. not specific to a project).

- Use `#projects` to ask questions that pertain to the project concepts in general.

- Do NOT use this server to rant about your performance in the class. This is a professional environment, and so such behavior will result in a ban. If you would like to discuss your grades, schedule office hours with me.

- You may not invite any people outside of our class into this server.

<br>

<a id="10"></a>

### Moses Center Statement of Disability

If you are a student with a disability who is requesting accommodations, please contact New York University’s Moses Center for Students with Disabilities (CSD) at 212-998-4980 or **mosescsd@nyu.edu**​. You must be registered with CSD to receive accommodations. Information about the Moses Center can be found at [**www.nyu.edu/csd**](**www.nyu.edu/csd**)​. The Moses Center is located at 726 Broadway on the 3rd floor.

Accommodations matter a _lot_ for this class, so please don't hesitate.

<br>

<a id="11"></a>

### Academic Integrity

NYU School of Engineering Policies and Procedures on Academic Misconduct [**Student Code of Conduct**](https://engineering.nyu.edu/life-tandon/student-life/student-advocacy/student-code-conduct).

- **Introduction**: The Tandon School of Engineering encourages academic excellence in an environment that promotes honesty, integrity, and fairness, and students at the Tandon School of Engineering are expected to exhibit those qualities in their academic work. It is through the process of submitting their own work and receiving honest feedback on that work that students may progress academically. Any act of academic dishonesty is seen as an attack upon the School and will not be tolerated. Furthermore, those who breach the School’s rules on academic integrity will be sanctioned under this Policy. Students are responsible for familiarizing themselves with the School’s Policy on Academic Misconduct.

- **Definition**: Academic dishonesty may include misrepresentation, deception, dishonesty, or any act of falsification committed by a student to influence a grade or other academic evaluation. Academic dishonesty also includes intentionally damaging the academic work of others or assisting other students in acts of dishonesty. Common examples of academically dishonest behavior include, but are not limited to, the following:

    - **Cheating**: intentionally using or attempting to use unauthorized notes, books, electronic media, or electronic communications in an exam; talking with fellow students or looking at another person’s work during an exam; submitting work prepared in advance for an in-class examination; having someone take an exam for you or taking an exam for someone else; violating other rules governing the administration of examinations.

    - **Fabrication**: including but not limited to, falsifying experimental data and/or citations.

    - **Plagiarism**: intentionally or knowingly representing the words or ideas of another as one’s own in any academic exercise; failure to attribute direct quotations, paraphrases, or borrowed facts or information.

    - **Unauthorized collaboration**: working together on work that was meant to be done individually.

    - **Duplicating work**: presenting for grading the same work for more than one project or in more than one class, unless express and prior permission has been received from the course instructor(s) or research adviser involved.

    - **Forgery**: altering any academic document, including, but not limited to, academic records, admissions materials, or medical excuses.

<a id="11-1"></a>

#### Generative AI

The question on everybody's mind at this point is most likely this: can I use generative AI (gen AI) in this class? This short answer is that, most of the time, _no, you cannot_. The long answer is more nuanced, given the omnipresence and easy-of-use of tech like ChatGPT and Copilot. My general advice is to use your common sense: is gen AI doing most of the work for me? If so, then you are likely in the danger zone, at least as far as this course is concerned.

In our opinion, we're not doing _anything_ in CS 3113 that warrants the use of gen AI. However, in order to minimize ambiguity, please abide by the following rules and you will be fine:

1. **Do not enter the project prompts, class notes, or my code into ChatGPT**. While is isn't an act of plagiarism, it is unauthorized use of university (and our) materials.
2. **Do not prompt gen AI with project requirements and use the resulting code**. _This is plagiarism_, full stop. Plus, even if you paraphrase our instructions instead of using our prompts directly, raylib is still a really large library; chances are that the code that gen AI will give you will be so irreconcilable with our course's that catching plagiarism will be a non-issue. Please, don't even think about it.
3. **If you absolutely must use gen AI, do so smartly and let Prof. Romero Cruz know**. If you really want to use gen AI for something not directly related to or taught in the class (i.e. path-finding algorithms, etc.), do the following:
    1. Ask Prof. Romero Cruz.
    2. In your code, cite the use of whichever gen AI technology you used. For example:
    ```c++
    /*
     * This algorithm was generated using ChatGPT using the following prompt:
     *  ...
     *  ...
     **/
    ```

It's also worth remembering that a single 100-word ChatGPT-4 response uses the equivalent of [**a whole 500ml bottle of water**](https://www.techrepublic.com/article/generative-ai-data-center-water-use/), with [**ChatGPT-3 not doing much better**](https://arxiv.org/pdf/2304.03271). This may or may not sound like a lot, but with a little under a [**third of the world's population having no access to potable water**](https://www.unesco.org/reports/wwdr/en/2024/s), it might be worth reserving the use of generative AI for things that actually need it and not to generate code that will give you an F in this class.

<a id="11-2"></a>

#### Penalties

If we determine that you plagiarized in any way, with or without gen AI, the following penalties will be handed out, with no exceptions:

- The penalty for 1 instance of plagiarism is an 0 on that project and an academic dishonesty report to the CSE department.
- The penalty for a 2nd instance of plagiarism will result in an F for the course and 2nd report to the CSE department.