#include "src/model.h"


// "Lengyel method"
// när man använder en normal map (de bilderna som är typ lite blåa)
// metoden tar fram tangent och binormal vektorer som beskriver hur texturen är orienterad i världen

// så vi tar fram riktningar med hjälp av vertex position och texcoord så vi kan använda det i pixel shadern för att göra bump mapping

void Model::compute_TB(Vertex& v0, Vertex& v1, Vertex& v2)
{
	//två vektorer som beskriver triangelns plan i world space
    vec3f edge1 = v1.Position - v0.Position;
    vec3f edge2 = v2.Position - v0.Position;

	//hur mycket vi måste gå i u- och v-riktning för att komma från v0 till v1 och v2
    float deltaU1 = v1.TexCoord.x - v0.TexCoord.x;
    float deltaV1 = v1.TexCoord.y - v0.TexCoord.y;
    float deltaU2 = v2.TexCoord.x - v0.TexCoord.x;
    float deltaV2 = v2.TexCoord.y - v0.TexCoord.y;

	//om den här är 0 så är det en degenerate triangel, och har då ingen yta
	//f är en faktor som står för hur mycket vi måste gå i u- och v-riktning för att nå från v0 till v1 och v2
    float f = 1.0f / (deltaU1 * deltaV2 - deltaU2 * deltaV1);

	// tangent och binormal är vektorer som beskriver hur texturen är orienterad i världen
    // så att vi kan använda det i pixel shadern för att göra bump mapping
    vec3f tangent, binormal;

	//beräkna riktningen på tangent och binormal, och normalisera 
    tangent.x = f * (deltaV2 * edge1.x - deltaV1 * edge2.x);
    tangent.y = f * (deltaV2 * edge1.y - deltaV1 * edge2.y);
    tangent.z = f * (deltaV2 * edge1.z - deltaV1 * edge2.z);

    binormal.x = f * (-deltaU2 * edge1.x + deltaU1 * edge2.x);
    binormal.y = f * (-deltaU2 * edge1.y + deltaU1 * edge2.y);
    binormal.z = f * (-deltaU2 * edge1.z + deltaU1 * edge2.z);

    v0.Tangent = v1.Tangent = v2.Tangent = normalize(tangent);
    v0.Binormal = v1.Binormal = v2.Binormal = normalize(binormal);
}