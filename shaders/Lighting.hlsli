
static const uint COLOR_FLAG = 0x00000001u;
static const uint NORMAL_FLAG = 0x00000002u;
static const uint SPECULAR_FLAG = 0x00000004u;

struct SURFACE
{
    float3 position;
    float3 normal;
};

struct LIGHT
{
    float4 position;
    float4 direction;
    float4 attributes;
    float4 color;
};

struct ATTRIBUTES
{
    float3 Kd; // diffuse reflectivity
    float d; // dissolve (transparency) 
    float3 Ks; // specular reflectivity
    float Ns; // specular exponent
    float3 Ka; // ambient reflectivity
    float sharpness; // local reflection map sharpness
    float3 Tf; // transmission filter
    float Ni; // optical density (index of refraction)
    float3 Ke; // emissive reflectivity
    uint illum; // illumination model
};

float4 CalculateDirectionalLight(ATTRIBUTES mat, LIGHT light, SURFACE surface)
{
    float lightRatio = saturate(dot(-light.direction.xyz, surface.normal.xyz));
    float4 result = float4(light.color.xyz * lightRatio, 0.0f);
    return result;
}

float4 CalculatePointLight(ATTRIBUTES mat, LIGHT light, SURFACE surface)
{
    float3 toLight = float3(light.position.xyz - surface.position.xyz);
    float lightRatio = saturate(dot(normalize(toLight), surface.normal));
    float attenuation = 1.0f - saturate(length(toLight) / light.attributes.z);
    float intensity = light.attributes.w;
    float4 result = float4(light.color * lightRatio * pow(attenuation, 2.0f) * intensity);
    return result;
}

float4 CalculateSpotLight(ATTRIBUTES mat, LIGHT light, SURFACE surface)
{
    float3 toLight = normalize(light.position.xyz - surface.position.xyz);
    float toLightDistance = length(light.position.xyz - surface.position.xyz);
    float spotRatio = saturate(dot(-toLight, normalize(light.direction.xyz)));
    float intensity = light.attributes.w;
    float innerConeAngle = light.attributes.x;
    float outerConeAngle = light.attributes.y;
    float lightRatio = saturate(dot(toLight, surface.normal.xyz));
    float attenuation = 1.0f - saturate(toLightDistance / light.attributes.z);
    attenuation *= (1.0f - saturate((innerConeAngle - spotRatio) / (innerConeAngle - outerConeAngle)));
    float4 result = light.color * lightRatio * pow(attenuation, 2.0f) * intensity;
    return result;
}

float4 CalculateSpecular(ATTRIBUTES mat, LIGHT light, SURFACE surface, float3 cameraPos)
{
    float3 toCam = normalize(cameraPos - surface.position);
    float3 toLight = normalize(light.position.xyz - surface.position.xyz);
    float attenuation = 1.0f;
    float specPower = mat.Ns;
    [branch]
    switch (int(light.position.w))
    {
        case 0:
            toLight = -normalize(light.direction.xyz);
            specPower = 256.0f;
            break;
        case 1:
            attenuation = 1.0f - saturate(length(light.position.xyz - surface.position.xyz) / light.attributes.z);
            break;
        case 2:
            float spotRatio = saturate(dot(-toLight, normalize(light.direction.xyz)));
            float innerConeAngle = light.attributes.x;
            float outerConeAngle = light.attributes.y;
            attenuation = 1.0f - saturate(length(light.position.xyz - surface.position.xyz) / light.attributes.z);
            attenuation *= (1.0f - saturate((innerConeAngle - spotRatio) / (innerConeAngle - outerConeAngle)));
            break;
        default:
            break;
    };
    
    float inLight = dot(toLight, surface.normal.xyz);
    float3 reflec = normalize(reflect(-toLight, surface.normal));
    float specIntensity = pow(saturate(dot(toCam, reflec)), specPower);
    float4 spec = float4(light.color.xyz * mat.Ks * specIntensity * pow(attenuation, 2.0f) * inLight, 0.0f);
    return spec;
};

float4 CalculateLight(ATTRIBUTES mat, LIGHT light, SURFACE surface)
{
    float4 luminance = float4(0, 0, 0, 0);
    [branch]
    switch (int(light.position.w))
    {
        case 0: // directional light
            luminance += CalculateDirectionalLight(mat, light, surface);
            break;
        case 1: // point light
            luminance += CalculatePointLight(mat, light, surface);
            break;
        case 2: // spot light
            luminance += CalculateSpotLight(mat, light, surface);
            break;
        default:
            break;
    }
    return luminance;
};

float3x3 CotangentFrame(float3 normal, float3 view, float2 texcoord)
{
    // get edge vectors of the pixel triangle
    float3 dp1 = ddx(view);
    float3 dp2 = ddy(view);
    float2 duv1 = ddx(texcoord);
    float2 duv2 = ddy(texcoord);

    // solve the linear system
    float3 dp2perp = cross(dp2, normal);
    float3 dp1perp = cross(normal, dp1);
    float3 T = (dp2perp * duv1.x + dp1perp * duv2.x);
    float3 B = (dp2perp * duv1.y + dp1perp * duv2.y);

    // construct a scale-invariant frame
    float invmax = rsqrt(max(dot(T, T), dot(B, B)));
    // returns a column-major matrix
    return transpose(float3x3(T * invmax, B * invmax, normal));
}

float3 PerturbNormal(float3 normal, float3 view, float2 texcoord, float3 lookup)
{
    float3x3 TBN = CotangentFrame(normal, -view, texcoord);
    return normalize(mul(TBN, lookup));
}