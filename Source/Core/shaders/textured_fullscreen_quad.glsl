R"(#version 450

in vec3 Vertex;
in vec2 UV;
out vec2 UVFrag;
void main()
{
    UVFrag = UV;
    gl_Position = vec4(Vertex, 1.0);
}

)"