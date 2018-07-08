R"(#version 450
#define MAX_BONE_COUNT 32
in vec3 Position;
in vec4 BoneIndex4;
in vec4 BoneWeight4;
in vec2 UV;
in vec3 Normal;

out vec2 UVFrag;
out vec3 NormalModel;
out vec3 FragPosWorld;

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
{
{
int bi0 = int ( BoneIndex4 . x ) ; 
int bi1 = int ( BoneIndex4 . y ) ; 
int bi2 = int ( BoneIndex4 . z ) ; 
int bi3 = int ( BoneIndex4 . w ) ; 
MatrixSkin = BoneTransforms [ bi0 ] * BoneInverseBindTransforms [ bi0 ] * BoneWeight4 . x + BoneTransforms [ bi1 ] * BoneInverseBindTransforms [ bi1 ] * BoneWeight4 . y + BoneTransforms [ bi2 ] * BoneInverseBindTransforms [ bi2 ] * BoneWeight4 . z + BoneTransforms [ bi3 ] * BoneInverseBindTransforms [ bi3 ] * BoneWeight4 . w ; 
}
{
PositionModel = MatrixSkin * vec4 ( Position , 1.0 ) ; 
}
{
PositionScreen = MatrixProjection * MatrixView * MatrixModel * PositionModel ; 
}
gl_Position = PositionScreen ; 
}
{
FragPosWorld = vec3 ( MatrixModel * PositionModel ) ; 
}
{
NormalModel = normalize ( ( MatrixModel * MatrixSkin * vec4 ( Normal , 0.0 ) ) . xyz ) ; 
}
{
UVFrag = UV ; 
}
{
int bi0 = int ( BoneIndex4 . x ) ; 
int bi1 = int ( BoneIndex4 . y ) ; 
int bi2 = int ( BoneIndex4 . z ) ; 
int bi3 = int ( BoneIndex4 . w ) ; 
MatrixSkin = BoneTransforms [ bi0 ] * BoneInverseBindTransforms [ bi0 ] * BoneWeight4 . x + BoneTransforms [ bi1 ] * BoneInverseBindTransforms [ bi1 ] * BoneWeight4 . y + BoneTransforms [ bi2 ] * BoneInverseBindTransforms [ bi2 ] * BoneWeight4 . z + BoneTransforms [ bi3 ] * BoneInverseBindTransforms [ bi3 ] * BoneWeight4 . w ; 
}
{
PositionModel = MatrixSkin * vec4 ( Position , 1.0 ) ; 
}
{
PositionScreen = MatrixProjection * MatrixView * MatrixModel * PositionModel ; 
}
gl_Position = PositionScreen ; 
}
)"