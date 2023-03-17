#pragma once

struct rtResource {
	void* cuPtr;
};

void rtCreateResource();
void rtCopyToResource();
void rtCopyFromResource();