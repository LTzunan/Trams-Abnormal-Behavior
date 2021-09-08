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
constexpr bool WRITETOFILE = false;								//�Ƿ�����ֵ������ļ�
constexpr bool SHOWRESULT = true;

void bwboundaries(IplImage *I, IplImage* rect);					//ǰ�����
void buildlist(int x, int y, int wid, int hei, tracks *head);	//��Ŀ�괮���������

void release_obj_link(tracks *link);							//�Ͽ�ԭ������������
void assigntrack(IplImage* image);								//�켣ƥ��
void restore(tracks *prehead);									//ɾ�������е�һ������
void takeout(tracks *prehead);
int cal_dist(int x1, int y1, int x2, int y2);					//����Ŀ��ֱ�߾���
void draw_line(IplImage *image, bboxlist *t);					//������ǰĿ�����й켣
void saveinfo(bboxlist *t);										//������ٽ��

string calImageValue(IplImage* src);							//����ͼƬ��ָ����Ϣ
double Similarity(string &str1, string &str2);					//����ָ����Ϣ��������ͼ������ƶ�
void extractvalue(IplImage* frame, tracks *head);				//���㵱ǰ����������Ŀ�����Ϣ

int nFrmNum = 0;
int label_num = 0;
int outputlabel = -1;

#endif