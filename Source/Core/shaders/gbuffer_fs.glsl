R"(#version 450
    #define LIGHT_DIRECT_COUNT 2
    #define LIGHT_OMNI_COUNT 2
    in vec2 UVFrag;
    in vec3 NormalModel;
    in vec3 FragPosWorld;
    in vec4 DiffuseColor;

    out vec4 outAlbedo;
    out vec4 outPosition;
    out vec4 outNormal;
    out vec4 outSpecular;
    out vec4 outEmission;

    uniform sampler2D DiffuseTexture;
    uniform sampler2D SpecularTexture;
    uniform sampler2D NormalTexture;
    uniform vec3 AmbientColor;
    uniform vec3 ViewPos;

    uniform vec3 Tint;
    uniform float Glossiness;
    uniform float Emission;

    void main()
    {        
        outAlbedo = vec4(
            (
                DiffuseColor * texture(DiffuseTexture, UVFrag)
            ).xyz * Tint, 
            1.0
        );
        outPosition = vec4(FragPosWorld, 1.0);
        outNormal = vec4(NormalModel, 1.0);
        outSpecular = vec4(texture(SpecularTexture, UVFrag).xyz * Glossiness, 1.0);
        outEmission = vec4(Emission, Emission, Emission, 1.0);
    }
)"