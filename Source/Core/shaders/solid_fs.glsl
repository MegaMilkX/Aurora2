R"(#version 450
    #define LIGHT_DIRECT_COUNT 2
    #define LIGHT_OMNI_COUNT 2
    in vec2 UVFrag;
    in vec3 NormalModel;
    in vec3 FragPosWorld;

    out vec4 fragOut;

    uniform sampler2D DiffuseTexture;
    uniform vec3 AmbientColor;
    uniform vec3 LightDirectRGB[LIGHT_DIRECT_COUNT];
    uniform vec3 LightDirect[LIGHT_DIRECT_COUNT];
    uniform vec3 LightOmniRGB[LIGHT_OMNI_COUNT];
    uniform vec3 LightOmniPos[LIGHT_OMNI_COUNT];
    uniform vec3 ViewPos;

    float fogFactorExp(
        const float dist,
        const float density
    ) {
        return 1.0 - clamp(exp(-density * dist), 0.0, 1.0);
    }

    vec3 applyFog( in vec3  rgb,      // original color of the pixel
                in float distance, // camera to point distance
                in vec3  rayOri,   // camera position
                in vec3  rayDir )  // camera to point vector
    {
        float fogAmount = c*exp(-rayOri.y*b)*(1.0-exp(-distance*rayDir.y*b))/rayDir.y;
        vec3  fogColor  = vec3(0.5,0.6,0.7);
        return mix( rgb, fogColor, fogAmount );
    }

    void main()
    {
        vec4 add2;
        vec4 add1;
        vec4 multiply0;
        vec4 Diffuse;
        vec4 Ambient;
        vec4 LightDirectLambert;
        vec4 SpecDirect;
        vec4 SpecOmni;
        vec4 LightOmniLambert;
        vec3 LightDirection[LIGHT_OMNI_COUNT];
        
        for (int i = 0; i < LIGHT_OMNI_COUNT; i++) 
        { 
            LightDirection[i] = normalize(LightOmniPos[i] - FragPosWorld); 
        }

        vec3 ViewDir = normalize(ViewPos - FragPosWorld);
        vec4 SpecularColor = vec4(0.0, 0.0, 0.0, 1.0); 
        
        LightOmniLambert = vec4 ( 0.0 , 0.0 , 0.0 , 1.0 ) ; 
        for (int i = 0; i < LIGHT_OMNI_COUNT; i++) 
        { 
            float diff = max(dot(NormalModel, LightDirection[i]) , 0.0); 
            float dist = distance ( LightOmniPos [ i ] , FragPosWorld ) ; 
            LightOmniLambert += vec4 ( LightOmniRGB [ i ] * diff * ( 1.0 / ( 1.0 + 0.5 * dist + 3.0 * dist * dist ) ) , 1.0 ) ; 
            if(dot(NormalModel, ViewDir) >= 0.0)
            {
                vec3 lightRef = normalize(reflect(-LightDirection[i], NormalModel));
                float s = max(pow(dot(lightRef, ViewDir), 16.0), 0.0);
                SpecOmni += vec4(LightOmniRGB [ i ] * s * ( 1.0 / ( 1.0 + 0.5 * dist + 3.0 * dist * dist ) ), 1.0);
            }
        } 
        
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

        float fogDistance = gl_FragDepth ;
        float fogFactor = fogFactorExp(fogDistance, 1.05);
        
        
        Ambient = vec4 ( AmbientColor , 1.0 ) ; 
        vec4 Light = 
            Ambient + 
            LightDirectLambert + 
            LightOmniLambert + 
            SpecOmni +
            SpecDirect;
        Diffuse = texture ( DiffuseTexture , UVFrag ) ; 
        Diffuse *= Light ; 

        Diffuse = vec4(
            applyFog(
                Diffuse.xyz,
                length(FragPosWorld - ViewPos),
                ViewPos,
                FragPosWorld - ViewPos
            ), 1.0
        );

        fragOut = Diffuse; 
    }
)"