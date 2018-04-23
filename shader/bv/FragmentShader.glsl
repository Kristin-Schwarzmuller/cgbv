#version 430 // change later

// Structures
// =============================================================================================================
struct Matrices
{
	mat4 mvp;
};


struct FragmentInput
{
	vec2 uv;
};


struct Textures
{
	sampler2DRect tex;
};


struct Parameter
{
    vec4 paramA;
    vec4 paramB;
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
uniform Textures textures;

subroutine uniform FragmentProgram fragmentprogram;


uniform Parameter parameter;


uniform vec2 offsets3x3[9] = vec2[]
    (
        vec2(-1,  -1), vec2(-1,  0), vec2(-1, 1),
		vec2( 0,  -1), vec2( 0,  0), vec2( 0, 1),
		vec2( 1,  -1), vec2( 1,  0), vec2( 1, 1)
	);

uniform vec2 offsets5x5[25] = vec2[] 
(
		vec2(-2, -2), vec2(-2, -1), vec2(-2, 0), vec2(-2, 1), vec2(-2, 2),
		vec2(-1, -2), vec2(-1, -1), vec2(-1, 0), vec2(-1, 1), vec2(-1, 2),
		vec2(0, -2), vec2(0, -1), vec2(0, 0), vec2(0, 1), vec2(0, 2),
		vec2(1, -2), vec2(1, -1), vec2(1, 0), vec2(1, 1), vec2(1, 2),
		vec2(2, -2), vec2(2, -1), vec2(2, 0), vec2(2, 1), vec2(2, 2)
);

uniform vec2 offsets7x7[49] = vec2[]
(
vec2(-3, -3), vec2(-3, -2), vec2(-3, -1), vec2(-3, 0), vec2(-3, 1), vec2(-3, 2), vec2(-3, 3),
vec2(-2, -3), vec2(-2, -2), vec2(-2, -1), vec2(-2, 0), vec2(-2, 1), vec2(-2, 2), vec2(-2, 3),
vec2(-1, -3), vec2(-1, -2), vec2(-1, -1), vec2(-1, 0), vec2(-1, 1), vec2(-1, 2), vec2(-1, 3),
vec2(0, -3), vec2(0, -2), vec2(0, -1), vec2(0, 0), vec2(0, 1), vec2(0, 2), vec2(0, 3),
vec2(1, -3), vec2(1, -2), vec2(1, -1), vec2(1, 0), vec2(1, 1), vec2(1, 2), vec2(1, 3),
vec2(2, -3), vec2(2, -2), vec2(2, -1), vec2(2, 0), vec2(2, 1), vec2(2, 2), vec2(2, 3),
vec2(3, -3), vec2(3, -2), vec2(3, -1), vec2(3, 0), vec2(3, 1), vec2(3, 2), vec2(3, 3)
);




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
layout (index = 0) subroutine (FragmentProgram) void passthrough()
{
    out_color = texture(textures.tex, Input.uv);
}


layout (index = 1) subroutine (FragmentProgram) void mean_value()
{
    vec4 texel = vec4(0.f, 0.f, 0.f, 1.f);

    //   1  1  1
    //   1  1  1
    //   1  1  1

    for(int i = 0; i < 9; ++i)
        texel += texture(textures.tex, Input.uv + offsets3x3[i]);

    out_color = texel / 9.f;
}


layout (index = 2) subroutine (FragmentProgram) void brightness_contrast()
{
	/* Op Parameter: Param #1 X
	* Scale in side menu just in integer steps, but a picture with brightness one is just white --> scale the param with 0.1
	*/
	float brightness = parameter.paramA.x * 0.1;

	/* Op Parameter: Param #2 X
	* Scale in side menu just in integer steps, for a better adjustment --> scale the param with 0.1
	* Due to that, the initial value is still 0*0.1, that's just gray --> the value should not be 0 --> define the range with the function clamp
	* But than, the initial value is still 0.01 and not the original pricture --> add 1 so it is 1.01 and ca. the picture we want
	*/
	// lecture formular just for changing contrast: sk(x,y) = s(x,y) * contrast + 0.5(1 - contrast)
	float contrast = clamp((parameter.paramB.x * 0.1) + 1.0, 0.01, 20);

	// lecture: s'(x,y) = (s(x,y) + brightness) *contrast

	// texture(textures.tex, Input.uv) is the color at the pixel with the coordinates u and v
    out_color = (texture(textures.tex, Input.uv) + brightness) * contrast + 0.5 * (1 - contrast);
}

layout (index = 3) subroutine (FragmentProgram) void sharpen()
{
	vec4 texel;
	float param = parameter.paramA.w * 0.1;
	float brightness = parameter.paramA.x * 0.1;
	float contrast = clamp((parameter.paramB.x * 0.1) + 1.0, 0.01, 20);
	
	// Lapacefilter ------------------------------------------------
	float H[9] = float[](	 0.f, -1.f,  0.f, 
							-1.f,  4.f, -1.f,
							 0.f, -1.f,  0.f);

	for(int i = 0; i < 9; ++i)
        texel += H[i] * texture(textures.tex, Input.uv + offsets3x3[i]);

<<<<<<< HEAD
	vec4 lapaceFiltered = ((texel / 2  + 0.5) + brightness) * contrast + 0.5 * (1 - contrast);
=======
	vec4 lapaceFiltered = texel;//((texel / 2  + 0.5) + brightness) * contrast + 0.5 * (1 - contrast);
>>>>>>> 16939e2833e74f96f45f45c91380cf0847af4d94
	// -------------------------------------------------------------

	// Sharpen = Original + c * lapace
    out_color = texture(textures.tex, Input.uv) + param * lapaceFiltered;
}


layout (index = 4) subroutine (FragmentProgram) void dilatation()
{
    vec4 texel = vec4(0.f, 0.f, 0.f, 1.f);
	vec4 max = vec4(0.f, 0.f, 0.f, 1.f);

    //   1  1  1
    //   1  1  1
    //   1  1  1

	// Auslesen der 3x3 Umgebung und Bestimmung der Maxima im R, G und B Bereich
    for(int i = 0; i < 9; ++i) {
        texel = texture(textures.tex, Input.uv + offsets3x3[i]);
		if (texel.x > max.x)
			max.x = texel.x;
		if (texel.y > max.y)
			max.y = texel.y;
		if (texel.z > max.z)
			max.z = texel.z;
	}
    out_color = max;
}


layout (index = 5) subroutine (FragmentProgram) void erosion()
{
    vec4 texel = vec4(0.f, 0.f, 0.f, 1.f);
	vec4 min = vec4(1.f, 1.f, 1.f, 1.f);
	float param  = parameter.paramA.w;

    //   1  1  1
    //   1  1  1
    //   1  1  1
<<<<<<< HEAD
	
	// Auslesen der 3x3 Umgebung und Bestimmung der Minima im R, G und B Bereich
    for(int i = 0; i < 9; ++i) {
        texel = texture(textures.tex, Input.uv + offsets3x3[i]);
		if (texel.x < min.x)
			min.x = texel.x;
		if (texel.y < min.y)
			min.y = texel.y;
		if (texel.z < min.z)
			min.z = texel.z;
=======
	if(param == 0.f){ 
		// Auslesen der 3x3 Umgebung und Bestimmung der Minima im R, G und B Bereich
		for(int i = 0; i < 9; ++i) {
			texel = texture(textures.tex, Input.uv + offsets3x3[i]);
			if (texel.x < min.x)
				min.x = texel.x;
			if (texel.y < min.y)
				min.y = texel.y;
			if (texel.z < min.z)
				min.z = texel.z;
		}
	}
	else if (param == 1.f){
			for(int i = 0; i < 25; ++i) {
				texel = texture(textures.tex, Input.uv + offsets5x5[i]);
				if (texel.x < min.x)
					min.x = texel.x;
				if (texel.y < min.y)
					min.y = texel.y;
				if (texel.z < min.z)
					min.z = texel.z;
			}
	}
	else {
			for(int i = 0; i < 49; ++i) {
				texel = texture(textures.tex, Input.uv + offsets7x7[i]);
				if (texel.x < min.x)
					min.x = texel.x;
				if (texel.y < min.y)
					min.y = texel.y;
				if (texel.z < min.z)
					min.z = texel.z;
		}
>>>>>>> 16939e2833e74f96f45f45c91380cf0847af4d94
	}
    out_color = min;
}


layout (index = 6) subroutine (FragmentProgram) void gauss3x3()
{
	// für die Formel für H siehe https://de.wikipedia.org/wiki/Gau%C3%9F-Filter

	// Weil wir eine 3x3 Umgebung betrachten
	int m = 3;
	const float pi = 3.141592653589;
	
	// Varianz die in der GUI eingegeben werden kann, wieder standardmäßig auf 1 setzen und in der Rage 0.01 und 10 bewegen lassen
	float var = clamp((parameter.paramA.y) + 1, 0.01, 10);

	// Filterkern
	float H[9] = float[](	
							0.f, 0.f, 0.f,
							0.f, 0.f, 0.f, 
							0.f, 0.f, 0.f	);
	// Summe der Filterkerne; besseres Ergebnis als bei der Verwendung von m^2 bei der Teilung der Summe
	float sumH = 0;
	
	vec4 texel = vec4(0.f, 0.f, 0.f, 1.f);

	for(int i = 0; i < 9; ++i) {
		// Berechnung des Filterkerns anhand der Formel
		H[i] = exp(-((offsets3x3[i].x)*(offsets3x3[i].x) + (offsets3x3[i].y)*(offsets3x3[i].y)) / (2.f * var)) / (2.f * pi * var);

		// Gewichtung und Aufsummierung
		texel += H[i] * texture(textures.tex, Input.uv + offsets3x3[i]);
		sumH += H[i];
		}

    out_color = texel / sumH;
}


layout (index = 7) subroutine (FragmentProgram) void gauss5x5()
{
    // für die Formel für H siehe https://de.wikipedia.org/wiki/Gau%C3%9F-Filter

	// Weil wir eine 5x5 Umgebung betrachten
	int m = 5;
	const float pi = 3.141592653589;
	
	// Varianz die in der GUI eingegeben werden kann, wieder standardmäßig auf 1 setzen und in der Rage 0.01 und 10 bewegen lassen
	float var = clamp((parameter.paramB.y) + 1, 0.01, 10);

	// Filterkern
	float H[25] = float[](	
							0.f, 0.f, 0.f, 0.f, 0.f,
							0.f, 0.f, 0.f, 0.f, 0.f,
							0.f, 0.f, 0.f, 0.f, 0.f,
							0.f, 0.f, 0.f, 0.f, 0.f,
							0.f, 0.f, 0.f, 0.f, 0.f);
	// Summe der Filterkerne; besseres Ergebnis als bei der Verwendung von m^2 bei der Teilung der Summe
	float sumH = 0;
	
	vec4 texel = vec4(0.f, 0.f, 0.f, 1.f);

	for(int i = 0; i < 25; ++i) {
		// Berechnung des Filterkerns anhand der Formel
		H[i] = exp(-((offsets5x5[i].x)*(offsets5x5[i].x) + (offsets5x5[i].y)*(offsets5x5[i].y)) / (2.f * var)) / (2.f * pi * var);
		
		// Gewichtung und Aufsummierung
		texel += H[i] * texture(textures.tex, Input.uv + offsets5x5[i]);
		sumH += H[i];
		}

    out_color = texel / sumH;
}


layout (index = 8) subroutine (FragmentProgram) void gauss7x7()
{
    // für die Formel für H siehe https://de.wikipedia.org/wiki/Gau%C3%9F-Filter

	// Weil wir eine 7x7 Umgebung betrachten
	int m = 7;
	const float pi = 3.141592653589;
	
	// Varianz die in der GUI eingegeben werden kann, wieder standardmäßig auf 1 setzen und in der Rage 0.01 und 10 bewegen lassen
<<<<<<< HEAD
	float var = clamp((parameter.paramA.z) + 1, 0.01, 10);
=======
	float var = clamp((parameter.paramA.z) + 1, 0.01, 10000);
>>>>>>> 16939e2833e74f96f45f45c91380cf0847af4d94

	// Filterkern
	float H[49] = float[](	
							0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f,
							0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f,
							0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f,
							0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f,
							0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f,
							0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f,
							0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f);
	// Summe der Filterkerne; besseres Ergebnis als bei der Verwendung von m^2 bei der Teilung der Summe
	float sumH = 0;
	
	vec4 texel = vec4(0.f, 0.f, 0.f, 1.f);

	for(int i = 0; i < 49; ++i) {
		// Berechnung des Filterkerns anhand der Formel
		H[i] = exp(-((offsets7x7[i].x)*(offsets7x7[i].x) + (offsets7x7[i].y)*(offsets7x7[i].y)) / (2.f * var)) / (2.f * pi * var);
		
		// Gewichtung und Aufsummierung
		texel += H[i] * texture(textures.tex, Input.uv + offsets7x7[i]);
		sumH += H[i];
		}

    out_color = texel / sumH;
}


layout (index = 9) subroutine (FragmentProgram) void gauss7x7vertical()
{
    float H[7] = float[]( 1.f, 2.f, 3.f, 4.f, 3.f, 2.f, 1.f);

	vec2 offsets7x1[7] = vec2[]
	(
	vec2(-3, 0), 
	vec2(-2, 0), 
	vec2(-1, 0), 
	 vec2(0, 0), 
	 vec2(1, 0), 
	 vec2(2, 0), 
	 vec2(3, 0) 
	);

	vec4 texel = vec4(0);

 for(int i = 0; i < 7; ++i)
        texel += H[i] * texture(textures.tex, Input.uv + offsets7x1[i]);

    out_color = texel / 16.f;

}


layout (index = 10) subroutine (FragmentProgram) void gauss7x7horizontal()
{
     float H[7] = float[]( 1.f, 2.f, 3.f, 4.f, 3.f, 2.f, 1.f);

	vec2 offsets1x7[7] = vec2[]
		(vec2(0, -3), vec2(0, -2), vec2(0, -1), vec2(0, 0), vec2(0, 1), vec2(0, 2), vec2(0, 3));
	vec4 texel = vec4(0);

 for(int i = 0; i < 7; ++i)
        texel += H[i] * texture(textures.tex, Input.uv + offsets1x7[i]);

    out_color = texel / 16.f;

}


layout (index = 11) subroutine (FragmentProgram) void sobel()
{
	// Filterkerne f¨ur horizontale und vertikale Kantenextraktion
	// 		-1 -2 -1 			1 0 -1
	// Hx =	 0  0  0 	Hy =	2 0 -2
	// 		 1  2  1 			1 0 -1
	
	//Schwellwert: Pixel der Sobelbetrag angezeigt werden, für die der Sobelbetrag größer oder gleich dem Schwellwert is
	float threshold = (parameter.paramB.z) * 0.1;

	vec4 texel[9];
	float outputSW;
	
	// Schreiben der 3x3 Umgebung in ein Array
	for(int i = 0; i < 9; ++i)
        	texel[i] = texture(textures.tex, Input.uv + offsets3x3[i]);

	// Aufsummierung und Gewichtung anhand der Filterkerne	
	vec4 Horiz = texel[2] + 2*texel[5] + texel[8] - (texel[0] + 2*texel[3] + texel[6]);
	vec4 Vert = texel[0] + 2*texel[1] + texel[2] - (texel[6] + 2*texel[7] + texel[8]);
	
	// Schwarz-Weiß Output --> am relevantesten, da Grauwertänderungen besonders interessant bei sobel
	outputSW = dot(sqrt(Horiz * Horiz + Vert * Vert).xyz, vec3(.299f, .587f, .114f));

	// Nur ab einem bewissen Schwellwert sollen "Steigungen" zwischen den Grauwerten angezeigt werden, sonst schwarz = 0
	if(outputSW >= threshold) 
   		out_color = vec4(outputSW); 
	else 
<<<<<<< HEAD
		 out_color = vec4(0.f, 0.f, 0.f, 1.f);
=======
		out_color = vec4(0.f, 0.f, 0.f, 1.f);
>>>>>>> 16939e2833e74f96f45f45c91380cf0847af4d94

	//--------------------------------------------
	/*  bunt
	vec4 sobelOutput = sqrt(Horiz * Horiz + Vert * Vert);

	if(((sobelOutput[0] + sobelOutput[1] + sobelOutput[2]) / 3) >= threshold) 
   		out_color = sobelOutput; 
	else 
<<<<<<< HEAD
		out_color = vec4(0.f, 0.f, 0.f, 1.f); */
		//-----------------------------------------
=======
		out_color = vec4(0.f, 0.f, 0.f, 1.f);
		-----------------------------------------*/
>>>>>>> 16939e2833e74f96f45f45c91380cf0847af4d94
}


layout (index = 12) subroutine (FragmentProgram) void laplace()
{
	vec4 texel;
	float brightness = parameter.paramA.x * 0.1;
	float contrast = clamp((parameter.paramB.x * 0.1) + 1.0, 0.01, 20);

	// Laplace-Filtermaske --> Gewichtung der umliegenden Pixel
<<<<<<< HEAD
	float H[9] = float[](	 0.f, -1.f,  0.f, 
							-1.f,  4.f, -1.f,
							 0.f, -1.f,  0.f);
=======
	float H[9] = float[](	-1.f, -1.f, -1.f, 
							-1.f,  8.f, -1.f,
							-1.f, -1.f, -1.f);

							/* 1.f, -2.f,  1.f, 
							-2.f,  4.f, -2.f,
							 1.f, -2.f,  1.f);*/
							 
							 /*0.f, -1.f,  0.f, 
							-1.f,  4.f, -1.f,
							 0.f, -1.f,  0.f);*/
>>>>>>> 16939e2833e74f96f45f45c91380cf0847af4d94
	
	// Gewichtung der umliegenden Pixel und Aufsummierung 
 	for(int i = 0; i < 9; ++i)
        	texel += H[i] * texture(textures.tex, Input.uv + offsets3x3[i]);

	/* 
	* Da die obere Summe auf Grund der Gewichtungen auch negativ sein kann, muss der Wertebereich von [-1;1] wieder nach [0;1] verschoben werden.
	* --> Wertebereich halbieren und um 0,5 verschieben;
	* Anpassung der Helligkeit und des Contrastes anhand der Formel für Brightness-Contrast
	*/
	out_color = ((texel / 2  + 0.5) + brightness) * contrast + 0.5 * (1 - contrast);
}

/*
* Bubble Sort Algorithmus
*/
float[9] mysort(float[9] array)
{
   int i, j;
   float tmp;

   for (i = 0; i < 9 ; i++) 
   {
      for (j = i; j < 9 ; j++) 
      {
          if (array[i] > array[j]) 
          {
              tmp = array[i];
              array[i] = array[j];
              array[j] = tmp;
          }
      }
   }
	return array; 
}

layout (index = 13) subroutine (FragmentProgram) void median()
{
    float texel[9];

	// Schreiben der Grauwerte in ein Array, die mittels Skalarproduk berechnert werden 
	//(hierbei ungleichmäßige Verteilung, der rgb Werte, da von menschlichem Auge unterschiedlich intensiv wahrgenommen)
    for(int i = 0; i < 9; ++i) {
        texel[i] = dot(texture(textures.tex, Input.uv + offsets3x3[i]).xyz, vec3(.299f, .587f, .114f));
	}

	// Sortieren des Arrays
	float sorted[9] = mysort(texel);

	// Median liegt genau in der Mitte des sortierten Arrays
    out_color = vec4(sorted[4]);
}
// =============================================================================================================
