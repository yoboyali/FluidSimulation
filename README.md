# FluidSimulation
A real-time SPH (Smoothed Particle Hydrodynamics) fluid simulation.

https://github.com/user-attachments/assets/98400d3c-eac2-4696-a3c8-96f76ebdbd3d

## How it works
The program revolves around the SPH interpolation formula
```math
A(r) = ∑ (mⱼ/ρⱼ) * Aⱼ * W(r - rⱼ, h)
       j
```

Where:
- **A(r)** = Value of property A at position r
- **∑** = Sum over all neighboring particles j
- **mⱼ** = Mass of particle j
- **ρⱼ** = Density of particle j
- **Aⱼ** = Property A of particle j (velocity, pressure, etc.)
- **W(r - rⱼ, h)** = Smoothing kernel
- **h** = Smoothing radius 

This is how we calculate every property **(A)** needed for the fluid.

Solving for **Density** gives us:
```math
ρ(rᵢ) = ∑ mⱼ * W(rᵢ - rⱼ, h)
        j
```

Solving for **Pressure Force** gives us:
```math
Fᵢ = -∑ mⱼ * (Pᵢ + Pⱼ)/(2ρⱼ) * ∇W(rᵢ - rⱼ, h)
      j
```

## Smoothing Kernels
The effect of particle B on particle A depends on how far particle B is from particle A — the closer the particle, the greater the effect.

We define a smoothing radius where any particle outside said radius has no effect on the particle. To achieve this effect, we use smoothing formulas **W(r - rⱼ, h)** that vary based on the distance between the two particles.

<img src="https://github.com/yoboyali/FluidSimulation/blob/main/ReadMe%20Assets/SmoothingKernels.png"/>

⬛️ Density &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; 🟥 Near Density &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; 🟩 Pressure &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; 🟪 Near Pressure

## Running the Program

### Prerequisites
To run the program you need the following libraries:
- **GLFW** - Window and input handling
- **GLAD** - OpenGL loader
- **GLM** - Mathematics library
- **ImGui** - GUI interface

And a valid C/C++ compiler supporting C++17 or later.

### Compilation

#### Windows (MinGW)
From inside the root folder run:
```bash
mkdir build
cd build
cmake -G "MinGW Makefiles" ..
cmake --build .
FluidSimulation.exe
```

#### Linux
```bash
mkdir build
cd build
cmake ..
cmake --build .
./FluidSimulation
```

### Controls
- **W/A/S/D** - Move camera
- **Space/Shift** - Move up/down
- **Mouse** - Look around
- **CTRL** - Toggle mouse capture
- **ESC** - Close simulation

## Credits
The following resources were used:
- https://unusualinsights.github.io/fluid_tutorial/
- https://matthias-research.github.io/pages/publications/sca03.pdf
- https://www.youtube.com/watch?v=rSKMYc1CQHE
- https://www.youtube.com/watch?v=iKAVRgIrUOU
- https://www.youtube.com/watch?v=D2M8jTtKi44
