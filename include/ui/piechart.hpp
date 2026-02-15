/* PIE LOGIC
New pie structure: no class inheriting, just using base ftxui components (more maintainable)
This version of the file is a standalone version (no menu, but full contents)
Note that directory sizes aren't computed (scanner's job)

- Retrieve current directory total size from Scanner.
- Retrieve all scanned entries up to a given max depth.
- For each entry:
    - Get its size.
    - Compute its percentage relative to the current root directory.
    - Convert that percentage into an angular sweep.
- Each depth level corresponds to one ring on the canvas:
    - Depth 0 -> inner ring.
    - Higher depths -> outer rings.
- Entries are sorted by:
    - Depth (ascending).
    - Type (directories first).
    - Size (descending).
- For each computed angle:
    - Accumulate offset angle.
    - Create a slice entry with inner/outer radius.
    - Push slice to render list.
- Inner ring defines base color.
- Outer rings will later dim or derive color from parent.
- Rendering draws ring sectors using computed angle and offset.
*/

# pragma once

#include "../../include/core/app_data.hpp"
#include "../../include/core/scanner.hpp"

#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/component/component.hpp>

using namespace ftxui;

class BonsaiPie {
public:
    static Component pie(std::shared_ptr<AppData::BonsaiData> data);
    static void worker(ScreenInteractive* screen, std::shared_ptr<AppData::BonsaiData> data, Scanner* scanner, const fs::path& default_path);

private:
    struct EntryInfo {
        std::filesystem::path path;
        uint64_t size;
        
        bool is_dir;
        int depth;
    };

    static void collectEntries(const fs::path& dir, std::vector<EntryInfo>& entries, int current_depth, int max_depth, Scanner* scanner);

    /* Draws a ring sector of an ellipse on the Canvas.
    - Inputs:
        - (x1, y1) -> center of the ellipse.
        - r1, r2    -> radii along x and y axes.
        - r3        -> inner radius to create a hollow ring.
        - starting_angle -> where the sector starts (degrees).
        - angle          -> sweep of the sector (degrees).
        - s -> stylizer (color/attributes).
    - Computes angular and radial bounds for each pixel.
    - Checks if pixel is:
        - Within angular sweep.
        - Outside the inner radius (to create ring effect).
    - Iterates over ellipse pixels using an ellipse midpoint-like algorithm.
    - Draws pixels that pass checks with c.DrawBlock.
    - Result: one ring-shaped slice corresponding to a portion of a pie chart.

    - The fact that this draws and Ellipse isn't rlly useful as we always call this with r1 = r2.
      Some terminals have text interline that causes a deformation on ftxui canvas. Maybe use r1 and r2 to account for that deformation in the future?
    */
    static void drawAngledBlockEllipseRingOffset(Canvas& c, int x1, int y1, int r1, int r2, int r3, double starting_angle, double angle, const Canvas::Stylizer& s);
};