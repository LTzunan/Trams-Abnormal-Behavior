#include "objSubstract.h"
double framerate;

int objSubstract(CvCapture* capture, const char bgname[FILENAME_MAX])
{
	IplImage* frame;										//从视频读取原图像
	frame = cvQueryFrame(capture);
	if (!frame)
	{
		cout << "Can not read frame" << endl;
		return -1;
	}
	CvSize sz;												//获取图像大小
	sz = cvGetSize(frame);

	IplImage* BG;											//导入背景图像
	BG = cvLoadImage("bg.jpg", 0);
	if (!BG)
	{
		cout << "Can not read background" << endl;
		return -1;
	}
	
	cout << "输入目标的编号，第一次运行请输入 -1，查看结果后输入要保存的目标值" << endl;
	cout << "坐标输出在输入视频同一个文件夹内的data.csv文件" << endl;
	cout << "保存格式为 左上x,左上y,右下x,右下y" << endl;
	//cout << "输入要保存的目标的编号:" << endl;
	//cin >> outputlabel;

	//IplImage* moudle;										//掩模模板
	//moudle = cvCreateImage(sz, IPL_DEPTH_8U, 1);			//创建图像模板，用来选定处理区域
	//bw2(moudle);											//创建图像模板，用来掩模，去除不关心的部分

	//创建相应的图像并分配内存
	IplImage* frame_gray;										//创建灰度图像
	IplImage* rect;												//声明矩形框图像
	//IplImage* hole;												//创建用来填补的空洞图像
	IplImage* temp;												//用来存放临时图片
	IplImage* Imask;											//用来存放二值图片
	frame_gray = cvCreateImage(sz, IPL_DEPTH_8U, 1);
	rect = cvCreateImage(sz, IPL_DEPTH_8U, 3);					//创建矩形框图像
	//hole = cvCreateImage(sz, IPL_DEPTH_8U, 1);
	temp = cvCreateImage(sz, IPL_DEPTH_8U, 1);				//创建临时图像，用来暂时存储图像
	Imask = cvCreateImage(sz, IPL_DEPTH_8U, 1);

	IplConvKernel *kernel;										//膨胀处理模板
	kernel = cvCreateStructuringElementEx(10, 10, 4, 4, CV_SHAPE_RECT);	//膨胀处理模板

	framerate = cvGetCaptureProperty(capture, CV_CAP_PROP_FPS);
	//cout << framerate << endl;

	cvSetCaptureProperty(capture, CV_CAP_PROP_POS_FRAMES, 0);		//视频回到起始位置
	while (true)
	{
		frame = cvQueryFrame(capture);								//读取图像
		if (!frame)
			break;
		nFrmNum++;
		cvCopy(frame, rect);

		cvCvtColor(frame, frame_gray, CV_BGR2GRAY);					//图像灰度化
		cvAbsDiff(frame_gray, BG, temp);							//前景背景差分
		cvInRangeS(temp, cvScalar(40), cvScalar(256), Imask);		//取出差值大于30的像素点，并二值化
		cvSmooth(Imask, Imask, CV_MEDIAN, 3, 3);					//中值滤波
		cvDilate(Imask, Imask, kernel, 1);							//膨胀

		//cvAnd(Imask, moudle, Imask);								//对图像和模板按位与，掩模
		//cvCopy(Imask, hole);										//
		//cvFloodFill(hole, CvPoint(0, 0), CvScalar(255));			//获取空洞图
		//cvSubRS(hole, cvScalar(255), hole);							//空洞图取反
		//cvAdd(hole, Imask, Imask);									//填充空洞
		bwboundaries(Imask, rect);									//在视频帧图框出目标
																	//将目标信息保存到当前链表curr_head
		extractvalue(frame, curr_head);

		if (nFrmNum == 1)											//提取当前框内信息保存到head
		{
			extractvalue(frame, prev_head);
		}
		assigntrack(frame);											//将curr_head与pre_head匹配
		
		if (SHOWRESULT)
		{	
			cvNamedWindow("input", 0); cvShowImage("input", frame);					//显示原图
			//cvNamedWindow("output", 0);cvShowImage("output", Imask);				//显示差分后的图像
			//cvShowImage("output2", rect);											//显示框图
		}
		if (cvWaitKey(25) == 27)
			break;
	}

	cvDestroyAllWindows();
	cvReleaseImage(&BG);
	cvReleaseImage(&frame_gray);
	cvReleaseImage(&Imask);
	cvReleaseImage(&rect);
	//cvReleaseImage(&hole);
	cvReleaseStructuringElement(&kernel);
	return 0;
}

