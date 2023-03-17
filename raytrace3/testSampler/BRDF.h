#pragma once

#include <cuda_runtime.h>
//#include "myMath.h"

#define PI 3.14159265358979323846f

__device__ __host__
inline float Pow5(float x){
	float x2=x*x;
	return x2*x2*x;
	//return x*x*x*x*x;
}

__device__ __host__
inline float3 Diffuse_Lambert(float3 baseColor){
	return baseColor*(1/PI);
}

// [Burley 2012, "Physically-Based Shading at Disney"]
__device__ __host__
inline float3 Diffuse_Burley(float3 baseColor,float roughness,float NoV,float NoL,float VoH)
{
	float FD90 = 0.5 + 2 * VoH * VoH * roughness;
	float FdV = 1 + (FD90 - 1) * Pow5( 1 - NoV );
	float FdL = 1 + (FD90 - 1) * Pow5( 1 - NoL );
	return baseColor * ( (1 / PI) * FdV * FdL );
}

__device__ __host__
inline float3 Diffuse_GGXApprox(float3 albedo,float a,float NoL,float NoV,float NoH,float VoH){
	float facing=VoH*VoH;//0.5+0.5*LoV=1/4*len(L+V)=VoH*VoH
	float rough=facing*(0.9-0.4*facing)/*(0.5+NoH)/NoH*/;
	float smooth=1.05*(1-Pow5(1-NoL))*(1-Pow5(1-NoV));
	float single=1/PI*lerp(smooth,rough,a);
	float multi=0.1159*a;
	float3 diffuse=albedo*(single+albedo*multi);
	return diffuse;
}

__device__ __host__
inline float3 F_Schlick(float3 F0,float VoH){
	return F0+(1-F0)*Pow5(1-VoH);
}

__device__ __host__
inline float D_GGX(float a,float NoH){
	float a2=a*a;
	float d=NoH*NoH*(a2-1)+1;
	return a2/(PI*d*d);
}

//__device__ __host__
//inline float smithG_GGX(float a,float NoV){
//    float a2=a*a;
//    float b=NoV*NoV;
//    return 1/(NoV+sqrt(a2+b-a2*b));
//}
//__device__ __host__
//inline float G_SchlicksmithGGX(float a,float NoL,float NoV){
//	return smithG_GGX(a,NoL)*smithG_GGX(a,NoV);
//}

__device__ __host__
inline float G_SchlicksmithGGX(float a,float NoL,float NoV){
	float k=(a+1)*(a+1)/8;
	float GL=NoL/(NoL*(1-k)+k);
	float GV=NoV/(NoV*(1-k)+k);
	return GL*GV;
}

__device__ __host__
inline float V_SmithGGXCorrelated(float a,float NoL,float NoV){
	float a2=a*a;
	float GL=NoV*sqrt(a2+(1-a2)*NoL*NoL);
	float GV=NoL*sqrt(a2+(1-a2)*NoV*NoV);
	return 0.5/(GL+GV);
}

__device__ __host__
inline float V_SmithGGXCorrelatedApprox(float a,float NoL,float NoV){
	return 0.5/lerp(2*NoL*NoV,NoL+NoV,a);
}

__device__ __host__
float3 BRDF(float3 V,
			float3 L,
			float3 N,
			float3 baseColor,
			float metallic,
			float roughness)
{
	float3 H=normalize(V+L);
//	printf("l:(%f,%f,%f) v:(%f,%f,%f) n:(%f,%f,%f) h:(%f,%f,%f)\n",
//	L.x,L.y,L.z,V.x,V.y,V.z,N.x,N.y,N.z,H.x,H.y,H.z);
	float NoV=clamp(dot(N,V),0.0f,1.0f);
	float NoL=clamp(dot(N,L),0.0f,1.0f);
	float NoH=clamp(dot(N,H),0.0f,1.0f);
	float VoH=clamp(dot(V,H),0.0f,1.0f);
	
	float a=fmaxf(0.001f,roughness*roughness);
	float3 F0=lerp(make_float3(0.04),baseColor,metallic);
	
	float D=D_GGX(a,NoH);
	float G=V_SmithGGXCorrelatedApprox(a,NoL,NoV);
	float3 F=F_Schlick(F0,/*NoV*/VoH);
	
//	printf("nov:%f nol:%f noh:%f voh:%f\n",NoV,NoL,NoH,VoH);
//	printf("a:%f F0:(%f,%f,%f) D:%f G:%f F:(%f,%f,%f) kd:(%f,%f,%f)\n",
//	a,F0.x,F0.y,F0.z,D,G,F.x,F.y,F.z,kd.x,kd.y,kd.z);
	
	float3 albedo=baseColor*(1-metallic);
	float3 diffuse=Diffuse_GGXApprox(albedo,a,NoL,NoV,NoH,VoH);
	
	return (1-F)*diffuse + /*(1-kd)*/D*F*G/*/(4*NoL*NoV)*/;
}



