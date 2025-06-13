# sphere renderer 🌐✨

## overview
a lightweight, interactive 3d sphere rendering application built with modern opengl and imgui. explore geometric visualization with real-time rendering and intuitive controls.

## features
- 🎨 dynamic sphere rendering
- 🖱️ interactive camera controls
- 🌈 color manipulation
- 🔬 wireframe and solid rendering modes
- 📊 performance metrics display

## prerequisites
- c++17 compatible compiler
- opengl 3.3+
- vcpkg package manager

## dependencies
- glfw
- glew
- imgui
- glm (opengl maths)

## build instructions

### windows (visual studio)
1. install vcpkg
```bash
git clone https://github.com/microsoft/vcpkg
.\vcpkg\bootstrap-vcpkg.bat
```

2. install dependencies
```bash
.\vcpkg\vcpkg install glfw3 glew imgui glm --triplet x64-windows
```

3. configure project
```bash
mkdir build && cd build
cmake -DCMAKE_TOOLCHAIN_FILE=path/to/vcpkg/scripts/buildsystems/vcpkg.cmake ..
```

4. build project
```bash
cmake --build . --config release
```

## controls
- 🖱️ left-click drag: rotate camera
- 🖱️ right-click drag: adjust color (in full color mode)
- 🖥️ ui buttons: switch rendering modes
- 🎚️ checkbox: toggle object visibility


## license
[specify your license here]

## contributions
contributions are welcome! please read the contribution guidelines before getting started.

## screenshot
![image](https://github.com/user-attachments/assets/0e51f94d-a1e2-4a9e-8151-2bf629f8cecf)


## troubleshooting
- ensure all dependencies are correctly installed
- verify opengl version compatibility
- check compiler requirements

## performance
- real time fps display
- lightweight rendering
- optimized sphere generation

## future roadmap
- [ ] advanced lighting models
- [ ] more complex mesh generation
- [ ] additional rendering modes 
