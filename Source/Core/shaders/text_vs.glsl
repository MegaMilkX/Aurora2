R"(#version 450
    in vec3 Position;
    in vec3 UVW;

    out vec3 UVFrag;

    uniform mat4 MatrixModel;
    uniform mat4 MatrixView;
    uniform mat4 MatrixProjection;

    uniform float GlyphPageCount;

    void main()
    {
        vec4 PositionScreen;
        vec4 PositionModel;
        
        PositionModel = vec4 ( Position , 1.0 ) ;

        float texelPageStep = 1.0 / (GlyphPageCount + 1.0);
        UVFrag = vec3(UVW.x, UVW.y, (UVW.z+0.5) / GlyphPageCount);

        PositionScreen = 
            MatrixProjection *
            MatrixModel *
            (PositionModel);
        gl_Position = PositionScreen ; 
    }
)"