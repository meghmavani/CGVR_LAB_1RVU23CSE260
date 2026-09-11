#include "MidpointCircle.h"

std::vector<Point> midpointCircle(int cx, int cy, int radius) {
    std::vector<Point> points;

    int x = radius;
    int y = 0;
    int err = 1 - radius;

    auto plotOctants = [&](int x, int y) {
        points.push_back({cx + x, cy + y});
        points.push_back({cx + y, cy + x});
        points.push_back({cx - y, cy + x});
        points.push_back({cx - x, cy + y});
        points.push_back({cx - x, cy - y});
        points.push_back({cx - y, cy - x});
        points.push_back({cx + y, cy - x});
        points.push_back({cx + x, cy - y});
    };

    while (x >= y) {
        plotOctants(x, y);
        y++;
        if (err < 0) {
            err += 2 * y + 1;
        } else {
            x--;
            err += 2 * (y - x) + 1;
        }
    }

    return points;
}
