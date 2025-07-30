#pragma once

#include <ftxui/dom/canvas.hpp>

class BonsaiCanvas : public ftxui::Canvas {
public:
    using ftxui::Canvas::Canvas;

    void DrawAngledBlockEllipse(int x1, int y1, int r1, int r2,
                                double max_angle_deg);

    void DrawAngledBlockEllipse(int x1, int y1, int r1, int r2,
                                const Stylizer& s, double max_angle_deg);

    void DrawAngledPointEllipse(int x1, int y1, int r1, int r2,
                                double max_angle_deg);

    void DrawAngledPointEllipse(int x1, int y1, int r1, int r2,
                                const Stylizer& s, double max_angle_deg);
};