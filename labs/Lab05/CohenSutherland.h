#pragma once

struct Vec2 {
    float x, y;
};

struct ClipWindow {
    float xmin, ymin, xmax, ymax;
};

struct ClipResult {
    bool visible;
    Vec2 p0, p1;
};

ClipResult cohenSutherlandClip(Vec2 p0, Vec2 p1, const ClipWindow& window);
