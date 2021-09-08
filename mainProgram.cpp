#include "opencv.hpp"
#include "windows.h"

void getfile();
TCHAR szFile[MAX_PATH];
void buildBg(CvCapture* capture, int history = 100, const char bgname[FILENAME_MAX] = "bg.jpg");		//频数统计背景，并保存
int objSubstract(CvCapture* capture, const char bgname[FILENAME_MAX]);									//背景减除

using namespace std;

int main()
{
	getfile();
	CvCapture* capture;
	capture = cvCreateFileCapture(szFile);   //argv[1]
	
	if (NULL == capture)
	{
		cout << "Can not open video!" << endl;
		getchar();
		return -1;
	}
	buildBg(capture, 150, "bg.jpg"); 

	cvSetCaptureProperty(capture, CV_CAP_PROP_POS_FRAMES, 0);
	objSubstract(capture, "bg.jpg");

	cvReleaseCapture(&capture);
	return 0;
}

void getfile()
{
	OPENFILENAME ofn;      // 公共对话框结构。

	// 初始化选择文件对话框。     
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = "All(*.*)\0*.*\0MP4(*.MP4)\0*.MP4\0AVI(*.AVI)\0*.AVI\0\0";

	GetOpenFileName(&ofn);
}