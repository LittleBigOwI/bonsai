#include "canvas_ext.hpp"

#include <cmath>

constexpr auto nostyle = [](ftxui::Pixel&) {};

void BonsaiCanvas::DrawAngledBlockEllipse(int x1, int y1, int r1, int r2,
                                          double max_angle_deg) {
    this->DrawAngledBlockEllipse(x1, y1, r1, r2, nostyle, max_angle_deg);
}

void BonsaiCanvas::DrawAngledBlockEllipse(int x1, int y1, int r1, int r2,
                                          const Stylizer& s, double max_angle_deg) {
    y1 /= 2;
    r2 /= 2;
    int x = -r1;
    int y = 0;
    int e2 = r2;
    int dx = (1 + 2 * x) * e2 * e2;
    int dy = x * x;
    int err = dx + dy;

    double max_angle_rad = max_angle_deg * M_PI / 180.0;

    auto draw_if_within_angle = [&](int dx, int dy) {
        double angle = atan2(dy * r1, dx * r2);
        if (angle < 0)
        angle += 2 * M_PI;
        if (angle <= max_angle_rad)
        this->DrawBlock(x1 + dx, 2 * (y1 + dy), true, s);
    };

    do {
        draw_if_within_angle(-x, y);
        draw_if_within_angle(x, y);
        draw_if_within_angle(x, -y);
        draw_if_within_angle(-x, -y);

        e2 = 2 * err;
        if (e2 >= dx) {
        x++;
        err += dx += 2 * r2 * r2;
        }
        if (e2 <= dy) {
        y++;
        err += dy += 2 * r1 * r1;
        }
    } while (x <= 0);

    while (y++ < r2) {
        draw_if_within_angle(0, y);
        draw_if_within_angle(0, -y);
    }
}

void BonsaiCanvas::DrawAngledPointEllipse(int x1, int y1, int r1, int r2,
                                          double max_angle_deg) {
    this->DrawAngledPointEllipse(x1, y1, r1, r2, nostyle, max_angle_deg);
}

void BonsaiCanvas::DrawAngledPointEllipse(int x1, int y1, int r1, int r2,
                                          const Stylizer& s, double max_angle_deg) {
    y1 /= 2;
    r2 /= 2;
    int x = -r1;
    int y = 0;
    int e2 = r2;
    int dx = (1 + 2 * x) * e2 * e2;
    int dy = x * x;
    int err = dx + dy;

    double max_angle_rad = max_angle_deg * M_PI / 180.0;

    auto draw_if_within_angle = [&](int dx, int dy) {
        double angle = atan2(dy * r1, dx * r2);
        if (angle < 0)
        angle += 2 * M_PI;
        if (angle <= max_angle_rad)
        this->DrawPoint(x1 + dx, 2 * (y1 + dy), true, s);
    };

    do {
        draw_if_within_angle(-x, y);
        draw_if_within_angle(x, y);
        draw_if_within_angle(x, -y);
        draw_if_within_angle(-x, -y);

        e2 = 2 * err;
        if (e2 >= dx) {
        x++;
        err += dx += 2 * r2 * r2;
        }
        if (e2 <= dy) {
        y++;
        err += dy += 2 * r1 * r1;
        }
    } while (x <= 0);

    while (y++ < r2) {
        draw_if_within_angle(0, y);
        draw_if_within_angle(0, -y);
    }
}