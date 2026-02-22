
Texture2D texDiffuse : register(t0);

struct PSIn
{
    float4 Pos : SV_Position;
    float3 Normal : NORMAL;
    float2 TexCoord : TEX;
    float3 PosWorld : POSWORLD;
};

cbuffer LightCamBuffer : register(b0)
{
    float4 CameraPosition;
    float4 LightPosition;
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
    
    
    
    
    float3 N = normalize(input.Normal); //ytans normala vektor
    float3 L = normalize(LightPosition.xyz - input.PosWorld); //vektorn från ytan till ljuskällan
    float3 V = normalize(CameraPosition.xyz - input.PosWorld); //vektorn från ytan till kameran
    float3 R = reflect(-L, N); //reflektionvektorn, -1*L eftersom reflect förväntar sig en vektor från ljuskällan till ytan, och vi har L som är från ytan till ljuskällan. N är den normala vektorn på ytan.

    //Ambient (grundljus, dvs färget ett objekt kommer ha vid 0 ljus)
    float3 ambient = Ambient.rgb;
    //Diffuse (lambert), dvs hur mycket ljus som reflekteras från ytan, beroende på vinkeln mellan ljus och yta. Ju mer rakt ljuset träffar ytan, desto mer ljus reflekteras.
    //0.0f för att inte få negativt ljus när ljuset kommer från "fel" håll, dvs bakom ytan
    //dot(N, L) ger oss cosinus av vinkeln mellan N och L, vilket är det som lambertian reflectance bygger på. Ju mer rakt ljuset träffar ytan, desto större cosinusvärde och därmed mer reflekterat ljus.
    //om de pekar i samma riktning (vinkel 0) så är dot(N, L) = 1 dvs max ljus
    float diff = max(dot(N, L), 0.0f); 
    float3 diffuse = diff * Diffuse.rgb; //mulitplicerar diff med materialets diffuse färg för att få den slutliga diffuse färgen
    //Specular (blänk), dvs hur mycket ljus som reflekteras i en specifik riktning
    //beroende på vinkeln mellan reflektionsvektorn R och vektorn till kameran V. Ju mer R pekar mot V, desto mer blänk. (samma som diff åvan)
    float spec = pow(max(dot(R, V), 0.0f), Specular.w);
    float3 specular = spec * Specular.rgb; //mulitplicerar spec med materialets specular färg för att få den slutliga specular färgen
    
    
    ////test med röd färg istället för material buffer
    //ambient
    //float3 ambient = float3(0.1f, 0.1f, 0.1f); // Mörkgrått grundljus
    ////diffuse
    //float diff = max(dot(N, L), 0.0f);
    //float3 diffuse = diff * float3(0.8f, 0.2f, 0.2f); // Röd färg
    ////specualr
    //float spec = pow(max(dot(R, V), 0.0f), 32.0f); // 32.0f är Shininess
    //float3 specular = spec * float3(1.0f, 1.0f, 1.0f); // Vitt blänk

    
    float3 finalColor = ambient + diffuse + specular;

    return float4(finalColor, 1.0f);
    
}