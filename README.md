# Bonsai

Bonsai is a terminal user interface (TUI) remake of the GNOME Baobab disk usage analyzer.  
It provides a lightweight, fast, and visually structured way to explore disk usage on Linux systems directly in the terminal.

<p align="center">
  <img src="img/app.gif" alt="App preview" width="640">
</p>

## Features

- Navigable sidebar displaying folder sizes, sorted by a scanning algorithm
- Pie chart visualization similar to GNOME Baobab:
  - Multiple rings representing nested directories
  - Color-coded sections for files and folders
  - Labels for each item
- Built using **CMake** and **FTXUI** for a modern TUI experience
- Hashmap scan for efficient folder size lookups
- Dynamic pie chart updates when navigating between folders in the sidebar  
- Pie chart highlighting when hovering over folders or files 
- Asynchronous UI rendering
- User customization via configuration file
- Ability to remove files / folders

## Planned Features

- Ability to search for folders
- Translations?
- Windows port?

## Dependencies

Before running, make sure you have:

- A terminal that supports RGB color coding
- CMake
- A C compiler
- Xdd

## Usage

### Keyboard Controls

- **`q`** → Quit
- **Backspace / Escape** → Go back a folder
- **Up / Down arrows** → Navigate the menu
- **Enter** → Select a folder in the menu or a button in a modal
- **Escape** → Exit a modal

### Mouse Controls

- **Scroll** → Navigate up and down in the menu
- **Click** → Select a button in a modal

## Installation
### Arch Linux (AUR)
You can install Bonsai directly from the AUR:

```bash
# Using an AUR helper like yay
yay -S bonsai
```

### Other Linux distributions

Build manually from source:
```bash
git clone https://github.com/LittleBigOwI/bonsai.git

cd bonsai
mkdir build

cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build

./build/bonsai
```