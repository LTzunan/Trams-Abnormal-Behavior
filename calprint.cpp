#include "calprint.h"

string calImageValue(IplImage* src)
{
	string resStr(64, '\0');
	IplImage* image = cvCreateImage(cvGetSize(src), src->depth, 1);
	//step one : 灰度化
	if (src->nChannels == 3)
		cvCvtColor(src, image, CV_BGR2GRAY);
	else
		cvCopy(src, image);
	//step two : 缩小尺寸 8*8
	IplImage* temp = cvCreateImage(cvSize(8, 8), image->depth, 1);

	cvResize(image, temp);
	//step three : 简化色彩
	uchar* pData;
	for (int i = 0; i < temp->height; i++)
	{
		pData = (uchar*)(temp->imageData + i * temp->widthStep);
		for (int j = 0; j < temp->width; j++)
			pData[j] = pData[j] / 4;
	}
	//step four : 计算平均灰度值
	int average = (int)cvAvg(temp).val[0];
	//step five : 计算特征值
	int index = 0;
	for (int i = 0; i < temp->height; i++)
	{
		pData = (uchar*)(temp->imageData + i * temp->widthStep);
		for (int j = 0; j < temp->width; j++)
		{
			if (pData[j] >= average)
				resStr[index++] = '1';
			else
				resStr[index++] = '0';
		}
	}
	return resStr;
}

//根据指纹信息计算两幅图像的相似度
double Similarity(string &str1, string &str2)
{
	double similarity = 1.0;
	for (int i = 0; i < 64; i++)
	{
		char c1 = str1[i];
		char c2 = str2[i];
		if (c1 != c2)
			similarity = similarity - 1.0 / 64;
	}
	return similarity;
}

//提取目标信息
void extractvalue(IplImage* frame, tracks *head)
{
	tracks* temp = NULL;
	IplImage *imageprint;
	for (temp = head->next; temp; temp = temp->next)
	{
		cvSetImageROI(frame, cvRect(temp->bbox->x, temp->bbox->y, temp->bbox->width, temp->bbox->height));
		imageprint = cvCreateImage(cvSize(temp->bbox->width, temp->bbox->height), frame->depth, frame->nChannels);
		cvCopy(frame, imageprint);
		temp->imgPrint = calImageValue(imageprint);
		cvResetImageROI(frame);
		cvReleaseImage(&imageprint);
	}
}
