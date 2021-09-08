#include "opencv.hpp"

using namespace std;
using namespace cv;

/*----------------------------------------------------------------------
函数功能：输入一段视频，基于统计像素点的稳定值，来获取视频中静态灰度的背景
输入：capture 视频结构体，histroy 用于生成背景的视频帧数
	  bgname 输出的背景图片保存的文件名
----------------------------------------------------------------------*/
void buildBg(CvCapture* capture, int history = 100, const char bgname[FILENAME_MAX] = "bg.jpg")
{
	//判断视频是否读入
	if (NULL == capture)
	{
		cout << "Can not open video!" << endl;
		return;
	}
	
	//获取视频帧数
	int numFrames;
	numFrames = (int)cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_COUNT);
	
	//读取一帧图，和获取视频图片的大小
	IplImage* frame;
	frame = cvQueryFrame(capture);		
	CvSize sz;												
	sz = cvGetSize(frame);
	
	//转成灰度图片，以减少数据量
	IplImage* frame_gray;
	frame_gray =  cvCreateImage(sz, IPL_DEPTH_8U, 1);
	
	//获取图像的长宽，和判断history是否溢出，如果溢出，则设为视频总帧数
	int rows,cols,num;
	rows = frame->height;
	cols = frame->width;
	num = numFrames > history ? history : numFrames;
	
	//创建动态分配数组array，并初始化置0，可以用指针访问存取数据
	unsigned char ***array;
	array = new unsigned char **[rows];
	int i,j,k;
    for (i = 0; i < rows; i++)
	{
        array[i] = new unsigned char *[cols];
        for (j = 0; j < cols; j++)
		{
            array[i][j] =new unsigned char [num + 1];
            for (k = 0; k < num + 1; k++)
			{
                array[i][j][k] = 0;
			}
		}
	}
	
	//获取图片像素值，并将数据存入array数组，将视频读取点置为开头
	int count = 0;
	uchar *pixel;
	cvSetCaptureProperty(capture, CV_CAP_PROP_POS_FRAMES, 0);	
	while (count < num)
	{
		frame = cvQueryFrame(capture);							
		if (!frame)
			break;	
		
		cvCvtColor(frame, frame_gray, CV_BGR2GRAY);			
		for (i = 0; i < rows; i++)
		{
			for (j = 0; j < cols; j++)
			{
				//通过指针获取像素值，存入数组
				pixel = (uchar*)frame_gray->imageData + i * frame_gray->widthStep + j;
				array[i][j][count] = *pixel;
			}
		}
		count++;
	}
	
	//创建背景图片，初始化为0
	IplImage *BG;
	BG = cvCreateImage(sz, IPL_DEPTH_8U, 1);
	cvZero(BG);
	
	//创建用于统计像素值的一维256列的矩阵，并初始化置0，每列表示其像素值
	uint his[256];
	memset(his, 0, 256 * sizeof(uint));
	
	//most_frequent是统计该像素值的频次,index保存最高频次的点的像素值
	uint most_frequent = 0；
	int index;
	
	unit hiscount;
	for (i = 0; i < rows; i++)
	{
		for (j = 0; j < cols; j++)
		{
			for (k = 0; k < num; k++)
			{	
				//将同一个位置的点的像素值统计到his内
				his[array[i][j][k]]++;
			}
			for (hiscount = 0; hiscount < 256; hiscount++)
			{
				//找出his内最高频次的点，和保存其像素值
				if (his[hiscount]>most_frequent)
				{
					most_frequent = his[hiscount];
					index = hiscount;
				}
			}
			//通过指针访问背景图片的像素点，将获取的高频次的点保存进去
			pixel = (uchar*)BG->imageData + i * BG->widthStep + j;
			*pixel = index;
			
			//处理完一个点后，全部清0，准备处理下一个点。
			memset(his, 0, 256 * sizeof(uint));
			most_frequent = 0;
		}
	}
	
	//显示处理完成的背景图片
	//cvShowImage("background", BG);
	//cvWaitKey(0);
	
	//将处理完的图片保存到文件内
	cvSaveImage(bgname, BG);
	
	//释放内存，防止内存泄漏或溢出
	for (i = 0; i < rows; i++)
	{
		for (j = 0; j < cols; j++)
		{
			delete array[i][j];
		}
		delete array[i];
	}
	delete[] array;
	cvDestroyAllWindows();
	cvReleaseImage(&BG);
	cvReleaseImage(&frame_gray);
}