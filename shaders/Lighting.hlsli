
static const uint COLOR_FLAG = 0x00000001u;
static const uint NORMAL_FLAG = 0x00000002u;
static const uint SPECULAR_FLAG = 0x00000004u;
static const float PI = 3.14159f;
static const float EPSILON = 0.000001f;

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

float4 CalculateDirectionalLight(LIGHT light, SURFACE surface)
{
    float lightRatio = saturate(dot(-light.direction.xyz, surface.normal.xyz));
    float4 result = float4(light.color.xyz * lightRatio, 0.0f);
    return result;
}

float4 CalculatePointLight(LIGHT light, SURFACE surface)
{
    float3 toLight = float3(light.position.xyz - surface.position.xyz);
    float lightRatio = saturate(dot(normalize(toLight), surface.normal));
    float attenuation = 1.0f - saturate(length(toLight) / light.attributes.z);
    float intensity = light.attributes.w;
    float4 result = float4(light.color * lightRatio * pow(attenuation, 2.0f) * intensity);
    return result;
}

float4 CalculateSpotLight(LIGHT light, SURFACE surface)
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

float4 CalculateLight(LIGHT light, SURFACE surface)
{
    float4 luminance = float4(0, 0, 0, 0);
    switch (int(light.position.w))
    {
        case 0: // directional light
            luminance += CalculateDirectionalLight(light, surface);
            break;
        case 1: // point light
            luminance += CalculatePointLight(light, surface);
            break;
        case 2: // spot light
            luminance += CalculateSpotLight(light, surface);
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
    float3 dp2perp = cross(dp2, normal);    // z = cross(x,y) ??
    float3 dp1perp = cross(normal, dp1);    // x = cross(y,z) ??
    float3 T = (dp2perp * duv1.x + dp1perp * duv2.x);
    float3 B = (dp2perp * duv1.y + dp1perp * duv2.y);

    float invmax = rsqrt(max(dot(T, T), dot(B, B)));
    return float3x3(T * invmax, B * invmax, normal);
}

float3 PerturbNormal(float3 normal, float3 view, float2 texcoord, float3 lookup)
{
    float3x3 TBN = CotangentFrame(normal, -view, texcoord);
    return normalize(mul(lookup, TBN));
}

//////////////////////////////////////////////////////////////////
// Normal Distribution Functions
float D_GGX(float NoH, float a)
{
    float a2 = a * a;
    float f = (NoH * a2 - NoH) * NoH + 1.0;
    return a2 / (PI * f * f);
}

// https://youtu.be/RRE-F57fbXw?t=684
float D_GGX_Trowbridge_Reitz(float alpha, float3 N, float3 H)
{
    float numerator = pow(alpha, 2.0f);
    float NdotH = max(dot(N, H), 0.0f);
    float denominator = max(PI * pow(pow(NdotH, 2.0f) * (numerator - 1.0f) + 1.0f, 2.0f), EPSILON);
    return numerator / denominator;
}
// End Normal Distribution Functions
//////////////////////////////////////////////////////////////////

float G_SchlickBeckmann(float alpha, float3 N, float3 X)
{
    float numerator = max(dot(N, X), 0.0f);
    float k = alpha / 2.0f;
    float denominator = max(numerator * (1.0f - k) + k, EPSILON);
    return numerator / denominator;
}

float G_Smith(float alpha, float3 N, float3 V, float3 L)
{
    return G_SchlickBeckmann(alpha, N, V) * G_SchlickBeckmann(alpha, N, L);

}

//////////////////////////////////////////////////////////////////
// Fresnel
float3 F_Schlick(float3 f0, float3 V, float3 H)
{
    return f0 + (float3(1.0, 1.0f, 1.0f) - f0) * pow(1.0 - max(dot(V, H), 0.0f), 5.0);
}

//////////////////////////////////////////////////////////////////

float V_SmithGGXCorrelated(float NoV, float NoL, float a)
{
    float a2 = a * a;
    float GGXL = NoV * sqrt((-NoL * a2 + NoL) * NoL + a2);
    float GGXV = NoL * sqrt((-NoV * a2 + NoV) * NoV + a2);
    return 0.5 / (GGXV + GGXL);
}

float Fd_Lambert()
{
    return 1.0 / PI;
}

// F0 = ???
// N = normalized surface normal
// L = normalized light direction (toLight)
// V = normalized view direction (toCamera)
// H = normalized halfvector (reflected vector)
float3 BRDF(float3 F0, float3 N, float3 L, float3 V, float3 H, float alpha)
{
    float3 albedo = float3(1.0f, 0.0f, 0.0f);
    float3 emissive = float3(0.0f, 0.0f, 0.0f);
    float3 lightColor = float3(1.0f, 1.0f, 1.0f);
    float3 Ks = F_Schlick(F0, V, H);
    float3 Kd = float3(1.0f, 1.0f, 1.0f) - Ks;
    float3 lambert = albedo / PI;
    float3 D = D_GGX_Trowbridge_Reitz(alpha, N, H);
    float3 G = G_Smith(alpha, N, V, L);
    float3 F = Ks;
    float3 cookTorranceNumerator = D * G * F;
    float cookTorranceDenominator = max(4.0f * max(dot(V, N), 0.0f) * max(dot(L, N), 0.0f), EPSILON);
    float3 cookTorrance = cookTorranceNumerator / cookTorranceDenominator;
    float3 BRDF = Kd * lambert + cookTorrance;
    float3 value = emissive + BRDF * lightColor * max(dot(N, L), 0.0f);
    return BRDF;
}