#include "CohenSutherland.h"

namespace {

const int INSIDE = 0;
const int LEFT = 1;
const int RIGHT = 2;
const int BOTTOM = 4;
const int TOP = 8;

int computeOutCode(Vec2 p, const ClipWindow& window) {
    int code = INSIDE;

    if (p.x < window.xmin) {
        code |= LEFT;
    } else if (p.x > window.xmax) {
        code |= RIGHT;
    }

    if (p.y < window.ymin) {
        code |= BOTTOM;
    } else if (p.y > window.ymax) {
        code |= TOP;
    }

    return code;
}

}  // namespace

ClipResult cohenSutherlandClip(Vec2 p0, Vec2 p1, const ClipWindow& window) {
    int outCode0 = computeOutCode(p0, window);
    int outCode1 = computeOutCode(p1, window);

    while (true) {
        if ((outCode0 | outCode1) == 0) {
            // Both endpoints inside the window: trivially accept.
            return ClipResult{true, p0, p1};
        }

        if ((outCode0 & outCode1) != 0) {
            // Both endpoints share an outside region: trivially reject.
            return ClipResult{false, p0, p1};
        }

        int outCodeOut = outCode0 != 0 ? outCode0 : outCode1;
        Vec2 intersection{};

        if (outCodeOut & TOP) {
            intersection.x = p0.x + (p1.x - p0.x) * (window.ymax - p0.y) / (p1.y - p0.y);
            intersection.y = window.ymax;
        } else if (outCodeOut & BOTTOM) {
            intersection.x = p0.x + (p1.x - p0.x) * (window.ymin - p0.y) / (p1.y - p0.y);
            intersection.y = window.ymin;
        } else if (outCodeOut & RIGHT) {
            intersection.y = p0.y + (p1.y - p0.y) * (window.xmax - p0.x) / (p1.x - p0.x);
            intersection.x = window.xmax;
        } else {
            intersection.y = p0.y + (p1.y - p0.y) * (window.xmin - p0.x) / (p1.x - p0.x);
            intersection.x = window.xmin;
        }

        if (outCodeOut == outCode0) {
            p0 = intersection;
            outCode0 = computeOutCode(p0, window);
        } else {
            p1 = intersection;
            outCode1 = computeOutCode(p1, window);
        }
    }
}
