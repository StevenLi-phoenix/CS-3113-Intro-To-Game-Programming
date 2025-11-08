#version 330

in vec3 vertexPosition;
in vec2 vertexTexCoord;

out vec2 fragTexCoord;
out vec2 fragPosition;

uniform mat4 mvp;
uniform vec2 isoOrigin;
uniform float isoScale;
uniform int useIsometric;

vec2 toIsometric(vec2 pos)
{
    vec2 centered = (pos - isoOrigin) / max(isoScale, 0.0001);
    float isoX = centered.x - centered.y;
    float isoY = (centered.x + centered.y) * 0.5;
    return vec2(isoX, isoY) * isoScale + isoOrigin;
}

void main()
{
    vec3 position = vertexPosition;
    if (useIsometric == 1)
    {
        position.xy = toIsometric(position.xy);
    }

    fragTexCoord = vertexTexCoord;
    fragPosition = position.xy;
    gl_Position = mvp * vec4(position, 1.0);
}
