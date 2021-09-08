#pragma once
#ifndef __OBJSUBSTRACT_H__
#define __OBJSUBSTRACT_H__

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

tracks *curr_head = new tracks();
tracks *prev_head = new tracks();
tracks *losstrack = new tracks();

constexpr int MINIDIST = 30;
constexpr int VISIBALSIZE = 1500;
constexpr bool WRITETOFILE = false;								//是否将坐标值输出到文件
constexpr bool SHOWRESULT = true;

void bwboundaries(IplImage *I, IplImage* rect);					//前景标记
void buildlist(int x, int y, int wid, int hei, tracks *head);	//将目标串到链表最后

void release_obj_link(tracks *link);							//断开原链表所有连接
void assigntrack(IplImage* image);								//轨迹匹配
void restore(tracks *prehead);									//删除链表中的一个连接
void takeout(tracks *prehead);
int cal_dist(int x1, int y1, int x2, int y2);					//计算目标直线距离
void draw_line(IplImage *image, bboxlist *t);					//画出当前目标所有轨迹
void saveinfo(bboxlist *t);										//保存跟踪结果

string calImageValue(IplImage* src);							//计算图片的指纹信息
double Similarity(string &str1, string &str2);					//根据指纹信息计算两幅图像的相似度
void extractvalue(IplImage* frame, tracks *head);				//计算当前链表内所有目标的信息

int nFrmNum = 0;
int label_num = 0;
int outputlabel = -1;

#endif