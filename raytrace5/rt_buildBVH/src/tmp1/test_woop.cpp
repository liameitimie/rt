#include <iostream>
#include "myMath.h"
//#define F32 float

using namespace std;
/*
struct Vec3f{
	float x,y,z;
};

Vec3f operator+(Vec3f a,Vec3f b){return {a.x+b.x,a.y+b.y,a.z+b.z};}
Vec3f operator-(Vec3f a,Vec3f b){return {a.x-b.x,a.y-b.y,a.z-b.z};}
Vec3f operator*(Vec3f a,float b){return {a.x*b,a.y*b,a.z*b};}

Vec3f cross(Vec3f a,Vec3f b){return {a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x};}
float dot(Vec3f a,Vec3f b){return a.x*b.x+a.y*b.y+a.z*b.z;}

struct Vec4f{
	float x,y,z,w;
	Vec4f(){}
	Vec4f(Vec3f v,float _w){x=v.x,y=v.y,z=v.z,w=_w;}
};
class Mat4f 
{
public:
	
	inline    const F32*          getPtr(void) const            { return &m00; }
	inline    F32*                getPtr(void)                  { return &m00; }
	inline    const Vec4f&        col(int c) const				{ return *(const Vec4f*)(getPtr() + c * 4); }
	inline    Vec4f&	          col(int c)					{ return *(Vec4f*)(getPtr() + c * 4); }
	inline    const Vec4f&        getCol(int c) const		    { return col(c); }
	inline    Vec4f			      getCol0() const		    { Vec4f col; col.x = m00; col.y = m01; col.z = m02; col.w = m03; return col; }
	inline    Vec4f			      getCol1() const		    { Vec4f col; col.x = m10; col.y = m11; col.z = m12; col.w = m13; return col; }
	inline    Vec4f			      getCol2() const		    { Vec4f col; col.x = m20; col.y = m21; col.z = m22; col.w = m23; return col; }
	inline    Vec4f			      getCol3() const		    { Vec4f col; col.x = m30; col.y = m31; col.z = m32; col.w = m33; return col; }
	inline    Vec4f               getRow(int r) const;
	inline    Mat4f               inverted4x4(void);
	inline    void                invert(void)                  { set(inverted4x4()); }
	inline    const float&        get(int idx) const             { return getPtr()[idx]; }
	inline    float&              get(int idx)                   { return getPtr()[idx]; }
	inline    const float&        get(int r, int c) const        { return getPtr()[r + c * 4]; }
	inline    float&              get(int r, int c)              { return getPtr()[r + c * 4]; }
	inline    void                set(const float& a)            { for (int i = 0; i < 4 * 4; i++) get(i) = a; }
	inline    void                set(const float* ptr)          { for (int i = 0; i < 4 * 4; i++) get(i) = ptr[i]; }
	inline    void                setZero(void)                  { set((float)0); }
	inline    void                setIdentity(void)              { setZero(); for (int i = 0; i < 4; i++) get(i, i) = (float)1; }
	inline    void				  setCol(int c, const Vec4f& v)   { col(c) = v; }
	inline    void				  setCol0(const Vec4f& v)   { m00 = v.x; m01 = v.y; m02 = v.z; m03 = v.w; }
	inline    void				  setCol1(const Vec4f& v)   { m10 = v.x; m11 = v.y; m12 = v.z; m13 = v.w; }
	inline    void				  setCol2(const Vec4f& v)   { m20 = v.x; m21 = v.y; m22 = v.z; m23 = v.w; }
	inline    void				  setCol3(const Vec4f& v)   { m30 = v.x; m31 = v.y; m32 = v.z; m33 = v.w; }
	inline    void                setRow(int r, const Vec4f& v);
	inline    void                set(const Mat4f& v) { set(v.getPtr()); }
	inline    Mat4f&              operator=   (const float& a)                { set(a); return *(Mat4f*)this; }
	inline    Mat4f               operator*   (const float& a) const          { Mat4f r; for (int i = 0; i < 4 * 4; i++) r.get(i) = get(i) * a; return r; }
	inline    const float&        operator()  (int r, int c) const        { return get(r, c); }
	inline    float&              operator()  (int r, int c)              { return get(r, c); }

	inline                    Mat4f(void)                      { setIdentity(); }
	inline    explicit        Mat4f(F32 a)                     { set(a); }
	static inline Mat4f       fromPtr(const F32* ptr)            { Mat4f v; v.set(ptr); return v; }

	inline Mat4f(const Mat4f& v) { set(v); }
	inline Mat4f& operator=(const Mat4f& v) { set(v); return *this; }

public:
	F32             m00, m10, m20, m30;
	F32             m01, m11, m21, m31;
	F32             m02, m12, m22, m32;
	F32             m03, m13, m23, m33;
};

Vec4f Mat4f::getRow(int r) const
{
	Vec4f v;
	v.x = get(r, 0);
	v.y = get(r, 1);
	v.z = get(r, 2);
	v.w = get(r, 3);
	return v;
}

Mat4f Mat4f::inverted4x4(void)
{
	float inv[16];
	float m[16] = {	m00, m10, m20, m30,
					m01, m11, m21, m31,
					m02, m12, m22, m32,
					m03, m13, m23, m33 };

	inv[0]  =  m[5] * m[10] * m[15] - m[5] * m[11] * m[14] - m[9] * m[6] * m[15] + m[9] * m[7] * m[14] + m[13] * m[6] * m[11] - m[13] * m[7] * m[10];
	inv[4]  = -m[4] * m[10] * m[15] + m[4] * m[11] * m[14] + m[8] * m[6] * m[15] - m[8] * m[7] * m[14] - m[12] * m[6] * m[11] + m[12] * m[7] * m[10];
	inv[8]  =  m[4] * m[9] *  m[15] - m[4] * m[11] * m[13] - m[8] * m[5] * m[15] + m[8] * m[7] * m[13] + m[12] * m[5] * m[11] - m[12] * m[7] * m[9];
	inv[12] = -m[4] * m[9] *  m[14] + m[4] * m[10] * m[13] + m[8] * m[5] * m[14] - m[8] * m[6] * m[13] - m[12] * m[5] * m[10] + m[12] * m[6] * m[9];
	inv[1]  = -m[1] * m[10] * m[15] + m[1] * m[11] * m[14] + m[9] * m[2] * m[15] - m[9] * m[3] * m[14] - m[13] * m[2] * m[11] + m[13] * m[3] * m[10];
	inv[5]  =  m[0] * m[10] * m[15] - m[0] * m[11] * m[14] - m[8] * m[2] * m[15] + m[8] * m[3] * m[14] + m[12] * m[2] * m[11] - m[12] * m[3] * m[10];
	inv[9]  = -m[0] * m[9] * m[15] + m[0] * m[11] * m[13] + m[8] * m[1] * m[15] - m[8] * m[3] * m[13] - m[12] * m[1] * m[11] + m[12] * m[3] * m[9];
	inv[13] =  m[0] * m[9] * m[14] - m[0] * m[10] * m[13] - m[8] * m[1] * m[14] + m[8] * m[2] * m[13] + m[12] * m[1] * m[10] - m[12] * m[2] * m[9];
	inv[2]  =  m[1] * m[6] * m[15] - m[1] * m[7] * m[14] - m[5] * m[2] * m[15] + m[5] * m[3] * m[14] + m[13] * m[2] * m[7] - m[13] * m[3] * m[6];
	inv[6]  = -m[0] * m[6] * m[15] + m[0] * m[7] * m[14] + m[4] * m[2] * m[15] - m[4] * m[3] * m[14] - m[12] * m[2] * m[7] + m[12] * m[3] * m[6];
	inv[10] =  m[0] * m[5] * m[15] - m[0] * m[7] * m[13] - m[4] * m[1] * m[15] + m[4] * m[3] * m[13] + m[12] * m[1] * m[7] - m[12] * m[3] * m[5];
	inv[14] = -m[0] * m[5] * m[14] + m[0] * m[6] * m[13] + m[4] * m[1] * m[14] - m[4] * m[2] * m[13] - m[12] * m[1] * m[6] + m[12] * m[2] * m[5];
	inv[3]  = -m[1] * m[6] * m[11] + m[1] * m[7] * m[10] + m[5] * m[2] * m[11] - m[5] * m[3] * m[10] - m[9] * m[2] * m[7] + m[9] * m[3] * m[6];
	inv[7]  =  m[0] * m[6] * m[11] - m[0] * m[7] * m[10] - m[4] * m[2] * m[11] + m[4] * m[3] * m[10] + m[8] * m[2] * m[7] - m[8] * m[3] * m[6];
	inv[11] = -m[0] * m[5] * m[11] + m[0] * m[7] * m[9] + m[4] * m[1] * m[11] - m[4] * m[3] * m[9] - m[8] * m[1] * m[7] + m[8] * m[3] * m[5];
	inv[15] =  m[0] * m[5] * m[10] - m[0] * m[6] * m[9] - m[4] * m[1] * m[10] + m[4] * m[2] * m[9] + m[8] * m[1] * m[6] - m[8] * m[2] * m[5];

	float det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

	if (det == 0)
		return Mat4f();

	det = 1.f / det;
	Mat4f inverse;
	for (int i = 0; i < 16; i++)
		inverse.get(i) = inv[i] * det;

	return inverse;
}
*/
int main(){
	
	/*Vec3f o{0,0,0},d{1,1.1,1};
	Vec3f V0{3,4,1},V1{7,9,6},V2{4,2,8};
	
	Mat4f mtx;
	mtx.setCol(0,Vec4f(V0-V2,0));
	mtx.setCol(1,Vec4f(V1-V2,0));
	mtx.setCol(2,Vec4f(cross(V0-V2,V1-V2),0));
	mtx.setCol(3,Vec4f(V2,1));
	
	for(int i=0;i<4;i++){
		for(int j=0;j<4;j++){
			cout<<mtx.get(i,j)<<' ';
		}
		cout<<endl;
	}
	cout<<endl;
	
	mtx.invert();
	
	for(int i=0;i<4;i++){
		for(int j=0;j<4;j++){
			cout<<mtx.get(i,j)<<' ';
		}
		cout<<endl;
	}
	
	Vec4f v0=mtx.getRow(2);
	Vec4f v1=mtx.getRow(0);
	Vec4f v2=mtx.getRow(1);
	
	//woop 
	{
		float oz=v0.w+o.x*v0.x+o.y*v0.y+o.z*v0.z;
		float dz=d.x*v0.x+d.y*v0.y+d.z*v0.z;
		float t=-oz/dz;
		
		float ox=v1.w+o.x*v1.x+o.y*v1.y+o.z*v1.z;
		float dx=d.x*v1.x+d.y*v1.y+d.z*v1.z;
		float u=ox+t*dx;
		
		float oy=v2.w+o.x*v2.x+o.y*v2.y+o.z*v2.z;
		float dy=d.x*v2.x+d.y*v2.y+d.z*v2.z;
		float v=oy+t*dy;
		
		cout<<t<<' '<<u<<' '<<v<<' '<<1-u-v<<endl;
		Vec3f vv=V0*u+V1*v+V2*(1-u-v);
		cout<<"v: "<<vv.x<<' '<<vv.y<<' '<<vv.z<<endl<<endl;
	}
	
	{
		Vec3f e1=V1-V0;
		Vec3f e2=V2-V0;
		Vec3f s=o-V0;
		Vec3f s1=cross(d,e2);
		Vec3f s2=cross(s,e1);
		float D=dot(s1,e1);
		//if(abs(d)<1e-6) return 0;
		D=1/D;
		float t=dot(s2,e2)*D;
		float b1=dot(s1,s)*D;
		float b2=dot(s2,d)*D;
		//if(t>=tmin&&t<tmax&&b1>=0&&b2>=0&&b1+b2<=1) return t;
		
		cout<<t<<' '<<1-b1-b2<<' '<<b1<<' '<<b2<<endl;
		Vec3f vv=V0*(1-b1-b2)+V1*b1+V2*b2;
		cout<<"v: "<<vv.x<<' '<<vv.y<<' '<<vv.z<<endl;
	}
	*/
	
	float3 o{0,0,0},d{1,1.1,1};
	float3 V0{3,4,1},V1{7,9,6},V2{4,2,8};
	
	float4x4 mtx;
	mtx.setCol(0,make_float4(V0-V2,0));
	mtx.setCol(1,make_float4(V1-V2,0));
	mtx.setCol(2,make_float4(cross(V0-V2,V1-V2),0));
	mtx.setCol(3,make_float4(V2,1));
	
	for(int i=0;i<4;i++){
		for(int j=0;j<4;j++){
			cout<<mtx.get(i,j)<<' ';
		}
		cout<<endl;
	}
	cout<<endl;
	
	mtx.invert();
	
	for(int i=0;i<4;i++){
		for(int j=0;j<4;j++){
			cout<<mtx.get(i,j)<<' ';
		}
		cout<<endl;
	}
	
	float4 v0=mtx.getRow(2);
	float4 v1=mtx.getRow(0);
	float4 v2=mtx.getRow(1);
	
	//woop 
	{
		float oz=v0.w+o.x*v0.x+o.y*v0.y+o.z*v0.z;
		float dz=d.x*v0.x+d.y*v0.y+d.z*v0.z;
		float t=-oz/dz;
		
		float ox=v1.w+o.x*v1.x+o.y*v1.y+o.z*v1.z;
		float dx=d.x*v1.x+d.y*v1.y+d.z*v1.z;
		float u=ox+t*dx;
		
		float oy=v2.w+o.x*v2.x+o.y*v2.y+o.z*v2.z;
		float dy=d.x*v2.x+d.y*v2.y+d.z*v2.z;
		float v=oy+t*dy;
		
		cout<<t<<' '<<u<<' '<<v<<' '<<1-u-v<<endl;
		float3 vv=V0*u+V1*v+V2*(1-u-v);
		cout<<"v: "<<vv.x<<' '<<vv.y<<' '<<vv.z<<endl<<endl;
	}
	
	{
		float3 e1=V1-V0;
		float3 e2=V2-V0;
		float3 s=o-V0;
		float3 s1=cross(d,e2);
		float3 s2=cross(s,e1);
		float D=dot(s1,e1);
		//if(abs(d)<1e-6) return 0;
		D=1/D;
		float t=dot(s2,e2)*D;
		float b1=dot(s1,s)*D;
		float b2=dot(s2,d)*D;
		//if(t>=tmin&&t<tmax&&b1>=0&&b2>=0&&b1+b2<=1) return t;
		
		cout<<t<<' '<<1-b1-b2<<' '<<b1<<' '<<b2<<endl;
		float3 vv=V0*(1-b1-b2)+V1*b1+V2*b2;
		cout<<"v: "<<vv.x<<' '<<vv.y<<' '<<vv.z<<endl;
	}
	
	return 0;
} 
