R"(

#vertex PositionScreen
    in vec4 PositionModel;
    uniform mat4 MatrixModel;
    uniform mat4 MatrixView;
    uniform mat4 MatrixProjection;
    out vec4 PositionScreen = 
        MatrixProjection *
        MatrixView *
        MatrixModel *
        PositionModel;
    
#vertex VertexBoneWeight4
    in vec4 BoneWeight4;
    out vec4 VertexBoneWeight4 = BoneWeight4;
#vertex VertexBoneWeight4
    in vec4 BoneIndex4;
    out vec4 VertexBoneIndex4 = BoneIndex4;

#vertex FragPosWorld
    in vec3 PositionModel;
    uniform mat4 MatrixModel;
    out vec3 FragPosWorld;
    FragPosWorld = vec3(MatrixModel * PositionModel);

#vertex UVFrag
    in vec2 UV;
    out vec2 UVFrag = UV;
    
#fragment Diffuse
    in vec2 UVFrag;
    uniform sampler2D DiffuseTexture;
    out vec4 Diffuse = texture(DiffuseTexture, UVFrag);
    
#fragment Ambient
    uniform vec3 AmbientColor;
    out vec4 Ambient = vec4(AmbientColor, 1.0);
#fragment LightDirection
    uniform vec3 LightOmniPos[LIGHT_OMNI_COUNT];
    in vec3 FragPosWorld;
    out vec3 LightDirection[LIGHT_OMNI_COUNT];
    for(int i = 0; i < LIGHT_OMNI_COUNT; i++)
    {
        LightDirection[i] = normalize(LightOmniPos[i] - FragPosWorld);
    }
#fragment LightOmniLambert
    in vec3 NormalModel;
    uniform vec3 LightOmniRGB[LIGHT_OMNI_COUNT];
    uniform vec3 LightOmniPos[LIGHT_OMNI_COUNT];
    in vec3 FragPosWorld;
    in vec3 LightDirection[LIGHT_OMNI_COUNT];
    out vec4 LightOmniLambert = vec4(0.0, 0.0, 0.0, 1.0);
    
    for(int i = 0; i < LIGHT_OMNI_COUNT; i++)
    {
        float diff = max(dot(NormalModel, LightDirection[i]), 0.0);
        float dist = distance(LightOmniPos[i], FragPosWorld);
        LightOmniLambert += 
            vec4(
                LightOmniRGB[i] * 
                diff *
                (1.0 / (1.0 + 0.5 * dist + 3.0 * dist * dist)),
                1.0
            );
    }
#fragment LightDirectLambert
    in vec3 NormalModel;
    uniform vec3 LightDirectRGB[LIGHT_DIRECT_COUNT];
    uniform vec3 LightDirect[LIGHT_DIRECT_COUNT];
    out vec4 LightDirectLambert = vec4(0.0, 0.0, 0.0, 1.0);
    
    for(int i = 0; i < LIGHT_DIRECT_COUNT; ++i)
    {
        float diff = max(dot(NormalModel, -LightDirect[i]), 0.0);
        LightDirectLambert += vec4(LightDirectRGB[i] * diff, 1.0);
    }
#fragment RimLight
    uniform vec3 RimColor;
    uniform mat4 MatrixView;
    in vec3 NormalModel;
    in vec3 FragPosWorld;
    out vec4 RimLight;
    vec3 camPos = inverse(MatrixView)[3].xyz;
    float diff = max(0.5 - dot(NormalModel, normalize(camPos - FragPosWorld)), 0.0);
    RimLight = vec4(RimColor * diff, 1.0);
        
#generic DistortFishEye
    in vec2 input;
    out vec2 output;
    
    output = input;
    vec2 v = output.xy / output.w;
    
    float theta = atan(v.y, v.x);
    float radius = length(v);
    radius = pow(radius, 0.8);
    v.x = radius * cos(theta);
    v.y = radius * sin(theta);
    output.xy = v.xy * output.w;

#fragment DebugRed
    out vec4 DebugRed = vec4(1.0, 0.0, 0.0, 1.0);
    
#fragment DebugBoneWeightColor
    in vec4 VertexBoneWeight4;
    out vec4 DebugBoneWeightColor = vec4(VertexBoneWeight4.xyz, 1.0);
    
#fragment DebugBoneIndexColor
    in vec4 VertexBoneIndex4;
    out vec4 DebugBoneIndexColor = vec4(VertexBoneIndex4.xyz, 1.0);
    
#generic multiply
    in vec4 first;
    in vec4 second;
    out vec4 result = first * second;
#generic add
    in vec4 first;
    in vec4 second;
    out vec4 result = first + second;

)"
