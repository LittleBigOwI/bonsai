# Bonsai

Bonsai is a terminal user interface (TUI) remake of the GNOME Baobab disk usage analyzer.  
It provides a lightweight, fast, and visually structured way to explore disk usage on Linux systems directly in the terminal.

## Features

- Navigable sidebar displaying folder sizes, sorted by a multithreaded scanning algorithm
- Pie chart visualization similar to GNOME Baobab:
  - Multiple rings representing nested directories
  - Color-coded sections for files and folders
  - Percentage size usage labels for each item
- Built using **CMake** and **FTXUI** for a modern TUI experience
- Tree and hashmap-based data structure for efficient folder size lookups
- Dynamic pie chart updates when navigating the sidebar  

## Planned Features

- Pie chart highlighting when hovering over folders or files  
- User customization via configuration file:
  - Colors
  - Icons
  - Sizes
  - More options to come  
- PKGBUILD for AUR installation  
- Asynchronous UI rendering before scan completion  

## Installation

Currently, Bonsai must be built manually:

```bash
git clone https://github.com/LittleBigOwI/bonsai.git
cd bonsai
mkdir build && cd build
cmake ..
make
./bonsai
```

## Image Previews

Coming soon.