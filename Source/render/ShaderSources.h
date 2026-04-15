#pragma once
#include "CommonIncludes.h"
namespace starflux::render::shaders
{
static constexpr const char* vertex = R"(
attribute vec2 position;
uniform float pointSize;
void main() {
    gl_Position = vec4(position, 0.0, 1.0);
    gl_PointSize = pointSize;
}
)";

static constexpr const char* fragment = R"(
uniform vec4 colour;
void main() {
    vec2 c = gl_PointCoord - vec2(0.5);
    float a = smoothstep(0.25, 0.0, dot(c,c));
    gl_FragColor = vec4(colour.rgb, colour.a * a);
}
)";
}
