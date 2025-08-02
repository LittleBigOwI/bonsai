#pragma once

#include <ftxui/dom/canvas.hpp>

class BonsaiCanvas : public ftxui::Canvas {
public:
    explicit BonsaiCanvas(ftxui::Canvas& canvas) : canvas_(canvas) {}

    void DrawAngledBlockCircle(int x, int y, int r, double angle);
    void DrawAngledBlockCircle(int x, int y, int r, double angle, const ftxui::Color& color);
    void DrawAngledBlockCircle(int x, int y, int r, double angle, const Stylizer& s);

    void DrawAngledPointCircle(int x, int y, int r, double angle);
    void DrawAngledPointCircle(int x, int y, int r, double angle, const ftxui::Color& color);
    void DrawAngledPointCircle(int x, int y, int r, double angle, const Stylizer& s);

    void DrawAngledBlockCircleFilled(int x, int y, int r, double angle);
    void DrawAngledBlockCircleFilled(int x, int y, int r, double angle, const ftxui::Color& color);
    void DrawAngledBlockCircleFilled(int x, int y, int r, double angle, const Stylizer& s);

    void DrawAngledPointCircleFilled(int x, int y, int r, double angle);
    void DrawAngledPointCircleFilled(int x, int y, int r, double angle, const ftxui::Color& color);
    void DrawAngledPointCircleFilled(int x, int y, int r, double angle, const Stylizer& s);

    void DrawAngledPointCircleRing(int x, int y, int r1, int r2, double angle);
    void DrawAngledPointCircleRing(int x, int y, int r1, int r2, double angle, const ftxui::Color& color);
    void DrawAngledPointCircleRing(int x, int y, int r1, int r2, double angle, const Stylizer& s);

    void DrawAngledBlockCircleRing(int x, int y, int r1, int r2, double angle);
    void DrawAngledBlockCircleRing(int x, int y, int r1, int r2, double angle, const ftxui::Color& color);
    void DrawAngledBlockCircleRing(int x, int y, int r1, int r2, double angle, const Stylizer& s);

    void DrawAngledBlockCircleRingOffset(int x, int y, int r1, int r2, double starting_angle, double angle);
    void DrawAngledBlockCircleRingOffset(int x, int y, int r1, int r2, double starting_angle, double angle, const ftxui::Color& color);
    void DrawAngledBlockCircleRingOffset(int x, int y, int r1, int r2, double starting_angle, double angle, const Stylizer &s);

    void DrawAngledPointCircleRingOffset(int x, int y, int r1, int r2, double starting_angle, double angle);
    void DrawAngledPointCircleRingOffset(int x, int y, int r1, int r2, double starting_angle, double angle, const ftxui::Color& color);
    void DrawAngledPointCircleRingOffset(int x, int y, int r1, int r2, double starting_angle, double angle, const Stylizer &s);

    void DrawAngledBlockEllipse(int x, int y, int r1, int r2, double angle);
    void DrawAngledBlockEllipse(int x, int y, int r1, int r2, double angle, const ftxui::Color& color);
    void DrawAngledBlockEllipse(int x, int y, int r1, int r2, double angle, const Stylizer& s);

    void DrawAngledPointEllipse(int x, int y, int r1, int r2, double angle);
    void DrawAngledPointEllipse(int x, int y, int r1, int r2, double angle, const ftxui::Color& color);
    void DrawAngledPointEllipse(int x, int y, int r1, int r2, double angle, const Stylizer& s);

    void DrawAngledBlockEllipseFilled(int x, int y, int r1, int r2, double angle);
    void DrawAngledBlockEllipseFilled(int x, int y, int r1, int r2, double angle, const ftxui::Color& color);
    void DrawAngledBlockEllipseFilled(int x, int y, int r1, int r2, double angle, const Stylizer& s);

    void DrawAngledPointEllipseFilled(int x, int y, int r1, int r2, double angle);
    void DrawAngledPointEllipseFilled(int x, int y, int r1, int r2, double angle, const ftxui::Color& color);
    void DrawAngledPointEllipseFilled(int x, int y, int r1, int r2, double angle, const Stylizer& s);

    void DrawAngledBlockEllipseRing(int x, int y, int r1, int r2, int r3, double angle);
    void DrawAngledBlockEllipseRing(int x, int y, int r1, int r2, int r3, double angle, const ftxui::Color& color);
    void DrawAngledBlockEllipseRing(int x, int y, int r1, int r2, int r3, double angle, const Stylizer& s);

    void DrawAngledPointEllipseRing(int x, int y, int r1, int r2, int r3, double angle);
    void DrawAngledPointEllipseRing(int x, int y, int r1, int r2, int r3, double angle, const ftxui::Color& color);
    void DrawAngledPointEllipseRing(int x, int y, int r1, int r2, int r3, double angle, const Stylizer& s);

    void DrawAngledBlockEllipseRingOffset(int x, int y, int r1, int r2, int r3, double starting_angle, double angle);
    void DrawAngledBlockEllipseRingOffset(int x, int y, int r1, int r2, int r3, double starting_angle, double angle, const ftxui::Color& color);
    void DrawAngledBlockEllipseRingOffset(int x, int y, int r1, int r2, int r3, double starting_angle, double angle, const Stylizer &s);

    void DrawAngledPointEllipseRingOffset(int x, int y, int r1, int r2, int r3, double starting_angle, double angle);
    void DrawAngledPointEllipseRingOffset(int x, int y, int r1, int r2, int r3, double starting_angle, double angle, const ftxui::Color& color);
    void DrawAngledPointEllipseRingOffset(int x, int y, int r1, int r2, int r3, double starting_angle, double angle, const Stylizer &s);
    
private:
    ftxui::Canvas& canvas_;
};