#include "DDA.h"

#include <cmath>
#include <vector>

std::vector<Point> ddaLine(int x0, int y0, int x1, int y1) {
    std::vector<Point> points;

    const int dx = x1 - x0;
    const int dy = y1 - y0;
    const int steps = std::max(std::abs(dx), std::abs(dy));

    if (steps == 0) {
        points.push_back({x0, y0});
        return points;
    }

    const float xIncrement = static_cast<float>(dx) / static_cast<float>(steps);
    const float yIncrement = static_cast<float>(dy) / static_cast<float>(steps);

    float x = static_cast<float>(x0);
    float y = static_cast<float>(y0);

    points.reserve(static_cast<std::size_t>(steps) + 1);
    for (int i = 0; i <= steps; ++i) {
        points.push_back({static_cast<int>(std::lround(x)), static_cast<int>(std::lround(y))});
        x += xIncrement;
        y += yIncrement;
    }

    return points;
}
