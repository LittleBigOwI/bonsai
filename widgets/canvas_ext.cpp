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

void BonsaiCanvas::DrawAngledBlockEllipseFilled(int x1, int y1, int r1, int r2, double angle)
{
    this->DrawAngledBlockEllipseFilled(x1, y1, r1, r2, angle, nostyle);
}

void BonsaiCanvas::DrawAngledBlockEllipseFilled(int x1, int y1, int r1, int r2, double angle, const ftxui::Color &color)
{
    this->DrawAngledBlockEllipseFilled(x1, y1, r1, r2, angle, [color](ftxui::Pixel &p)
                                       { p.foreground_color = color; });
}

void BonsaiCanvas::DrawAngledBlockEllipseFilled(int x1, int y1, int r1, int r2, double angle, const Stylizer &s)
{
    int x = -r1;
    int y = 0;
    int e2 = r2;
    int dx = (1 + 2 * x) * e2 * e2;
    int dy = x * x;
    int err = dx + dy;

    double max_angle_rad = angle * M_PI / 180.0;

    auto within_angle = [&](int dx, int dy)
    {
        double angle = atan2(dy * r1, dx * r2);
        if (angle < 0)
            angle += 2 * M_PI;
        return angle <= max_angle_rad;
    };

    do
    {
        for (int xx = x1 + x; xx <= x1 - x; ++xx)
        {
            int dx = xx - x1;
            if (within_angle(dx, y))
                this->DrawBlock(xx, y1 + y, true, s);
            if (within_angle(dx, -y))
                this->DrawBlock(xx, y1 - y, true, s);
        }

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
        int dy_pos = y;
        int dy_neg = -y;
        if (within_angle(0, dy_pos))
            this->DrawBlock(x1, y1 + dy_pos, true, s);
        if (within_angle(0, dy_neg))
            this->DrawBlock(x1, y1 + dy_neg, true, s);
    }
}

void BonsaiCanvas::DrawAngledPointEllipseFilled(int x, int y, int r1, int r2, double angle)
{
    this->DrawAngledPointEllipseFilled(x, y, r1, r2, angle, nostyle);
}

void BonsaiCanvas::DrawAngledPointEllipseFilled(int x, int y, int r1, int r2, double angle, const ftxui::Color &color)
{
    this->DrawAngledPointEllipseFilled(x, y, r1, r2, angle, [color](ftxui::Pixel &p)
                                       { p.foreground_color = color; });
}

void BonsaiCanvas::DrawAngledPointEllipseFilled(int x1, int y1, int r1, int r2, double angle, const Stylizer &s)
{

    int x = -r1;
    int y = 0;
    int e2 = r2;
    int dx = (1 + 2 * x) * e2 * e2;
    int dy = x * x;
    int err = dx + dy;

    double max_angle_rad = angle * M_PI / 180.0;

    auto within_angle = [&](int dx, int dy)
    {
        double angle = atan2(dy * r1, dx * r2);
        if (angle < 0)
            angle += 2 * M_PI;
        return angle <= max_angle_rad;
    };

    do
    {
        for (int xx = x1 + x; xx <= x1 - x; ++xx)
        {
            int dx = xx - x1;
            if (within_angle(dx, y))
                DrawPoint(xx, y1 + y, true, s);
            if (within_angle(dx, -y))
                DrawPoint(xx, y1 - y, true, s);
        }

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
        int dy_pos = y;
        int dy_neg = -y;
        if (within_angle(0, dy_pos))
            DrawPoint(x1, y1 + dy_pos, true, s);
        if (within_angle(0, dy_neg))
            DrawPoint(x1, y1 + dy_neg, true, s);
    }
}

void BonsaiCanvas::DrawAngledPointEllipseRing(int x, int y, int r1, int r2, int r3, double angle)
{
    this->DrawAngledPointEllipseRing(x, y, r1, r2, r3, angle, nostyle);
}

void BonsaiCanvas::DrawAngledPointEllipseRing(int x, int y, int r1, int r2, int r3, double angle, const ftxui::Color &color)
{
    this->DrawAngledPointEllipseRing(x, y, r1, r2, r3, angle, [color](ftxui::Pixel &p)
                                     { p.foreground_color = color; });
}

void BonsaiCanvas::DrawAngledPointEllipseRing(int x1, int y1, int r1, int r2, int r3, double angle, const Stylizer &s)
{
    int x = -r1;
    int y = 0;
    int e2 = r2;
    int dx = (1 + 2 * x) * e2 * e2;
    int dy = x * x;
    int err = dx + dy;

    double max_angle_rad = angle * M_PI / 180.0;
    int r3_squared = r3 * r3;

    auto within_angular_ring = [&](int dx, int dy)
    {
        double angle = atan2(dy * r1, dx * r2);
        if (angle < 0)
            angle += 2 * M_PI;
        if (angle > max_angle_rad)
            return false;

        double norm_x = static_cast<double>(dx) / r1;
        double norm_y = static_cast<double>(dy) / r2;
        double distance_squared = norm_x * norm_x + norm_y * norm_y;

        double inner_radius_ratio = static_cast<double>(r3) / std::min(r1, r2);
        return distance_squared >= inner_radius_ratio * inner_radius_ratio;
    };

    do
    {
        for (int xx = x1 + x; xx <= x1 - x; ++xx)
        {
            int dx = xx - x1;
            if (within_angular_ring(dx, y))
                DrawPoint(xx, y1 + y, true, s);
            if (within_angular_ring(dx, -y))
                DrawPoint(xx, y1 - y, true, s);
        }

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
        if (within_angular_ring(0, y))
            DrawPoint(x1, y1 + y, true, s);
        if (within_angular_ring(0, -y))
            DrawPoint(x1, y1 - y, true, s);
    }
}
