#include "rt_api.h"

float v_data[] = {
	0,0,0,
	1,0,0,
	0,1,0,
	2,0,1,
	1,2,3,
	-1,0,2
};

int i_data[] = {
	0,1,2,
	1,2,3,
	0,2,3,
	1,3,4,
	2,4,5,
	3,4,5
};

int main(){
	rtBuffer vbuffer = rtCreateBuffer(sizeof(v_data));
	rtBuffer ibuffer = rtCreateBuffer(sizeof(i_data));

	rtCopyToBuffer(vbuffer, v_data, sizeof(v_data));
	rtCopyToBuffer(ibuffer, i_data, sizeof(i_data));

	rtGeometryDesc g_desc;
	g_desc.VertexBuffer = vbuffer;
	g_desc.IndexBuffer = ibuffer;
	g_desc.VertexCount = 6;
	g_desc.IndexCount = 18;

	rtAccelerationStructureBuildInputs input;
	input.NumDescs = 1;
	input.pGeometryDescs = &g_desc;

	rtAccelerationStructureDesc as_desc;
	as_desc.Inputs = input;

	rtBuildAccelerationStructure(as_desc);


	return 0;
}