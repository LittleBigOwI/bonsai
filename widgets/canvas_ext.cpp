#include "canvas_ext.hpp"

#include <cmath>

constexpr auto nostyle = [](ftxui::Pixel &) {};

void BonsaiCanvas::DrawAngledBlockCircle(int x, int y, int r, double angle)
{
    this->DrawAngledBlockEllipse(x, y, r, r, angle);
}

void BonsaiCanvas::DrawAngledBlockCircle(int x, int y, int r, double angle, const ftxui::Color &color)
{
    this->DrawAngledBlockEllipse(x, y, r, r, angle, color);
}

void BonsaiCanvas::DrawAngledBlockCircle(int x, int y, int r, double angle, const Stylizer &s)
{
    this->DrawAngledBlockEllipse(x, y, r, r, angle, s);
}

void BonsaiCanvas::DrawAngledPointCircle(int x, int y, int r, double angle)
{
    this->DrawAngledPointEllipse(x, y, r, r, angle);
}

void BonsaiCanvas::DrawAngledPointCircle(int x, int y, int r, double angle, const ftxui::Color &color)
{
    this->DrawAngledPointEllipse(x, y, r, r, angle, color);
}

void BonsaiCanvas::DrawAngledPointCircle(int x, int y, int r, double angle, const Stylizer &s)
{
    this->DrawAngledPointEllipse(x, y, r, r, angle, s);
}

void BonsaiCanvas::DrawAngledBlockEllipse(int x, int y, int r1, int r2, double angle)
{
    this->DrawAngledBlockEllipse(x, y, r1, r2, angle, nostyle);
}

void BonsaiCanvas::DrawAngledBlockEllipse(int x, int y, int r1, int r2, double angle, const ftxui::Color &color)
{
    this->DrawAngledBlockEllipse(x, y, r1, r2, angle, [color](ftxui::Pixel &p)
                                 { p.foreground_color = color; });
}

void BonsaiCanvas::DrawAngledBlockEllipse(int x1, int y1, int r1, int r2, double angle, const Stylizer &s)
{
    y1 /= 2;
    r2 /= 2;
    int x = -r1;
    int y = 0;
    int e2 = r2;
    int dx = (1 + 2 * x) * e2 * e2;
    int dy = x * x;
    int err = dx + dy;

    double max_angle_rad = angle * M_PI / 180.0;

    auto draw_if_within_angle = [&](int dx, int dy)
    {
        double angle = atan2(dy * r1, dx * r2);
        if (angle < 0)
            angle += 2 * M_PI;
        if (angle <= max_angle_rad)
            this->DrawBlock(x1 + dx, 2 * (y1 + dy), true, s);
    };

    do
    {
        draw_if_within_angle(-x, y);
        draw_if_within_angle(x, y);
        draw_if_within_angle(x, -y);
        draw_if_within_angle(-x, -y);

        e2 = 2 * err;
        if (e2 >= dx)
        {
            x++;
            err += dx += 2 * r2 * r2;
        }
        if (e2 <= dy)
        {
            y++;
            err += dy += 2 * r1 * r1;
        }
    } while (x <= 0);

    while (y++ < r2)
    {
        draw_if_within_angle(0, y);
        draw_if_within_angle(0, -y);
    }
}

void BonsaiCanvas::DrawAngledPointEllipse(int x, int y, int r1, int r2, double angle)
{
    this->DrawAngledPointEllipse(x, y, r1, r2, angle, nostyle);
}

void BonsaiCanvas::DrawAngledPointEllipse(int x, int y, int r1, int r2, double angle, const ftxui::Color &color)
{
    this->DrawAngledPointEllipse(x, y, r1, r2, angle, [color](ftxui::Pixel &p)
                                 { p.foreground_color = color; });
}

void BonsaiCanvas::DrawAngledPointEllipse(int x1, int y1, int r1, int r2, double angle, const Stylizer &s)
{
    y1 /= 2;
    r2 /= 2;
    int x = -r1;
    int y = 0;
    int e2 = r2;
    int dx = (1 + 2 * x) * e2 * e2;
    int dy = x * x;
    int err = dx + dy;

    double max_angle_rad = angle * M_PI / 180.0;

    auto draw_if_within_angle = [&](int dx, int dy)
    {
        double angle = atan2(dy * r1, dx * r2);
        if (angle < 0)
            angle += 2 * M_PI;
        if (angle <= max_angle_rad)
            this->DrawPoint(x1 + dx, 2 * (y1 + dy), true, s);
    };

    do
    {
        draw_if_within_angle(-x, y);
        draw_if_within_angle(x, y);
        draw_if_within_angle(x, -y);
        draw_if_within_angle(-x, -y);

        e2 = 2 * err;
        if (e2 >= dx)
        {
            x++;
            err += dx += 2 * r2 * r2;
        }
        if (e2 <= dy)
        {
            y++;
            err += dy += 2 * r1 * r1;
        }
    } while (x <= 0);

    while (y++ < r2)
    {
        draw_if_within_angle(0, y);
        draw_if_within_angle(0, -y);
    }
}