//图像连通域外接矩形框标识
void bwboundaries(IplImage *I, IplImage* rect)
{
	IplImage* temp;												//用来临时存放图片
	temp = cvCloneImage(I);
	CvMemStorage* storage = cvCreateMemStorage(0);				//查找连通体的储存空间
	CvSeq *contour = 0;											//连通体信息的结构体
	CvRect r;
	cvFindContours(temp, storage, &contour, sizeof(CvContour),
		CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, cvPoint(0, 0));

	cvZero(I);													//清除图像，下面重建图像
	double area;
	int total = 0;

	release_obj_link(curr_head->next);
	
	tracks *p = NULL;
	p = curr_head;
	p->next = NULL;
	p->bbox = NULL;

	while (contour) 
	{
		//获取当前轮廓面积
		area = cvContourArea(contour, CV_WHOLE_SEQ);
		area = fabs(cvContourArea(contour, CV_WHOLE_SEQ));		//获取当前轮廓面积
		if (area < VISIBALSIZE)
		{	
			contour = contour->h_next;
			continue;
		}
		total += 1;
		//画外接矩形
		r = ((CvContour*)contour)->rect;
		cvRectangle(rect, cvPoint(r.x, r.y), cvPoint(r.x + r.width, r.y + r.height), CV_RGB(255, 0, 0), 1, CV_AA, 0);
		cvDrawContours(I, contour, cvScalar(255), cvScalar(255), 0, CV_FILLED);

		//printf("第%d个目标左上坐标：（%d , %d）", total, r.x, r.y); 
		//printf("右下坐标：（%d , %d）\r\n", r.x + r.width, r.y + r.height);
		buildlist(r.x, r.y, r.width, r.height, curr_head);
		if (nFrmNum == 1)
		{
			buildlist(r.x, r.y, r.width, r.height, prev_head);
		}
		contour = contour->h_next;
	}
	//cout << "检测到的目标数：" << total << endl;
	//cout << endl;
	if (nFrmNum == 1)
	{
		for (p = prev_head->next; p; p = p->next)
		{
			label_num++;
			p->label = label_num;
		}
	}
	cvReleaseMemStorage(&storage);
	cvReleaseImage(&temp);
}

//保存跟踪结果
void saveinfo(bboxlist *t)
{
	ofstream outFile;
	bboxlist *t1 = t;
	static int flag = 0;
	if (flag == 0)			//如果是该目标的第一帧，则初始化data.csv
	{
		outFile.open("data.csv", ios::out);
		outFile << "frameNum,leftup_x,leftup_y,rightdown_x,rightdown_y,framerate" << endl;
		outFile.close();
		flag = 1;
	}
	if (flag == 1)			//如果是该目标的第二帧，则输出framerate
	{
		outFile.open("data.csv", ios::app);
		outFile << nFrmNum << "," << t1->x << "," << t1->y << "," << t1->x + t1->width << "," << t1->y + t1->height << "," << framerate << endl;
		outFile.close();
		flag = 2;
		return;
	}

	outFile.open("data.csv", ios::app);
	outFile << nFrmNum << "," << t1->x << "," << t1->y << "," << t1->x + t1->width << "," << t1->y + t1->height << endl;
	outFile.close();
	outFile.clear();
}