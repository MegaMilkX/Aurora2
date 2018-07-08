R"(#version 450
    in vec3 Position;
    in vec2 UV;

    out vec2 UVFrag;

    uniform mat4 MatrixModel;
    uniform mat4 MatrixView;
    uniform mat4 MatrixProjection;

    uniform vec2 QuadSize;
    uniform vec2 ScreenSize;

    void main()
    {
        vec4 PositionScreen;
        vec4 PositionModel;
        
        PositionModel = vec4 ( Position , 1.0 ) ;
        UVFrag = vec2(UV.x, 1.0 - UV.y) ;
        PositionScreen = 
            MatrixProjection *
            MatrixModel *
            (PositionModel *
            vec4(QuadSize.x, QuadSize.y, 0.0, 1.0));
        gl_Position = PositionScreen ; 
    }
)"