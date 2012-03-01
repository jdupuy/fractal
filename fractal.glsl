#version 330

uniform float uIterationCnt; // number of iterations
uniform float uScale;      // scene scale
uniform float uExponent;     // factal exponent
uniform vec2 uOffset;      // scene offset;

// functions
vec2 conjugate(vec2 z) {
	return vec2(z.x,-z.y);
}

vec2 powc(vec2 z, float p) {
	float zmod = length(z);
	float zarg = atan(z.y,z.x);
	return pow(zmod,p)*vec2(cos(p*zarg), sin(p*zarg));
}


#ifdef _VERTEX_
layout(location=0) in vec4 iPosition;

out vec2 varPosition;
#define oPosition varPosition

void main() {
	gl_Position = iPosition;
	oPosition   = iPosition.xy*uScale+uOffset;
}
#endif // _VERTEX_


#ifdef _FRAGMENT_
in vec2 varPosition;
#define iPosition varPosition

layout(location=0) out vec4 oColour;

void main() {
	oColour  = vec4(0);
	vec2 z   = iPosition;
	float i = 0;

	for (; i < uIterationCnt && dot(z,z) < 4.0; ++i) {
#if defined _MANDELBROT // mandelbrot set code
		z = powc(z,uExponent) + iPosition;
#elif defined _TRICORN // tricorn set code
		z = powc(conjugate(z),uExponent) + iPosition;
#elif defined _BURNING_SHIP // burning ship set code
		z = powc(vec2(abs(z.x), -abs(z.y)),uExponent) + iPosition;
#endif
	}

	oColour = vec4(i/uIterationCnt);

}
#endif // _FRAGMENT_

