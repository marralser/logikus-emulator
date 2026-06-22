Logikus Linux Port
Origin

In the year 1990, Franz Dimbeck wrote a LOGIKUS simulator for the Amiga.
This project is a modern Linux/SDL2 reimplementation of this Logikus simulator, an educational logic-circuit simulator originally written for the Commodore Amiga.

The original Amiga version was developed in Modula-2 and distributed for the Amiga platform. The Linux version preserves the original concepts, user interface layout, and educational functionality while replacing the Amiga-specific operating system, graphics, and input code with modern cross-platform SDL2 code.

Credits

Original Amiga version:

Author: Franz Dimbeck
Original program: Logikus
Platform: Commodore Amiga
Language: Modula-2
Original source code and design: original author(s) of the Amiga release

Linux port:

SDL2/C++ reimplementation derived from the original Modula-2 source code
Intended for educational, historical, and preservation purposes
Preservation Note

This project exists because the original Amiga source code was available, making it possible to preserve and modernize the software for current operating systems while maintaining compatibility with the original program logic and user experience.

All rights to the original work remain with their respective copyright holders.

# Logikus Linux SDL2 Port

## Build

```bash
sudo apt install build-essential cmake pkg-config libsdl2-dev
mkdir src
# save or copy the file main.cpp in this folder!
mkdir build
cd build
cmake ..
make
./logikus
```

If CMake reports that `CMakeCache.txt` was created in another directory, remove the build directory and rerun CMake:

```bash
rm -rf build
mkdir build
cd build
cmake ..
make
./logikus
```

## Controls

- Left click a hole: start or finish a wire
- Right click while drawing: add a bend point
- Right click an existing connected hole: remove that wire
- Left click a switch: toggle it
- Hold the left mouse button on the T button: temporary contact; the button turns yellow while pressed
- Click a lamp label (`L0`...`L9`) to edit it. Type the new label, press `Enter` to keep it, `Esc` to cancel, `Backspace` to delete characters. Labels are saved and loaded with the circuit.
- `N`: new board
- `S`: save to `logikus.save` in the current working directory
- `L`: load from `logikus.save` in the current working directory
- `R`: toggle red live-wire display mode
- `Q` or `Esc`: quit

## Origin / Acknowledgment

This SDL2/C++ version is a modern Linux reimplementation of the original **Logikus** program for the Commodore Amiga. The original source was written in Modula-2 and used Amiga-specific graphics, mouse, windowing, and menu APIs. This port preserves the educational logic-simulator idea and board behavior while replacing the Amiga-specific user interface with SDL2.

This project is intended for educational, historical, and preservation use. Rights to the original Amiga work remain with their respective author(s) and copyright holder(s).

## Notes

This version uses a 640x330 logical canvas scaled 2x. The original Amiga board area is still 640x256, but there is now extra space below the board for command/status text so labels do not overlap the switches or board elements.


## Layout note

The Linux version uses a slightly taller board than the original Amiga screen so the title, editable lamp labels, connector points, switch labels, and command/status line do not overlap on modern displays.
