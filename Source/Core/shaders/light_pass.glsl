R"(#version 450
#define LIGHT_DIRECT_COUNT 2
#define LIGHT_OMNI_COUNT 32

in vec2 UVFrag;
out vec4 fragOut;
uniform sampler2D inAlbedo;
uniform sampler2D inPosition;
uniform sampler2D inNormal;
uniform sampler2D inSpecular;

uniform vec3 ViewPos;

uniform vec3 LightDirectRGB[LIGHT_DIRECT_COUNT];
uniform vec3 LightDirect[LIGHT_DIRECT_COUNT];
uniform vec3 LightOmniRGB[LIGHT_OMNI_COUNT];
uniform vec3 LightOmniPos[LIGHT_OMNI_COUNT];

void main()
{
    vec4 LightDirectLambert;
    vec4 SpecDirect;
    vec4 SpecOmni;
    vec4 LightOmniLambert;
    vec3 NormalModel = texture(inNormal, UVFrag).xyz;
    vec3 FragPosWorld = texture(inPosition, UVFrag).xyz;
    vec3 ViewDir = normalize(ViewPos - FragPosWorld);

    LightDirectLambert = vec4 ( 0.0 , 0.0 , 0.0 , 1.0 ) ; 
    for (int i = 0; i < LIGHT_DIRECT_COUNT; ++i)
    { 
        float diff = max ( dot ( NormalModel , - LightDirect [ i ] ) , 0.0 ) ; 
        LightDirectLambert += vec4 ( LightDirectRGB [ i ] * diff , 1.0 ) ; 
        if(dot(NormalModel, ViewDir) >= 0.0)
        {
            vec3 lightRef = normalize(reflect(LightDirect[i], NormalModel));
            float s = pow(max(dot(lightRef, ViewDir), 0.0), 16.0);
            SpecDirect += vec4(LightOmniRGB [ i ] * s, 1.0);
        }
    }

    LightOmniLambert = vec4 ( 0.0 , 0.0 , 0.0 , 1.0 ) ; 
    for (int i = 0; i < LIGHT_OMNI_COUNT; i++) 
    { 
        vec3 lightDirection = normalize(LightOmniPos[i] - FragPosWorld);

        float diff = max(dot(NormalModel, lightDirection) , 0.0); 
        float dist = distance ( LightOmniPos [ i ] , FragPosWorld ) ; 
        LightOmniLambert += vec4 ( LightOmniRGB [ i ] * diff * ( 1.0 / ( 1.0 + 0.5 * dist + 3.0 * dist * dist ) ) , 1.0 ) ; 
        if(dot(NormalModel, ViewDir) >= 0.0)
        {
            vec3 lightRef = normalize(reflect(-lightDirection, NormalModel));
            float s = max(pow(dot(lightRef, ViewDir), 16.0), 0.0);
            SpecOmni += vec4(LightOmniRGB [ i ] * s * ( 1.0 / ( 1.0 + 0.5 * dist + 3.0 * dist * dist ) ), 1.0);
        }
    } 

    vec4 Light = LightDirectLambert 
        + LightOmniLambert
        + SpecDirect
        + SpecOmni;

    Light = vec4(
        Light.x / (Light.x + 1.0),
        Light.y / (Light.y + 1.0),
        Light.z / (Light.z + 1.0),
        1.0
    );

    fragOut = texture(inAlbedo, UVFrag) * Light;
}

)"