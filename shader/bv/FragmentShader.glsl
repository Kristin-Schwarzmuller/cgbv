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
    out_color = texture(textures.tex, Input.uv);
}


layout (index = 4) subroutine (FragmentProgram) void dilatation()
{
    vec4 texel = vec4(0.f, 0.f, 0.f, 1.f);
	vec4 max = vec4(0.f, 0.f, 0.f, 1.f);

    //   1  1  1
    //   1  1  1
    //   1  1  1

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

    //   1  1  1
    //   1  1  1
    //   1  1  1

    for(int i = 0; i < 9; ++i) {
        texel = texture(textures.tex, Input.uv + offsets3x3[i]);
		if (texel.x < min.x)
			min.x = texel.x;
		if (texel.y < min.y)
			min.y = texel.y;
		if (texel.z < min.z)
			min.z = texel.z;
	}
    out_color = min;
}


layout (index = 6) subroutine (FragmentProgram) void gauss3x3()
{
	int m = 3;
	int k = (m - 1) / 2;
	vec4 texel = vec4(0.f, 0.f, 0.f, 1.f);
	vec2 offset = vec2(0.f, 0.f);
	int H [9] = int[](
				1, 2, 1,
				2, 4, 2,
				1, 2, 1		);

	for(int i = 0; i < m; ++i) {
		for(int j = 0; j < m; ++j) {
			offset.x = k - i;
			offset.y = k - j;
			texel += H[m * i + j] * texture(textures.tex, Input.uv + offset);
		}
	}

    out_color = 9 * texel / (m * m * 16);
}


layout (index = 7) subroutine (FragmentProgram) void gauss5x5()
{
    out_color = texture(textures.tex, Input.uv);
}


layout (index = 8) subroutine (FragmentProgram) void gauss7x7()
{
    out_color = texture(textures.tex, Input.uv);
}


layout (index = 9) subroutine (FragmentProgram) void gauss7x7vertical()
{
    out_color = texture(textures.tex, Input.uv);
}


layout (index = 10) subroutine (FragmentProgram) void gauss7x7horizontal()
{
    out_color = texture(textures.tex, Input.uv);
}


layout (index = 11) subroutine (FragmentProgram) void sobel()
{
    out_color = texture(textures.tex, Input.uv);
}


layout (index = 12) subroutine (FragmentProgram) void laplace()
{
	out_color = texture(textures.tex, Input.uv);
}

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

    for(int i = 0; i < 9; ++i) {
        texel[i] = dot(texture(textures.tex, Input.uv + offsets3x3[i]).xyz, vec3(.299f, .587f, .114f));
	}

	float sorted[9] = mysort(texel);

    out_color = vec4(sorted[4]);
}
// =============================================================================================================