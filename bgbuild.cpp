#include "opencv.hpp"

using namespace std;
using namespace cv;

/*----------------------------------------------------------------------
�������ܣ�����һ����Ƶ������ͳ�����ص���ȶ�ֵ������ȡ��Ƶ�о�̬�Ҷȵı���
���룺capture ��Ƶ�ṹ�壬histroy �������ɱ�������Ƶ֡��
	  bgname ����ı���ͼƬ������ļ���
----------------------------------------------------------------------*/
void buildBg(CvCapture* capture, int history = 100, const char bgname[FILENAME_MAX] = "bg.jpg")
{
	//�ж���Ƶ�Ƿ����
	if (NULL == capture)
	{
		cout << "Can not open video!" << endl;
		return;
	}
	
	//��ȡ��Ƶ֡��
	int numFrames;
	numFrames = (int)cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_COUNT);
	
	//��ȡһ֡ͼ���ͻ�ȡ��ƵͼƬ�Ĵ�С
	IplImage* frame;
	frame = cvQueryFrame(capture);		
	CvSize sz;												
	sz = cvGetSize(frame);
	
	//ת�ɻҶ�ͼƬ���Լ���������
	IplImage* frame_gray;
	frame_gray =  cvCreateImage(sz, IPL_DEPTH_8U, 1);
	
	//��ȡͼ��ĳ������ж�history�Ƿ������������������Ϊ��Ƶ��֡��
	int rows,cols,num;
	rows = frame->height;
	cols = frame->width;
	num = numFrames > history ? history : numFrames;
	
	//������̬��������array������ʼ����0��������ָ����ʴ�ȡ����
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
	
	//��ȡͼƬ����ֵ���������ݴ���array���飬����Ƶ��ȡ����Ϊ��ͷ
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
				//ͨ��ָ���ȡ����ֵ����������
				pixel = (uchar*)frame_gray->imageData + i * frame_gray->widthStep + j;
				array[i][j][count] = *pixel;
			}
		}
		count++;
	}
	
	//��������ͼƬ����ʼ��Ϊ0
	IplImage *BG;
	BG = cvCreateImage(sz, IPL_DEPTH_8U, 1);
	cvZero(BG);
	
	//��������ͳ������ֵ��һά256�еľ��󣬲���ʼ����0��ÿ�б�ʾ������ֵ
	uint his[256];
	memset(his, 0, 256 * sizeof(uint));
	
	//most_frequent��ͳ�Ƹ�����ֵ��Ƶ��,index�������Ƶ�εĵ������ֵ
	uint most_frequent = 0��
	int index;
	
	unit hiscount;
	for (i = 0; i < rows; i++)
	{
		for (j = 0; j < cols; j++)
		{
			for (k = 0; k < num; k++)
			{	
				//��ͬһ��λ�õĵ������ֵͳ�Ƶ�his��
				his[array[i][j][k]]++;
			}
			for (hiscount = 0; hiscount < 256; hiscount++)
			{
				//�ҳ�his�����Ƶ�εĵ㣬�ͱ���������ֵ
				if (his[hiscount]>most_frequent)
				{
					most_frequent = his[hiscount];
					index = hiscount;
				}
			}
			//ͨ��ָ����ʱ���ͼƬ�����ص㣬����ȡ�ĸ�Ƶ�εĵ㱣���ȥ
			pixel = (uchar*)BG->imageData + i * BG->widthStep + j;
			*pixel = index;
			
			//������һ�����ȫ����0��׼��������һ���㡣
			memset(his, 0, 256 * sizeof(uint));
			most_frequent = 0;
		}
	}
	
	//��ʾ������ɵı���ͼƬ
	//cvShowImage("background", BG);
	//cvWaitKey(0);
	
	//���������ͼƬ���浽�ļ���
	cvSaveImage(bgname, BG);
	
	//�ͷ��ڴ棬��ֹ�ڴ�й©�����
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