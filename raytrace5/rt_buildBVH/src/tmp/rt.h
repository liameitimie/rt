#pragma once
#include "types.h"

struct rtBuffer
{
	uint64 StartAddress;
	uint32 Size;
	uint32 Stride;
};

extern "C" rtBuffer rtCreateBuffer(uint Size, uint Stride = 0);
extern "C" void rtReleaseBuffer(rtBuffer Buffer);
extern "C" void rtCopyToBuffer(rtBuffer Buffer, void* ptr, uint Size);
extern "C" void rtCopyFromBuffer(rtBuffer Buffer, void* ptr, uint Size);

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

extern "C" 
void rtGetAccelerationStructurePrebuildInfo(
	rtAccelerationStructureBuildInputs* pInputs,
	rtAccelerationStructurePrebuildInfo* pInfo
);

extern "C"
void rtBuildAccelerationStructure(rtAccelerationStructureDesc Desc);
