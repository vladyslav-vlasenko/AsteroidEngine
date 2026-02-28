# Asteroid Engine

High-performance simulation of celestial bodies' movement written on OpenGL 4.6 Core applying modern graphic techniques

****

## Resources used

* GLAD:

*Used to retrieve OpenGL function pointers from the GPU driver.*

* GLFW:

*Provides a cross-platform API for window creation, contexts, and input handling.*

* GLM:

*Provides convenient work with different mathematical objects like vectors and matrices.*

* STB_IMAGE:

*Used to load images.*

****

## Description

**Asteroid Project** - it's real-time simulation of N-body system, where objects interaction with the central object of system obeys Gravitational law:

$$
\mathbf{F}_i = -\frac{GMm_i}{r^3_i}\mathbf{r}_i
$$

where $m_i$ - appears to be mass of $i^{th}$ body, $M$ - mass of central body, $\mathbf{r_i}$ - radius-vector from the central body to  $i^{th}$ body.

Taking into account that objects don't interact with each other we can easily move to acceleration of $i^{th}$ body:

$$
\mathbf{a}_i = -\frac{k}{r^3_i} \mathbf{r}_i
$$

where $k$ - denotes proportionality coefficient ($k=GM$).

#### Processing trajectories

Here we use **Semi-implicit Euler method** to compute trajectory points, because it provides a good precision and minimizes mechanical energy deviations of system. We choose comparibly small time interval $\delta t$. According to this method:

$$
\begin{aligned}
\mathbf{v_i}(t+\delta t) &= \mathbf{v_i}(t)+\mathbf{a_i}(t)\delta t \\
\mathbf{r}_i(t+\delta t) &= \mathbf{r}_i(t) + \mathbf{v_i}(t)\delta t +\frac{\mathbf{a_i}(t)\delta t^2}{2}
\end{aligned}
$$

As there is a lot of points to compute we offload this process to a separate thread to improve performance. This is realized using

```
std::future<>
std::async()
std::thread()
```

Then we filter points, that stand from each other on some constant distance and write them to separate array.



#### Rendering Techniques

To ensure high performance and handle massive datasets, the engine utilizes several modern OpenGL features:



* **Instanced Rendering**: Used for rendering multiple celestial bodies with a single draw call, significantly reducing CPU-to-GPU overhead.



* **Bindless Textures (_GL_ARB_bindless_texture_)**: Allows the shaders to access textures directly via handles, eliminating the need for constant rebinding.



* **Shader Storage Buffer Objects (_GL_ARB_shader_storage_buffer_object_)**: Enables passing large, dynamic datasets (like transformation matrices) to shaders with high efficiency.



* **Legacy Pipeline (VAO/VBO/EBO)**: Utilized specifically for the UI and fixed interface elements where the vertex data volume is relatively small.

#### Interactive Interface

To provide an intuitive experience and robust object management, the engine features:



* **Object Picking System**: Uses ID-masking technique to accurately detect which celestial body or UI element is under the cursor.



* **Stencil Buffer Outlining**: When hovering over buttons or selecting objects, a real-time outline is generated using the Stencil Buffer and a custom post-processing shader.



* **Interactive Trajectories**: Selecting an object (via mouse click) triggers the visualization of its calculated path, allowing for real-time orbit analysis.



* **ButtonBlock System**: A custom UI component (menu) that manages groups of interactive buttons with automated layout and hover-state handling.

****

## Introduction to Asteroid Engine

<table border="0">
  <tr>
    <td width="250" valign="top">
      <img src="AsteroidEngine.gif" width="250">
    </td>
    <td valign="top">
      <strong>Asteroid Engine in Motion</strong><br>
      This demonstration showcases the seamless integration of high-performance physics and modern rendering. 
      By utilizing the <em>Semi-implicit Euler method</em>, we achieve stable orbital mechanics that preserve 
      the system's energy over time. The visual feedback, including the <strong>Stencil Buffer</strong> 
      outlining and real-time trajectory plotting, provides an interactive way to explore N-body 
      dynamics directly within the OpenGL-powered viewport.
    </td>
  </tr>
</table>
<br clear="all">

****
## Building Project on Your Device
To install required files and build this project all you need to do is just enter these in **bash** or **cmd** depending on your operation system:
* clone repository  
```bash
git clone https://github.com/vladyslav-vlasenko/AsteroidEngine
```
* entering directory
```bash
cd AsteroidEngine
```
* building solution files for your architecture and then compiling and linking them
```bash
cmake -S . -B build 
cmake --build build
```
As for now project is realized only in Debug version, so to run we will need:
```
bash
```
```bash
cd build/Debug
./Asteroid
``` 
or for Windows:
```
cmd
```
```bash
cd build\Debug
Asteroid.exe
```
And now you should see window with program interface.
## Plans for future
As for now this project is very far from ideal result. What is intended to realize in near future:
* **Building Trajectories**: As for now trajectory is processed only for closed move. But it will also be realized for open trajectories.
* **Slider Usage**: It is planned to apply slider to change simulation pace or to handle bodies speed while launch.
* **Interface Scale**: It is planned to extend interface set to typing line, menus, etc.
## References
Also I manage YouTube and Instagram channels named **_GCode_** and there I post videos about my projects. 
* **YouTube** channel: https://youtube.com/@gcoding?si=zIiAphIP12LJ_GQc
* **Instagram** channel: https://www.instagram.com/glcl.coding/
