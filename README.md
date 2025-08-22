# Bonsai

Bonsai is a terminal user interface (TUI) remake of the GNOME Baobab disk usage analyzer.  
It provides a lightweight, fast, and visually structured way to explore disk usage on Linux systems directly in the terminal.

<p align="center">
  <img src="img/app.gif" alt="App preview" width="600">
</p>

## Features

- Navigable sidebar displaying folder sizes, sorted by a multithreaded scanning algorithm
- Pie chart visualization similar to GNOME Baobab:
  - Multiple rings representing nested directories
  - Color-coded sections for files and folders
  - Percentage size usage labels for each item
- Built using **CMake** and **FTXUI** for a modern TUI experience
- Tree and hashmap-based data structure for efficient folder size lookups
- Dynamic pie chart updates when navigating between folders in the sidebar  
- Pie chart highlighting when hovering over folders or files 
- Asynchronous UI rendering before scan completion
- User customization via configuration file

## Planned Features

- Visually better async rendering when scanning disk
- Add ability to remove files / folders

## Installation

Currently, Bonsai must be built manually:

```bash
git clone https://github.com/LittleBigOwI/bonsai.git
cd bonsai
mkdir build && cd build
cmake .. && make
./bonsai
```