
Texture2D texDiffuse : register(t0);
SamplerState texSampler : register(s0); //sampler som används för att hämta färg från texturen
Texture2D texNormal : register(t1); //normal map textur

struct PSIn
{
    float4 Pos : SV_Position;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float3 Binormal : BINORMAL;
    float2 TexCoord : TEX;
    float3 PosWorld : POSWORLD;
};

cbuffer LightCamBuffer : register(b0)
{
    float4 LightPosition;
    float4 CameraPosition;
};

cbuffer MaterialBuffer : register(b1)
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;
};


//-----------------------------------------------------------------------------------------
// Pixel Shader
//-----------------------------------------------------------------------------------------

float4 PS_main(PSIn input) : SV_Target
{
	// Debug shading #1: map and return normal as a color, i.e. from [-1,1]->[0,1] per component
	// The 4:th component is opacity and should be = 1
	//return float4(input.Normal*0.5+0.5, 1);
	
	
	// Debug shading #2: map and return texture coordinates as a color (blue = 0)
    //	return float4(input.TexCoord, 0, 1);
    
    
    float3 T = normalize(input.Tangent); //ytans tangentvektor
    float3 B = normalize(input.Binormal); //ytans binormalvektor
    float3 N = normalize(input.Normal); //ytans normala vektor
    float3x3 TBN = float3x3(T, B, N); //TBN-matris som används för att transformera normaler från tangent space till world space
    
    // Sampla normalen från normal-mappen (t1) färgen från bilden
    
        
        float4 normalSample = texNormal.Sample(texSampler, input.TexCoord);
    // gör om normalsample från färg (0-1) till normalvektor (-1,1)
    float3 localNormal = normalSample.rgb * 2.0f - 1.0f;
    // Transformera lokalen normalen till World Space med hjälp av TBN
    float3 bumpedNormalW = normalize(mul(localNormal, TBN)); //rätt ordning
    //float3 bumpedNormalW = normalize(mul(TBN, localNormal));
    
    
    //om det inte finns en normal i normal-mappen (dvs alpha < 1) så använder vi den vanliga normalen istället för den bumpade normalen
    if (normalSample.a < 1) 
    {
        bumpedNormalW = N;
    }
    
    if (input.Pos.x > 500)
    {
        bumpedNormalW = N;
    }
    
        float3 L = normalize(LightPosition.xyz - input.PosWorld); //vektorn från ytan till ljuskällan
    float3 V = normalize(CameraPosition.xyz - input.PosWorld); //vektorn från ytan till kameran
    float3 R = reflect(-L, bumpedNormalW); //reflektionvektorn, -1*L eftersom reflect förväntar sig en vektor från ljuskällan till ytan, och vi har L som är från ytan till ljuskällan. N är den normala vektorn på ytan.
    //byter ut N mot bumpedNormalW för att använda den bumpade normalen istället för den vanliga normalen i ljusberäkningarna
    
    
    //Ambient (grundljus, dvs färget ett objekt kommer ha vid 0 ljus)
    float3 ambient = Ambient.rgb;
    //Diffuse (lambert), dvs hur mycket ljus som reflekteras från ytan, beroende på vinkeln mellan ljus och yta. Ju mer rakt ljuset träffar ytan, desto mer ljus reflekteras.
    //0.0f för att inte få negativt ljus när ljuset kommer från "fel" håll, dvs bakom ytan
    //dot(N, L) ger oss cosinus av vinkeln mellan N och L, vilket är det som lambertian reflectance bygger på. Ju mer rakt ljuset träffar ytan, desto större cosinusvärde och därmed mer reflekterat ljus.
    //om de pekar i samma riktning (vinkel 0) så är dot(N, L) = 1 dvs max ljus
    float diff = max(dot(bumpedNormalW, L), 0.0f); 
    //byter ut N mot bumpedNormalW för att använda den bumpade normalen istället för den vanliga normalen i ljusberäkningarna
    float3 diffuse = diff * Diffuse.rgb; //mulitplicerar diff med materialets diffuse färg för att få den slutliga diffuse färgen
    //Specular (blänk), dvs hur mycket ljus som reflekteras i en specifik riktning
    //beroende på vinkeln mellan reflektionsvektorn R och vektorn till kameran V. Ju mer R pekar mot V, desto mer blänk. (samma som diff åvan)
    float spec = pow(max(dot(R, V), 0.0f), Specular.w);
    float3 specular = spec * Specular.rgb; //mulitplicerar spec med materialets specular färg för att få den slutliga specular färgen
    
    //tex sampler för att hämta färgen från texturen, multiplicerar den med ambient och diffuse
    float3 texColor = texDiffuse.Sample(texSampler, input.TexCoord * 1.0f).rgb; //multiplicerar texCoord med 1.0f så det är lätt att ändra till ex:3.0f för att via fler repitioner av texturen
    
    //multiplicerar ambient och diffuse med texturfärgen för att få den slutliga färgen
    float3 finalAmbient = ambient * texColor;
    //multiplicerar ambient och diffuse med texturfärgen för att få den slutliga färgen
    float3 finalDiffuse = diffuse * texColor;
    
    ////test med röd färg istället för material buffer
    //ambient
    //float3 ambient = float3(0.1f, 0.1f, 0.1f); // Mörkgrått grundljus
    ////diffuse
    //float diff = max(dot(N, L), 0.0f);
    //float3 diffuse = diff * float3(0.8f, 0.2f, 0.2f); // Röd färg
    ////specualr
    //float spec = pow(max(dot(R, V), 0.0f), 32.0f); // 32.0f är Shininess
    //float3 specular = spec * float3(1.0f, 1.0f, 1.0f); // Vitt blänk

    
    float3 finalColor = finalAmbient + finalDiffuse + specular;
    return float4(finalColor, 1.0f);
    
}