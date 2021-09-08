#pragma once
#ifndef __TRACKING_H__
#define __TRACKING_H__

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
extern tracks *losstrack;
constexpr int MINIDIST = 30;

string calImageValue(IplImage* src);							//计算图片的指纹信息
double Similarity(string &str1, string &str2);					//根据指纹信息计算两幅图像的相似度
void extractvalue(IplImage* frame, tracks *head);				//计算当前链表内所有目标的信息
void saveinfo(bboxlist *t);										//保存跟踪结果

extern int nFrmNum;
extern int label_num;
extern int outputlabel;

#endif

