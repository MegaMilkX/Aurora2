R"(#version 450
#define MAX_BONE_COUNT 64
in vec3 Position;
in vec4 BoneIndex4;
in vec4 BoneWeight4;
in vec2 UV;
in vec3 Normal;

out vec2 UVFrag;
out vec3 NormalModel;
out vec3 FragPosWorld;
out vec4 DiffuseColor;

uniform mat4 MatrixModel;
uniform mat4 MatrixView;
uniform mat4 MatrixProjection;
uniform mat4 BoneInverseBindTransforms[MAX_BONE_COUNT];
uniform mat4 BoneTransforms[MAX_BONE_COUNT];

void main()
{
    vec4 PositionScreen;
    vec4 PositionModel;
    mat4 MatrixSkin;

    int bi0 = int ( BoneIndex4 . x ) ; 
    int bi1 = int ( BoneIndex4 . y ) ; 
    int bi2 = int ( BoneIndex4 . z ) ; 
    int bi3 = int ( BoneIndex4 . w ) ; 

    vec3 Pos = (MatrixModel * vec4(Position, 1.0)).xyz;
    vec3 Norm = (MatrixModel * vec4(Normal, 0.0)).xyz;

    PositionModel = (
        (BoneTransforms[bi0] * BoneInverseBindTransforms[bi0]) * vec4(Pos, 1.0) * BoneWeight4.x +
        (BoneTransforms[bi1] * BoneInverseBindTransforms[bi1]) * vec4(Pos, 1.0) * BoneWeight4.y +
        (BoneTransforms[bi2] * BoneInverseBindTransforms[bi2]) * vec4(Pos, 1.0) * BoneWeight4.z +
        (BoneTransforms[bi3] * BoneInverseBindTransforms[bi3]) * vec4(Pos, 1.0) * BoneWeight4.w 
    );
    vec3 NormalSkinned = (
        (BoneTransforms[bi0] * BoneInverseBindTransforms[bi0]) * vec4 ( Norm , 0.0 ) * BoneWeight4.x +
        (BoneTransforms[bi1] * BoneInverseBindTransforms[bi1]) * vec4 ( Norm , 0.0 ) * BoneWeight4.y +
        (BoneTransforms[bi2] * BoneInverseBindTransforms[bi2]) * vec4 ( Norm , 0.0 ) * BoneWeight4.z +
        (BoneTransforms[bi3] * BoneInverseBindTransforms[bi3]) * vec4 ( Norm , 0.0 ) * BoneWeight4.w 
    ).xyz;

    PositionScreen = MatrixProjection * MatrixView * PositionModel ; 

    FragPosWorld = vec3 ( PositionModel ) ; 
    NormalModel = normalize(NormalSkinned) ; 
    UVFrag = UV ; 
    DiffuseColor = vec4(1.0, 1.0, 1.0, 1.0);

    gl_Position = PositionScreen ; 
}
)"