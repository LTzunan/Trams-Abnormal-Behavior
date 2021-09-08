#include "objSubstract.h"
double framerate;

int objSubstract(CvCapture* capture, const char bgname[FILENAME_MAX])
{
	IplImage* frame;										//����Ƶ��ȡԭͼ��
	frame = cvQueryFrame(capture);
	if (!frame)
	{
		cout << "Can not read frame" << endl;
		return -1;
	}
	CvSize sz;												//��ȡͼ���С
	sz = cvGetSize(frame);

	IplImage* BG;											//���뱳��ͼ��
	BG = cvLoadImage("bg.jpg", 0);
	if (!BG)
	{
		cout << "Can not read background" << endl;
		return -1;
	}
	
	cout << "����Ŀ��ı�ţ���һ������������ -1���鿴���������Ҫ�����Ŀ��ֵ" << endl;
	cout << "���������������Ƶͬһ���ļ����ڵ�data.csv�ļ�" << endl;
	cout << "�����ʽΪ ����x,����y,����x,����y" << endl;
	//cout << "����Ҫ�����Ŀ��ı��:" << endl;
	//cin >> outputlabel;

	//IplImage* moudle;										//��ģģ��
	//moudle = cvCreateImage(sz, IPL_DEPTH_8U, 1);			//����ͼ��ģ�壬����ѡ����������
	//bw2(moudle);											//����ͼ��ģ�壬������ģ��ȥ�������ĵĲ���

	//������Ӧ��ͼ�񲢷����ڴ�
	IplImage* frame_gray;										//�����Ҷ�ͼ��
	IplImage* rect;												//�������ο�ͼ��
	//IplImage* hole;												//����������Ŀն�ͼ��
	IplImage* temp;												//���������ʱͼƬ
	IplImage* Imask;											//������Ŷ�ֵͼƬ
	frame_gray = cvCreateImage(sz, IPL_DEPTH_8U, 1);
	rect = cvCreateImage(sz, IPL_DEPTH_8U, 3);					//�������ο�ͼ��
	//hole = cvCreateImage(sz, IPL_DEPTH_8U, 1);
	temp = cvCreateImage(sz, IPL_DEPTH_8U, 1);				//������ʱͼ��������ʱ�洢ͼ��
	Imask = cvCreateImage(sz, IPL_DEPTH_8U, 1);

	IplConvKernel *kernel;										//���ʹ���ģ��
	kernel = cvCreateStructuringElementEx(10, 10, 4, 4, CV_SHAPE_RECT);	//���ʹ���ģ��

	framerate = cvGetCaptureProperty(capture, CV_CAP_PROP_FPS);
	//cout << framerate << endl;

	cvSetCaptureProperty(capture, CV_CAP_PROP_POS_FRAMES, 0);		//��Ƶ�ص���ʼλ��
	while (true)
	{
		frame = cvQueryFrame(capture);								//��ȡͼ��
		if (!frame)
			break;
		nFrmNum++;
		cvCopy(frame, rect);

		cvCvtColor(frame, frame_gray, CV_BGR2GRAY);					//ͼ��ҶȻ�
		cvAbsDiff(frame_gray, BG, temp);							//ǰ���������
		cvInRangeS(temp, cvScalar(40), cvScalar(256), Imask);		//ȡ����ֵ����30�����ص㣬����ֵ��
		cvSmooth(Imask, Imask, CV_MEDIAN, 3, 3);					//��ֵ�˲�
		cvDilate(Imask, Imask, kernel, 1);							//����

		//cvAnd(Imask, moudle, Imask);								//��ͼ���ģ�尴λ�룬��ģ
		//cvCopy(Imask, hole);										//
		//cvFloodFill(hole, CvPoint(0, 0), CvScalar(255));			//��ȡ�ն�ͼ
		//cvSubRS(hole, cvScalar(255), hole);							//�ն�ͼȡ��
		//cvAdd(hole, Imask, Imask);									//���ն�
		bwboundaries(Imask, rect);									//����Ƶ֡ͼ���Ŀ��
																	//��Ŀ����Ϣ���浽��ǰ����curr_head
		extractvalue(frame, curr_head);

		if (nFrmNum == 1)											//��ȡ��ǰ������Ϣ���浽head
		{
			extractvalue(frame, prev_head);
		}
		assigntrack(frame);											//��curr_head��pre_headƥ��
		
		if (SHOWRESULT)
		{	
			cvNamedWindow("input", 0); cvShowImage("input", frame);					//��ʾԭͼ
			//cvNamedWindow("output", 0);cvShowImage("output", Imask);				//��ʾ��ֺ��ͼ��
			//cvShowImage("output2", rect);											//��ʾ��ͼ
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

//ͼ����ͨ����Ӿ��ο��ʶ
void bwboundaries(IplImage *I, IplImage* rect)
{
	IplImage* temp;												//������ʱ���ͼƬ
	temp = cvCloneImage(I);
	CvMemStorage* storage = cvCreateMemStorage(0);				//������ͨ��Ĵ���ռ�
	CvSeq *contour = 0;											//��ͨ����Ϣ�Ľṹ��
	CvRect r;
	cvFindContours(temp, storage, &contour, sizeof(CvContour),
		CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, cvPoint(0, 0));

	cvZero(I);													//���ͼ�������ؽ�ͼ��
	double area;
	int total = 0;

	release_obj_link(curr_head->next);
	
	tracks *p = NULL;
	p = curr_head;
	p->next = NULL;
	p->bbox = NULL;

	while (contour) 
	{
		//��ȡ��ǰ�������
		area = cvContourArea(contour, CV_WHOLE_SEQ);
		area = fabs(cvContourArea(contour, CV_WHOLE_SEQ));		//��ȡ��ǰ�������
		if (area < VISIBALSIZE)
		{	
			contour = contour->h_next;
			continue;
		}
		total += 1;
		//����Ӿ���
		r = ((CvContour*)contour)->rect;
		cvRectangle(rect, cvPoint(r.x, r.y), cvPoint(r.x + r.width, r.y + r.height), CV_RGB(255, 0, 0), 1, CV_AA, 0);
		cvDrawContours(I, contour, cvScalar(255), cvScalar(255), 0, CV_FILLED);

		//printf("��%d��Ŀ���������꣺��%d , %d��", total, r.x, r.y); 
		//printf("�������꣺��%d , %d��\r\n", r.x + r.width, r.y + r.height);
		buildlist(r.x, r.y, r.width, r.height, curr_head);
		if (nFrmNum == 1)
		{
			buildlist(r.x, r.y, r.width, r.height, prev_head);
		}
		contour = contour->h_next;
	}
	//cout << "��⵽��Ŀ������" << total << endl;
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

//������ٽ��
void saveinfo(bboxlist *t)
{
	ofstream outFile;
	bboxlist *t1 = t;
	static int flag = 0;
	if (flag == 0)			//����Ǹ�Ŀ��ĵ�һ֡�����ʼ��data.csv
	{
		outFile.open("data.csv", ios::out);
		outFile << "frameNum,leftup_x,leftup_y,rightdown_x,rightdown_y,framerate" << endl;
		outFile.close();
		flag = 1;
	}
	if (flag == 1)			//����Ǹ�Ŀ��ĵڶ�֡�������framerate
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