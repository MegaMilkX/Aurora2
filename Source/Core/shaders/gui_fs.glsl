R"(#version 450
    in vec2 UVFrag;

    out vec4 fragOut;

    uniform sampler2D DiffuseTexture;
    uniform vec4 Color;

    void main()
    {
        vec4 Diffuse;
         
        Diffuse = texture ( DiffuseTexture , UVFrag ) * Color ; 
        fragOut = Diffuse ; 
    }
)"