//__device__ __host__
//float sqr(float x) { return x*x; }
//
//__device__ __host__
//float SchlickFresnel(float u)
//{
//    float m = clamp(1-u, 0.0f, 1.0f);
//    float m2 = m*m;
//    return m2*m2*m; // pow(m,5)
//}
//
//__device__ __host__
//float GTR1(float NdotH, float a)
//{
//    if (a >= 1) return 1/PI;
//    float a2 = a*a;
//    float t = 1 + (a2-1)*NdotH*NdotH;
//    return (a2-1) / (PI*log(a2)*t);
//}
//
//__device__ __host__
//float GTR2(float NdotH, float a)
//{
//    float a2 = a*a;
//    float t = 1 + (a2-1)*NdotH*NdotH;
//    return a2 / (PI * t*t);
//}
//
//__device__ __host__
//float GTR2_aniso(float NdotH, float HdotX, float HdotY, float ax, float ay)
//{
//    return 1 / (PI * ax*ay * sqr( sqr(HdotX/ax) + sqr(HdotY/ay) + NdotH*NdotH ));
//}
//
//__device__ __host__
//float smithG_GGX(float NdotV, float alphaG)
//{
//    float a = alphaG*alphaG;
//    float b = NdotV*NdotV;
//    return 1 / (NdotV + sqrt(a + b - a*b));
//}
//
//__device__ __host__
//float smithG_GGX_aniso(float NdotV, float VdotX, float VdotY, float ax, float ay)
//{
//    return 1 / (NdotV + sqrt( sqr(VdotX*ax) + sqr(VdotY*ay) + sqr(NdotV) ));
//}
//
////float3 mon2lin(float3 x)
////{
////    return float3(pow(x[0], 2.2), pow(x[1], 2.2), pow(x[2], 2.2));
////}
//
//
//__device__ __host__
//float3 BRDF( float3 L,float3 V,float3 N,float3 baseColor,float metallic,float roughness )
//{
////	color baseColor .82 .67 .16
////	float subsurface 0 1 0
//	float specular = 1;
////	float specularTint 0 1 0
////	float anisotropic 0 1 0
////	float sheen 0 1 0
////	float sheenTint 0 1 .5
////	float clearcoat 0 1 0
////	float clearcoatGloss 0 1 1
//	
//    float NdotL = dot(N,L);
//    float NdotV = dot(N,V);
//    if (NdotL < 0 || NdotV < 0) return make_float3(0);
//
//    float3 H = normalize(L+V);
//    float NdotH = dot(N,H);
//    float LdotH = dot(L,H);
//
//    float3 Cdlin = baseColor;
////    float Cdlum = .3*Cdlin.x + .6*Cdlin.y  + .1*Cdlin.z; // luminance approx.
//
////    vec3 Ctint = Cdlum > 0 ? Cdlin/Cdlum : vec3(1); // normalize lum. to isolate hue+sat
//    float3 Cspec0 = lerp(specular*.08*make_float3(1), Cdlin, metallic);
////    vec3 Csheen = mix(vec3(1), Ctint, sheenTint);
//
//    // Diffuse fresnel - go from 1 at normal incidence to .5 at grazing
//    // and mix in diffuse retro-reflection based on roughness
//    float FL = SchlickFresnel(NdotL), FV = SchlickFresnel(NdotV);
//    float Fd90 = 0.5 + 2 * LdotH*LdotH * roughness;
//    float Fd = lerp(1.0, Fd90, FL) * lerp(1.0, Fd90, FV);
//
//    // Based on Hanrahan-Krueger brdf approximation of isotropic bssrdf
//    // 1.25 scale is used to (roughly) preserve albedo
//    // Fss90 used to "flatten" retroreflection based on roughness
////    float Fss90 = LdotH*LdotH*roughness;
////    float Fss = mix(1.0, Fss90, FL) * mix(1.0, Fss90, FV);
////    float ss = 1.25 * (Fss * (1 / (NdotL + NdotV) - .5) + .5);
//
//    // specular
//    float a  = max(.001, sqr(roughness));
//    float Ds = GTR2(NdotH,a);
//    float FH = SchlickFresnel(LdotH);
//    float3 Fs = lerp(Cspec0, make_float3(1), FH);
//    float Gs;
//    Gs  = smithG_GGX(NdotL,a);
//    Gs *= smithG_GGX(NdotV,a);
//
////    // sheen
////    vec3 Fsheen = FH * sheen * Csheen;
//
//    // clearcoat (ior = 1.5 -> F0 = 0.04)
////    float Dr = GTR1(NdotH, mix(.1,.001,clearcoatGloss));
////    float Fr = mix(.04, 1.0, FH);
////    float Gr = smithG_GGX(NdotL, .25) * smithG_GGX(NdotV, .25);
//
//    return ((1/PI) * Fd*Cdlin)
//        * (1-metallic)
//        + Gs*Fs*Ds /*+ .25*clearcoat*Gr*Fr*Dr*/;
//}

