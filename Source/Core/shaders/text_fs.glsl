R"(#version 450
    in vec3 UVFrag;

    out vec4 fragOut;

    uniform sampler2D DiffuseTexture;
    uniform sampler3D GlyphAtlas;

    void main()
    {
        vec4 Diffuse;
        
        Diffuse = texture ( GlyphAtlas , UVFrag ) ; 
        fragOut = Diffuse.xxxx ; 
    }
)"