R"(#version 450
    in vec3 Position;
    in vec2 UV;
    in vec3 Normal;

    out vec2 UVFrag;
    out vec3 NormalModel;
    out vec3 FragPosWorld;

    uniform mat4 MatrixModel;
    uniform mat4 MatrixView;
    uniform mat4 MatrixProjection;

    void main()
    {
        vec4 PositionScreen;
        vec4 PositionModel;
        
        PositionModel = vec4 ( Position , 1.0 ) ;
        FragPosWorld = vec3 ( MatrixModel * PositionModel ) ; 
        NormalModel = normalize ( ( MatrixModel * vec4 ( Normal , 0.0 ) ) . xyz ) ; 
        UVFrag = UV ;  
        PositionScreen = MatrixProjection * MatrixView * MatrixModel * PositionModel ; 
        gl_Position = PositionScreen ; 
    }
)"