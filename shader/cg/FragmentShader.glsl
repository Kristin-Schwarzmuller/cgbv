#version 430 // change later

// Eingabe-Werte pro Vertex
in vec4 vVertex; // Vertex-Position in Objektkoordinaten

// Uniform-Eingabe-Werte
uniform mat4 MV; // ModelView-Matrix
uniform mat3 NormalM; // Normalen-Matrix

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
	// Source: Buch Seite 234 und https://de.wikipedia.org/wiki/Phong-Beleuchtungsmodell

	// ambient: =========================== 
	/* unabh.: Einfallswinkel des Lichtstrahls der Punktlichtquelle 
	* unabhh.: Blickwinkel des Beobachters 
	* abh.: konstanten Umgebungslicht 
	* abh.: empirisch bestimmten Reflexionsfaktor (Materialkonstante). 
	* I_ambient = I_a * k_ambient */

	vec4 I_a = vec4(1, 0, 1, 1);
	vec4 k_ambient = vec4(.3f);

	vec4 out_ambient = I_a * k_ambient;
	
	// diffus: ===========================
	/* unabh.: Standpunkt des Betrachters in alle Richtungen reflektiert (Lambertsches Gesetz)
	* abh: Einfallswinkel des reflektierten Lichts 
	* abh.: empirisch bestimmten Reflexionsfaktor (Materialkonstante) 
	* I_diffus = I_in * k_diffus * (normalenVektor * einhVekLicht) */

	 vec4 I_in = vec4(1, 1, 1, 1);
	 vec4 k_diffus = vec4(0, 0, 1, 1);

	vec4 out_diffus =  I_in * k_diffus * max(dot(normalize(Input.lightDir), normalize(Input.normal)), .0f); // beide noch zu normierene mit normalize

	// specular: ===========================
	/* abh.: Einfallswinkel des Lichtstrahls der Punktlichtquelle
	* abh.: empirisch bestimmten Reflexionsfaktor (Materialkonstante)
	* abh.: Oberflächenbeschaffenheit
	* abh.: Blickwinkel des Beobachters
	* I_specular = I_in * k_specular * (Reflexionsrichtung * Blickrichtung)^ Oberflächenbeschaffenheit */
	
	I_in = vec4(0, 1, 0, 1);
	vec4 k_specular = vec4(1, 1, 0, 1);
	vec4 out_specular = vec4(.0f);

	//Oberflächenbeschaffenheit (rau kleiner 32, glatt größer 32, n=unenhdlich  wäre ein perfekter Spiegel)
	float shininess = 32.0f; 
	//Normalisierungsfaktor
	float n_fac = (shininess + 2.0f) / (2.0f * 3.14f);

	// Richtungslichtquelle
	vec3 L = normalize(Input.lightDir);
	// Normalen-Vektor aus Objekt- in Augenpunktskoordinaten
	vec3 N = NormalM *  normalize(Input.normal);
	?oat diffuseLight = max(dot(N, L), 0.0);
	// Augenpunktsvektor A Siehe PDF Seite 346, keine Ahnung welche Buchseite
	vec3 A = normalize(vec3(.0f, 0.f, 1.f));
	// Halfway-Vektor: h = (l + a)/ | l + a | mit Augenpunktsvektor a und Lichtvetor l, siehe Buch S. 232
	vec3 H = normalize((L + A)/ abs(L + A ));

	out_specular = I_in * k_specular * n_fac * pow(max(dot(H, N), 0), shininess);

	out_color = out_ambient + out_diffus + out_specular;
}