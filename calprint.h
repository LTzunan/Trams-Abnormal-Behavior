#pragma once
#ifndef __CALPRINT_H__
#define __CALPRINT_H__

#include <stdio.h>
#include <opencv.hpp>

using namespace cv;
using namespace std;

struct bboxlist
{
	int x;
	int y;
	int width;
	int height;

	struct bboxlist *next;
};

struct tracks
{
	int label;
	int age;
	int coorx;
	int coory;

	int combineflag;
	int divflag;

	std::string imgPrint;

	struct bboxlist *bbox;
	struct tracks *next;
};

extern tracks *curr_head;
extern tracks *prev_head;

#endif