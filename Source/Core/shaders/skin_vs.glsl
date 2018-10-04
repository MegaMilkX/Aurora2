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

uniform mat4 SkinBindPose;
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

    vec3 Pos = (SkinBindPose * vec4(Position, 1.0)).xyz;
    vec3 Norm = (SkinBindPose * vec4(Normal, 0.0)).xyz;

    vec4 weights = BoneWeight4;
    if(weights.x + weights.y + weights.z + weights.w > 1.0) {
        weights = normalize(weights);
    }
    float bindWeight = 1.0 - (weights.x + weights.y + weights.z + weights.w);

    PositionModel = (
        vec4(Pos, 1.0) * bindWeight +
        (BoneTransforms[bi0] * BoneInverseBindTransforms[bi0]) * vec4(Pos, 1.0) * weights.x +
        (BoneTransforms[bi1] * BoneInverseBindTransforms[bi1]) * vec4(Pos, 1.0) * weights.y +
        (BoneTransforms[bi2] * BoneInverseBindTransforms[bi2]) * vec4(Pos, 1.0) * weights.z +
        (BoneTransforms[bi3] * BoneInverseBindTransforms[bi3]) * vec4(Pos, 1.0) * weights.w 
    );
    vec3 NormalSkinned = (
        vec4(Norm , 0.0) * bindWeight +
        (BoneTransforms[bi0] * BoneInverseBindTransforms[bi0]) * vec4 ( Norm , 0.0 ) * BoneWeight4.x +
        (BoneTransforms[bi1] * BoneInverseBindTransforms[bi1]) * vec4 ( Norm , 0.0 ) * BoneWeight4.y +
        (BoneTransforms[bi2] * BoneInverseBindTransforms[bi2]) * vec4 ( Norm , 0.0 ) * BoneWeight4.z +
        (BoneTransforms[bi3] * BoneInverseBindTransforms[bi3]) * vec4 ( Norm , 0.0 ) * BoneWeight4.w 
    ).xyz;

    PositionScreen = MatrixProjection * MatrixView * MatrixModel * inverse(SkinBindPose) * PositionModel ; 

    FragPosWorld = (MatrixModel * inverse(SkinBindPose) * PositionModel).xyz; 
    NormalModel = normalize(MatrixModel * inverse(SkinBindPose) * vec4(NormalSkinned, 0.0)).xyz ; 
    UVFrag = UV ; 
    DiffuseColor = vec4(1.0, 1.0, 1.0, 1.0);

    gl_Position = PositionScreen ; 
}
)"