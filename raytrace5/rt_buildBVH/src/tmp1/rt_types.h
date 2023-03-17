#pragma once

typedef unsigned int uint;
typedef unsigned int uint32;
typedef long long int64;
typedef unsigned long long uint64;

struct rtBuffer
{
	uint64 StartAddress;
	uint32 Size;
	uint32 Stride;
};

struct rtGeometryDesc
{
	uint IndexCount;
	uint VertexCount;
	rtBuffer IndexBuffer;
	rtBuffer VertexBuffer;
};

struct rtAccelerationStructureBuildInputs
{
	uint NumDescs;
	rtGeometryDesc* pGeometryDescs;
};

struct rtAccelerationStructurePrebuildInfo
{
	uint32 ResultDataSize;
	uint32 ScratchDataSize;
};

struct rtAccelerationStructureDesc
{
	rtAccelerationStructureBuildInputs Inputs;
	rtBuffer ResultBuffer;
	rtBuffer ScratchBuffer;
};
