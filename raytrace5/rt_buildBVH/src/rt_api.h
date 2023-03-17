#pragma once

#include "rt_types.h"

extern "C" rtBuffer rtCreateBuffer(uint Size, uint Stride = 0);
extern "C" void rtReleaseBuffer(rtBuffer Buffer);
extern "C" void rtCopyToBuffer(rtBuffer Buffer, void* ptr, uint Size);
extern "C" void rtCopyFromBuffer(rtBuffer Buffer, void* ptr, uint Size);

extern "C" 
void rtGetAccelerationStructurePrebuildInfo(
	rtAccelerationStructureBuildInputs* pInputs,
	rtAccelerationStructurePrebuildInfo* pInfo
);

extern "C"
void rtBuildAccelerationStructure(rtAccelerationStructureDesc Desc);
