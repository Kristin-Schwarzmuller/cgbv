#version 430 // change later



// Structures
// =============================================================================================================
struct Matrices
{
	mat4 mvp;
	mat4 mv;
	mat3 normal;
};


struct Light
{
    vec3 lightPos;
};


struct FragmentInput
{
    vec3 normal;

	vec3 lightDir;
	vec3 viewDir;
};

struct Normalized
{
    vec3 normal;
    vec3 lightDir;
    vec3 viewDir;
};
// =============================================================================================================




// Subroutines
// =============================================================================================================
subroutine void FragmentProgram();
// =============================================================================================================




// Variables
// =============================================================================================================
layout(location = 0) in FragmentInput Input;

uniform Matrices matrices;

uniform Light light;

subroutine uniform FragmentProgram fragmentprogram;

layout(location = 0) out vec4 out_color;
// =============================================================================================================




// Methods
// =============================================================================================================
void main()
{
	fragmentprogram();
}
// =============================================================================================================




// Subroutine Implementation
// =============================================================================================================
layout (index = 0) subroutine (FragmentProgram) void toon()
{
    Normalized n;
    n.viewDir = normalize(Input.viewDir);
    n.lightDir = normalize(Input.lightDir);
    n.normal = normalize(Input.normal);

    float intensity = dot(n.lightDir, n.normal);

    vec4 toon = vec4(1.f);

    if(abs(intensity) > .9f)
        toon = vec4(1.f);

    else if(abs(intensity) > .8f)
        toon = vec4(.8f);

    else if(abs(intensity) > .7f)
        toon = vec4(.7f);

    else if(abs(intensity) > .6f)
        toon = vec4(.6f);

    else if(abs(intensity) > .5f)
        toon = vec4(.5f);

    else if(intensity > .4f)
        toon = vec4(.4f);

    else if(intensity > .3f)
        toon = vec4(.3f);

    else
        toon = vec4(.2f);

    toon.w = 1.f;


    out_color = toon * vec4((Input.normal * .5f) + .5f, 1);
}

// Subroutine einfarbing Rot
// =============================================================================================================
layout (index = 1) subroutine (FragmentProgram) void red()
{

    out_color = vec4(1,0,0,1);
}
// Subroutine ändern mit Parameter
// =============================================================================================================
layout (index = 2) subroutine (FragmentProgram) void changeByParam()
{

    //out_color = vec4(Input.lightDir.xyz,1);
	out_color = vec4((Input.normal * .5f) + .5f, 1);
}
// =============================================================================================================
layout (index = 3) subroutine (FragmentProgram) void phong()
{
	// Source: https://de.wikipedia.org/wiki/Phong-Beleuchtungsmodell

	// ambient: =========================== 
	/* unabh.: Einfallswinkel des Lichtstrahls der Punktlichtquelle 
	* unabhh.: Blickwinkel des Beobachters 
	* abh.: konstanten Umgebungslicht 
	* abh.: empirisch bestimmten Reflexionsfaktor (Materialkonstante). 
	* I_ambient = I_a * k_ambient */

	vec4 I_a = vec4(1, 1, 1, 1);
	vec4 k_ambient = vec4(.3f);

	vec4 out_ambient = I_a * k_ambient;
	
	// diffus: ===========================
	/* unabh.: Standpunkt des Betrachters in alle Richtungen reflektiert (Lambertsches Gesetz)
	* abh: Einfallswinkel des reflektierten Lichts 
	* abh.: empirisch bestimmten Reflexionsfaktor (Materialkonstante) 
	* I_diffus = I_in * k_diffus * (normalenVektor * einhVekLicht) */

	 vec4 I_in = vec4(1, 1, 1, 1);
	 vec4 k_diffus = vec4(0, 0, 1, 1);

	vec4 out_diffus =  I_in * k_diffus * dot(Input.lightDir.normalize, Input.normal); // beide noch zu normierene mit normalize

	// specular: ===========================
	/* abh.: Einfallswinkel des Lichtstrahls der Punktlichtquelle
	* abh.: empirisch bestimmten Reflexionsfaktor (Materialkonstante)
	* abh.: Oberflächenbeschaffenheit
	* abh.: Blickwinkel des Beobachters
	* I_specular = I_in * k_specular * (Reflexionsrichtung * Blickrichtung)^ Oberflächenbeschaffenheit */
	
	I_in = vec4(1, 1, 1, 1);
	vec4 k_specular = vec4(1, 0, 0, 1);;
	//Oberflächenbeschaffenheit (rau kleiner 32, glatt größer 32, {\displaystyle n=\infty } n=\infty  wäre ein perfekter Spiegel)
	float n = 32.0f; 
	//Normalisierungsfaktor
	float n_fac = (n + 2.0f) / (2.0f * 3.14f);

	//Reflexionsrichtung des ausfallenden Lichtstrahls
	//vec4 R = vec4();
	// Blickrichtung des Betrachters
	//vec4 V = ()

	//vec4 out_specular = I_in * k_specular * n_fac * pow(dot(R, V), n)

	out_color = out_ambient + out_diffus; // + out_specular;
}