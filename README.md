# Trams-Abnormal-Behavior
Detection of abnormal behavior in trams

[TOC]

### 代码整体流程图：

![image-20210907154615579](http://ltzunanimage.oss-cn-shenzhen.aliyuncs.com/img/image-20210907154615579.png)

可识别的异常行为：**摔倒**、**徘徊、越界**

运行环境：**Windows10+Visual Studio 2019/2017+ OpenCV2.4.13.6**

说明：

1. 在运行代码前，需要将代码中的各种路径参数修改为自己本人电脑中存储相应文件的路径，例如，源代码中视频文件路径为 `X:\\车厢内异常行为识别\\视频材料\\有轨电车内摔倒徘徊视频\\视频\\单人徘徊2.mp4` ，而假设本身电脑中视频的路径为`D:\车厢内异常行为识别\单人徘徊2.mp4`，则应将源代码中的路径改为`D:\\车厢内异常行为识别\\单人徘徊2.mp4`，其他文件路径同理。

2.  **视频格式匹配**：行为判断规则中，“摔倒”和“徘徊”设计的指标，有部分指标依赖于视频格式，视频格式包括视频帧宽、高和帧率，本项目测试视频帧率为25帧/秒。这些指标分别是：0.12秒间隔（25 * 0.12 = 3帧）、0.2秒间隔（25 * 0.2 = 5帧）、1.44秒间隔（36帧）、2.92秒间隔（73帧）、边界框左上角纵坐标像素距离、边界框左上角纵坐标差值、边界框右下角纵坐标、边界框左下角横坐标差值、边界框右下角横坐标差值。具体做法是：读取视频帧宽高和帧率，然后更新上述指标即可。具体代码为tracking.cpp中的：

```c++
#define compare_with (int)(frate*0.12+0.5) //保存几帧前的信息（如果是要保存0.12秒前的信息，frate为视频帧率）
#define detectfrom (int)(frate*0.2+0.5)  //从第几帧起开始判断（如果是视频开始后0.2秒开始判断，frate为视频帧率）
```

也可用Pr等软件将测试视频的帧率修改为25帧/秒。

### 1. 正负样本归一化

```c++
/*对图片大小进行归一化并放在新的文件夹内，然后重新将新文件夹每个文件名写入到txt文件中（可优化，将几个函数合为一个函数）*/
Pos_List();
Neg_List();
resize_and_flip_Pos();
resize_and_flip_Neg();
Pos_List_AfterFlip();
Neg_List_AfterFlip();
```

`Main()` 函数首先调用 `Create_List.cpp` 中的 `Pos_List()` 和 `Neg_List()` 两个函数，将正样本图片依次写入到 `PositiveImageList.txt`，将负样本图片依次写入到 `NegativeImageList.txt`  文本文档中。

然后调用 `Resize.cpp` 中的 `resize_and_flip_Pos()` 函数将正样本归一化为128\*256大小并做镜像处理，再调用 `Resize.cpp` 中的 `resize_and_flip_Neg()` 函数归一化负样本大小并做镜像处理，负样本情况较复杂，若高度值大于宽度值，则归一化大小为128\*256，若高度值小于宽度值，则归一化大小为320\*240，做镜像后正负样本数量增加一倍。

最后，调用 `Create_List.cpp` 中的 `Pos_List_AfterFlip()` 和 `Neg_List_AfterFlip()` 函数，分别将归一化并做镜像处理后的正负样本的新文件夹的图片文件名循环写入到 `PositiveImageList.txt` 和 `NegativeImageList.txt` 中。

```c++
set_SVM("X:\\车厢内异常行为识别\\整合\\训练文本\\灰度+Sobel.xml");	//训练分类器
```

在归一化之后，`main()` 函数调用 `SetSVM.cpp` 中的 `set_SVM(“const char* A”)` 函数提取样本的HOG特征并进行分类器训练，`const char* A`是训练好后的svm模型所保存的xml路径及文件。

![image-20210907155449229](http://ltzunanimage.oss-cn-shenzhen.aliyuncs.com/img/image-20210907155449229.png)

### 2. 正负样本预处理（缩放，灰度化，边缘检测）

```c++
ImgName = "X:\\车厢内异常行为识别\\整合\\样本\\归一化样本\\素材pos帧_resize\\" + ImgName;//加上正样本的路径名

Mat src = imread(ImgName);//读取图片
resize(src,src,Size(64,128)); //将正样本图片缩为64*128

cvtColor(src, src,CV_RGB2GRAY);	//对图像灰度化

//Sobel算子进行运算，进行边缘检测
Mat grad_x, grad_y;
Mat abs_grad_x, abs_grad_y;
Sobel(src, grad_x, CV_16S, 1, 0, 3, 1, 1,BORDER_DEFAULT);
convertScaleAbs(grad_x, abs_grad_x);	//将图片转化成为8位图形
Sobel(src, grad_y, CV_16S, 0,  1,3, 1, 1, BORDER_DEFAULT);
convertScaleAbs(grad_y,abs_grad_y);
addWeighted(abs_grad_x, 0.5, abs_grad_y, 0.5, 0, src);
```

在提取HOG特征之前，程序先对归一化后的正负样本进行预处理：将图片缩放为64*128大小，并进行灰度化，然后用Sobel算子提取边缘，以提升训练、检测的速度，上面是正样本进行预处理的代码示例（负样本预处理操作相同，只是读取路径时为负样本的路径）

### 3. HOG特征提取

```c++
vector<float> descriptors;//HOG描述子向量
hog.compute(src,descriptors,Size(8,8));//计算HOG描述子，检测窗口移动步长(8,8)
```

首先声明HOG描述子向量 `descriptors`，然后调用 `hog.compute()` 函数计算HOG描述子descriptors，步长为(8,8)

```c++
Mat sampleFeatureMat;	//所有训练样本的特征向量组成的矩阵，行数等于所有样本的个数，列数等于HOG描述子维数
Mat sampleLabelMat;	//训练样本的类别向量，行数等于所有样本的个数，列数等于1；1表示有人，-1表示无人

//处理第一个样本时初始化特征向量矩阵和类别矩阵，因为只有知道了特征向量的维数才能初始化特征向量矩阵
if(0 == num)
{
	DescriptorDim = descriptors.size();//HOG描述子的维数

	//初始化所有训练样本的特征向量组成的矩阵，行数等于所有样本的个数，列数等于HOG描述子维数DescriptorDim
	sampleFeatureMat = Mat::zeros(PosSamNO + NegSamNO + HardExampleNO, DescriptorDim, CV_32FC1);

	//初始化训练样本的类别向量，行数等于所有样本的个数，列数等于1；1表示有人，-1表示无人
	sampleLabelMat = Mat::zeros(PosSamNO + NegSamNO + HardExampleNO, 1, CV_32FC1);
}

```

然后声明特征向量矩阵 `sampleFeatureMat` 和样本类别矩阵 `sampleLabelMat`，并在处理第一个样本时对它们进行初始化。

```c++
//将计算好的HOG描述子复制到样本特征矩阵sampleFeatureMat
for(int i=0; i<DescriptorDim; i++)
{
	sampleFeatureMat.at<float>(num,i) = descriptors[i];//第num个样本的特征向量中的第i个元素
}
sampleLabelMat.at<float>(num,0) = 1;//正样本类别为1，有人；负样本类别为-1，无人
```

初始化后，先后将计算好的正负样本HOG描述子descriptors赋值到特征矩阵中 `sampleFeatureMat` 中，而类别矩阵 `sampleLabelMat` 中样本类别1表示有人，-1表示无人。

### 4. 进行SVM分类器训练

在本设计的SVM参数中，SVM分类器类型为C类支持向量分类器 `CvSVM::C_SVC`，核函数为线性核函数 `CvSVM::LINEAR`，核函数的参数gamma设定为1，参数C设定为0.01（分别对应于下面代码中 `CvSVMParams param` 后的1和0.01，其中参数C为对于误分类样本的惩罚性因子,C越大惩罚性越强,相当于对数据的信心越大,数据噪声比较小）；迭代终止条件为迭代满1000次或误差小于FLT_EPSILON = 1.192092896e-07F

```c++
//迭代终止条件，当迭代满1000次或误差小于FLT_EPSILON时停止迭代
CvTermCriteria criteria = cvTermCriteria(CV_TERMCRIT_ITER+CV_TERMCRIT_EPS, 1000, FLT_EPSILON);

//SVM参数：SVM类型为C_SVC；线性核函数；松弛因子C=0.01
CvSVMParams param(CvSVM::C_SVC, CvSVM::LINEAR, 0, 1, 0, 0.01, 0, 0, 0, criteria);

cout<<"开始训练SVM分类器"<<endl;
start = clock();
svm.train(sampleFeatureMat, sampleLabelMat, Mat(), Mat(), param); /////////
end = clock();
time = (end-start)/CLOCKS_PER_SEC;
cout<<"SVM训练完成！"<<endl;
cout<<"SVM训练所需时间为"<<time<<"秒"<<endl;

svm.save(A);//将训练好的SVM模型保存为xml文件
```

如上面代码所示，设定好上述SVM参数后，用 `svm.train()` 函数开始训练SVM分类器，训练完成后将SVM模型保存为xml文件到A的路径（一开始调用set_SVM()函数时传入的路径参数）。

```c++
char videoo[] = "X:\\车厢内异常行为识别\\视频材料\\有轨电车内摔倒徘徊视频\\视频\\单人摔倒2.mp4";
CvCapture* capture = cvCaptureFromFile(videoo);
frate = cvGetCaptureProperty(capture, CV_CAP_PROP_FPS) + 0.5; //获取帧率
cout << "视频帧率：" << frate << endl;

Detecting("X:\\车厢内异常行为识别\\整合\\训练文本\\灰度+Sobel.xml", videoo, nuno1, nuno2);
```

在SVM模型训练完成后，程序返回到 `main()` 函数， `main()` 继续调用 `Detection.cpp` 中的 `Detecting()` 函数并输入SVM模型（即“`XXX.xml`”文件）和测试视频进行目标检测，同时，获取测试视频的帧率frate。

![image-20210907160304130](http://ltzunanimage.oss-cn-shenzhen.aliyuncs.com/img/image-20210907160304130.png)

### 5. 计算 g(x) = wT\*x + b 中的检测子 wT 和 b

由于在图像领域内，数据不可能做到线性可分，所以本设计使用SVM线性不可分的思想进行实验，即将数据映射到一个高维的平面上，从而在更高维的平面上完成线性分类。其决策函数表达式如式3-1：
$$
g(x) = wT\times x + b\tag{3-1}
$$
故在检测之前，需先计算检测子 wT 和 b（偏移量），其中由代码

```c++
//计算-(alphaMat * supportVectorMat),结果放到resultMat中，加负号是因为HOG进行提取的矩阵和计算出来的矩阵结果是相反的（网上的解释）
resultMat = -1.0 * alphaMat *  supportVectorMat;
//将resultMat中的数据复制到数组myDetector第一行中
for (int i = 0; i < DescriptorDim; i++)
{
	myDetector.push_back(resultMat.at<float>(0, i));
}
```

计算得出 wT

再添加偏移量 roh（即b），得到最终可用的检测子

```c++
//最后添加偏移量rho，得到检测子
myDetector.push_back(svm.get_rho());
cout << "检测子维数：" << myDetector.size() << endl;
//设置HOGDescriptor的检测子
HOGDescriptor myHOG;
myHOG.setSVMDetector(myDetector);
```

### 6. 视频图片预处理（缩放，灰度化，边缘检测）

```c++
Mat src = cvarrToMat(frame);

resize(src, src, Size(num1, num2));	//缩小图片
Mat src_Gray;
namedWindow("原图", 1);
imshow("原图", src);

cvtColor(src, src_Gray, CV_RGB2GRAY);	//灰度化图像（预处理步骤）

//Sobel算子进行运算
Mat grad_x, grad_y;
Mat abs_grad_x, abs_grad_y;
Sobel(src_Gray, grad_x, CV_16S, 1, 0, 3, 1, 1, BORDER_DEFAULT);
convertScaleAbs(grad_x, abs_grad_x);	//将图片转化成为8位图形
Sobel(src_Gray, grad_y, CV_16S, 0, 1, 3, 1, 1, BORDER_DEFAULT);
convertScaleAbs(grad_y, abs_grad_y);
addWeighted(abs_grad_x, 0.3, abs_grad_y, 0.7, 0, src_Gray);
namedWindow("after_Sobel", 1);
imshow("after_Sobel", src_Gray);
```

在得到检测子并设置到 myHOG 后，开始读入视频图片进行 HOG 目标检测，在检测前，同样先对视频图片进行缩放大小、灰度化和边缘检测的预处理

### 7. 进行HOG多尺度检测，保存所有矩形框信息

```c++
vector<Rect> found, found_filtered;             //**********矩形框数组*************
myHOG.detectMultiScale(src_Gray, found, 0, Size(8, 8), Size(32, 32), 1.5, 1);   //对图片进行多尺度行人检测（交通路口1.03-1.05，车厢1.3-1.5）
cout << "找到的矩形框个数：" << found.size() << endl;
```

声明矩形框数组 found 和 found_filtered，并进行 HOG 多尺度检测，并将所有矩形框信息保存在 found 中

### 8. 帧间处理（过滤不合适的矩形框数据，矩形框越界等操作）

```c++
int flag = 0;	//前后帧突然出现方框的标记
for (int i = 0; i < found.size(); i++)
{
Rect r = found[i];
int j = 0, T = 0;
flag = 0;
if (num != 0)
{
	//过滤坐标与上一帧坐标偏离太远的坐标
	for (int k = 0; pre2_found[k][0] != 0; k++)
	{
		//cout<<1<<endl;
		if (((found[i].x - pre2_found[k][0])*(found[i].x - pre2_found[k][0]) < 900) && ((found[i].y - pre2_found[k][1])*(found[i].y - pre2_found[k][1]) < 900))
		{
			for (int K = 0; pre_found[K][0] != 0; K++)
			{
				if (((found[i].x - pre_found[K][0])*(found[i].x - pre_found[K][0]) < 225) && ((found[i].y - pre_found[K][1])*(found[i].y - pre_found[K][1]) < 225))
				{
					flag = 2;	//存在相邻数组
					break;
				}
			}
		}
	}
}
for (j = 0; j < found.size(); j++)
{
	if (flag != 2)
		break;
	//找出所有没有嵌套的矩形框r,如果有嵌套的话,则取外面最大的那个矩形框
	if ((j != i) && ((r & found[j]) == r))
		break;
}
```
进行帧间处理是为了过滤掉矩形框数组 found 中一些坐标与上一帧坐标偏离太远的矩形框和嵌套在大矩形框里的小矩形框，具体方法是：

1. 检测第 x 帧时，将前 n 帧（本设计的 n = 2）程序识别出的方框储存起来。然后先取出第 x 帧检测出来的方框的左上角坐标与前一帧 pre_found 距离该坐标距离最近的左上角的坐标，算出两者的欧氏距离，如果距离大于15个像素点，则将其过滤掉；然后再将该坐标与前两帧 pre2_found 距离该坐标距离最近的左上角的坐标，再次算出欧氏距离，如果距离大于30个像素点，再将其过滤掉，最后输出没有被过滤掉的坐标。具体代码实现如下框，其中 if 判断中的 900 和 225 分别是 30 和 15（像素点）的平方。
2. 找出所有没有嵌套的矩形框r,如果有嵌套的话,则取外面最大的那个矩形框。


```c++
if (j == found.size())
	found_filtered.push_back(r);          //将过滤后的矩形框的数据传入矩形框数组found_filtered中

num = found.size();
```

在过滤完成后，将过滤后的矩形框数据传入矩形框数组 found_filtered 中

```c++
if (r->x < 0) r->x = 0;           //防止矩形框越界
if (r->y < 0) r->y = 0;           //防止矩形框越界
```

此外，为防止矩形框越界（即显示的矩形框超出视频图像范围外），程序设计为：如果有超出的部分，则将超出的部分的起始坐标直接设为0

### 9. 将过滤后的矩形框数组作为参数传入，调用 objSubstract() 函数，转入跟踪模块

```c++
//使用缩放后的图像帧作为显示帧，目的是为了对应svm检测时使用了缩放的图像
IplImage* fra_img;
fra_img = &IplImage(src);

objSubstract(fra_img, found_filtered);   //***转入跟踪模块****  
```

过滤完成后，检测模块结束，转入跟踪模块，调用 `objSubstract.cpp` 中的 `objSubstract()` 函数，将过滤后得到的最终矩形框数组 `found_filtered` 和缩放后的图像帧作为参数传入

![image-20210907161816921](http://ltzunanimage.oss-cn-shenzhen.aliyuncs.com/img/image-20210907161816921.png)

![image-20210907161831392](http://ltzunanimage.oss-cn-shenzhen.aliyuncs.com/img/image-20210907161831392.png)

输入参数是当前帧的矩形框数组、当前帧的图像，经过处理会将矩形框进行跟踪并标号，首先修改存储矩形框个数的数组，保证 `numk[0]` 始终是当前帧的矩形框数量， `numk[x]` 始终为前 x 帧的矩形框数量，

### 10. 清除链表数据后，将矩形框数组存入链表中，并将计算的特征值也存储进去

1. `bwboundaries` 函数：输入矩形框数组found，依次调用以下函数

   - `release_obj_link(curr_head->next)` ，给存储当前帧矩形框的链表（curr_head）清除链接。
   - 两次调用 `buildlist`（注：输入矩形框数组中的左上角坐标值、长、宽、链表头指针），建立当前帧链表（curr_head）和前一帧链表（prev_head）。过程：依次矩形框数组中的元素，将数据存储到链表结构体中。

2. 两次调用 `extractvalue`（注：输入当前帧图像信息frame、链表头指针），将提取的目标信息放入到当前帧链表（curr_head）和前一帧链表（prev_head）中

   ![image-20210907162143915](http://ltzunanimage.oss-cn-shenzhen.aliyuncs.com/img/image-20210907162143915.png)

   - ` cvSetImageROI`：按矩形框裁剪图片。

   - `cvCreateImage`：给imageprint分配内存空间。

   - `cvCopy`：复制图片，将裁剪完后的图片复制给imageprint。

   - 调用`calImageValue`，将图片依次执行以下操作：1.灰度化 2.缩小尺寸 8\*8 3.简化色彩 4.计算平均灰度值 5.计算特征值。

   - ```c++
     1.if (src->nChannels == 3) cvCvtColor(src, image, CV_BGR2GRAY);
     elsecvCopy (src, image)；
     2.IplImage* temp = cvCreateImage(cvSize(8, 8), image->depth, 1;
     3.for (int i = 0; i < temp->height; i++){
     pData = (uchar*)(temp->imageData + i * temp->widthStep);
     for (int j = 0; j < temp->width; j++)
     pData[j] = pData[j] / 4;
     }
     4.int average = (int)cvAvg(temp).val[0];
     5.int index = 0;
     for (int i = 0; i < temp->height; i++)
     {pData = (uchar*)(temp->imageData + i * temp->widthStep);
     for (int j = 0; j < temp->width; j++)
     {if (pData[j] >= average)
     resStr[index++] = '1';
     else
     esStr[index++] = '0';}
     }
     ```

   - `cvResetImageROI` :释放基于给定的矩形设置图像的ROI

### 11. 运用距离结合目标特征相似度判断目标轨迹并给目标编号

调用 `assigntrack(当前帧图像信息frame)` ，进行目标跟踪，并得到每隔约 0.12s 的矩形框信息，包括左上角坐标值，矩形框的长宽。

1. 调用 opencv 函数 `cvRectangle` 绘制矩形框。

2. 以当前帧矩形框数据作为外层循环，依次与上一帧矩形框数据进行比对。`cal_dist` 进行质心距离计算，返回值为质心距离。若质心距离超过 MINIDIST 即为符合条件，当前循环的矩形框与原目标所有矩形框均为距离过大，则假设为新目标，并编号。

3. 由于重心偏移可能会出现误差，在经过重心偏移标号后，再进行特征匹配。

   ![image-20210907162429572](http://ltzunanimage.oss-cn-shenzhen.aliyuncs.com/img/image-20210907162429572.png)

   Similarity用于计算两张图片的相似度。

   ![image-20210907162444806](http://ltzunanimage.oss-cn-shenzhen.aliyuncs.com/img/image-20210907162444806.png)

   str1，str2 为图像特征值。假设 similarity为1 则两张图片相等，在 Similarity 函数中，若有判断 str1 不等于 str2 ，则相似度减去 1/64。由于之前已定义图像特征值有64个，当所有特征均不匹配时，则 similarity 为 0。similarity 越大，图像相似度越高。

   若图像符合如果只是简单的根据距离进行匹配，在目标较多时会造成目标重复匹配或错位，因此需要同时进行距离的比较，当特征值相似度比较高且距离较近时，才认为二者是同一个目标。这种方法可以进一步提高匹配的准确度，为了降低误检率，还可将特征阈值设置低一点，因为两个目标既靠近，特征相似度又高的非同一目标的可能性非常低。然后再对目标融合和分离进行判断，主要采用的判断方式是通过分析相邻两帧的所有目标之间的关系。

4. 调用 `draw_line` 函数将相同编号目标的重心链接起来，显示轨迹。

   ![image-20210907162626753](http://ltzunanimage.oss-cn-shenzhen.aliyuncs.com/img/image-20210907162626753.png)

5. 调用 `saveinfo` 保存跟踪结果。

### 12. 将相隔0.12s的矩形框信息存储在一个三维vector中，为转入行为检测模块做准备

调用 `stora` 函数存储当前帧矩形框与 0.12 秒前矩形框的信息。该函数运用三维vecInt \[]\[]\[]实现。第一维表示与当前帧相差的帧数；第二维表示该帧矩形框个数，声明时初始化大小为 100，若某一帧视频中矩形框个数超过 100 会发生越界，后续可进行修改，例如声明时先不确定大小，用 `push_back` 和 `erase` 来将信息存入 `vector` 中；第三维表示该帧该矩形框下的 5 个参数，0~4 分别表示当前矩形框的 x，y，weight，height，label。

![image-20210907162902057](http://ltzunanimage.oss-cn-shenzhen.aliyuncs.com/img/image-20210907162902057.png)

首先根据视频帧率判断出 0.12s 与 0.2s 约为多少帧，因为输出矩形框信息是从 0.2s 后开始输出的，并且每隔 0.12s 输出一次。
`Stora()` 函数首先判断帧数是否大于 detectfrom，是的话将进行以下操作：

![image-20210907162921609](http://ltzunanimage.oss-cn-shenzhen.aliyuncs.com/img/image-20210907162921609.png)

每过一帧，相当于前  k-1 帧变成了前 k 帧，则需要给矩形框数组元素变换位置。

![image-20210907162935891](http://ltzunanimage.oss-cn-shenzhen.aliyuncs.com/img/image-20210907162935891.png)

然后再添加新一帧的信息。

![image-20210907162944581](http://ltzunanimage.oss-cn-shenzhen.aliyuncs.com/img/image-20210907162944581.png)

最后调用二重循环输出当前帧与前四帧矩形框数组信息。

### 13. 先释放上一帧链表，再将当前帧链表赋值给前一帧链表，清零当前帧链表，等待下一帧循环。

![image-20210907163011519](http://ltzunanimage.oss-cn-shenzhen.aliyuncs.com/img/image-20210907163011519.png)

### 14. 转入行为判断模块

```c++
falling(frame);                                            //摔倒行为判断
wander(vecInt[0], frame);                                  //进行徘徊的检测并且在矩形框中显示出状态
```

轨迹匹配完毕后，返回 `objSubstract()` 函数，继续调用 `falling()` 函数进行摔倒行为判断，调用 `wander()` 函数进行徘徊行为判断。

由于这两个函数的代码注释都较为详细，故本文档只做行为判断规则和少部分代码的介绍。

### 15. 摔倒行为判断
“摔倒”的行为判断规则分两部分：倒地前和倒地后。

![image-20210907163120167](http://ltzunanimage.oss-cn-shenzhen.aliyuncs.com/img/image-20210907163120167.png)

倒地前的行为判断采用了四个指标：边界框高度的比值、边界框宽高比的比值、边界框左上角纵坐标高度差以及边界框交并比。其规则是：
1. 从视频开始的第 0.2 秒开始，计算 0.12 秒前和当前帧的边界框高度的比值是否大于阈值或边界框宽高比的比值是否大于阈值、边界框左上角纵坐标差是否大于阈值以及边界框交集与并集的比是否小于阈值。

2. 若是，则给倒地次数+1，累计达到0.36秒则给边界框标记状态为“falling”，接着做倒地后的判断。

3. 若边界框高度的比值小于阈值、边界框左上角纵坐标高度差小于阈值以及边界框交集与并集的比小于阈值，则给站起来次数+1，累计达到 0.36 秒，则重置边界框状态。

之所以采用 0.12 秒的间隔计算人体边界框变化，是因为 0.12 秒能够比较精细地观察到人体行为变化而又不至于太疏漏而漏查或者太频繁而误检；之所以使用边界框高度的比值/边界框宽高比的比值，是因为高度的比值和边界框宽高比的比值能够比较好地避免尺度变化带来的影响。而高度的比值能反映高度变化大小，边界框宽高比的比值能够比较好反映边界框形状；至于边界框的交并比（用于反映人体位置变化情况），是为了进一步的避免行为的误判断。之所以设置为 累计三次则标记状态为 “falling” 是因为倒地前的时间很短暂 （ 0.5 秒到 1 秒不等），而 0.36 秒不会太长也不会太短。

![image-20210907163328142](http://ltzunanimage.oss-cn-shenzhen.aliyuncs.com/img/image-20210907163328142.png)

倒地后的行为判断采用了一个指标：边界框的交并比。其规则是：若边界框状态标记为 “falling”，且边界框的交并比大于阈值（即人体位置变化不大），则每一帧给摔倒次数+1，累计达到 1.92 秒则将边界框状态标记为 “fall” ，即判断行为为 “摔倒”。

实验中，为了反映人体位置变化情况，测试了三种指标，包括：边界框的中心点、边界框的宽高比和边界框的交并比。在人体倒地挣扎起来的过程中，人体动作幅度比较大，但是人体位置变化不大，而相比于边界框的中心点、宽高比，边界框的交并比更能反映这种位置状态。最终采集的实验数据也表明边界框的交并比比其他指标更有效可靠，所以倒地后衡量位置变化的指标用边界框的交并比。至于选择 1.92 秒作为判断阈值，是考虑到实际情况中，人摔倒后站起来要经过一段时间，这样设置一段时间的阈值（可以不是 1.92 秒，稍长或稍短都可以）可以在避免误判断的同时，更符合实际情况。

此外，程序会将摔倒行为判断指标（即存储检测信息）保存在 `action.txt` 中，以供查看程序检测判断的过程。

### 16. 徘徊行为判断

徘徊的行为判断规则也分为两部分：行走和徘徊。

![image-20210907163440302](http://ltzunanimage.oss-cn-shenzhen.aliyuncs.com/img/image-20210907163440302.png)

行走的判断采用了三个指标：边界框右下角横坐标、边界框右下角纵坐标、边界框交并比。其规则是：
1)	从视频开始的第 0.2 秒开始，计算 0.12 秒前和当前帧的边界框右下角横坐标变化或边界框右下角纵坐标变化是否大于设置的阈值，同时边界框的交并比是否小于设置的阈值（即人体位置变化较大）。
2)	若是，则给行走次数+1，累计达到 1.44 秒，则给边界框状态标记为 “walk”。
3)	若边界框右下角横坐标变化小于设置的阈值、边界框右下角纵坐标变化小于设置的阈值，同时边界框的交并比小于设置的阈值，则给停留次数+1，累计达到1.44秒，则重置边界框状态。

![image-20210907163531399](http://ltzunanimage.oss-cn-shenzhen.aliyuncs.com/img/image-20210907163531399.png)

徘徊的判断采用了三个指标：边界框右下角纵坐标、边界框左下角横坐标、

边界框右下角横坐标。其规则 [A.4] 是：最早从视频的第 2.92 秒开始，若边界框状态标记为 “walk”，且初始行走方向是从镜头前往镜头远方行走，在经过 1.44 秒后，边界框状态标记为 “向前走” ，行为状态标记为 “向前走-往回走” ，同时每累计行走 1.44 秒，若边界框右下角纵坐标的差大于阈值，则给向前走次数+1。

同理，若边界框状态标记为 “walk”，初始行走方向是从镜头远方往镜头前行走，在经过 1.44 秒后，边界框状态标记为 “往回走”，行为状态标记为 “往回走-向前走”，同时每累计行走 1.44 秒，若边界框右下角纵坐标的差大于阈值，则给往回走次数+1。

若边界框状态标记为 “walk”，初始行走方向是从镜头左方往镜头右方行走，在经过 1.44 秒后，边界框状态标记为 “向右走”，行为状态标记为 “向右走-向左走”，同时每累计行走 1.44 秒，若边界框右下角横坐标的差大于阈值，则给向右走次数+1。

若边界框状态标记为“walk”，初始行走方向是从镜头右方往镜头左方行走，在经过 1.44 秒后，边界框状态标记为 “向左走”，行为状态标记为 “向左走-向右走” ，同时每累计行走 1.44 秒，若边界框左下角横坐标的差大于阈值，则给向左走次数+1。

若行为状态为 “向前走-往回走” ，往回走的次数至少为1且向前走和往回走的次数相差1，则给 “向前走-往回走” 徘徊次数+1，其他行为状态同理。
当徘徊次数累计大于等于2时，判断为 “徘徊” ，且在边界框上标注 “wander”。

徘徊判断代码说明：

**使用方法：**在每帧视频检测完，并将矩形框信息都存入了对应的变量后，调用 `wander（vector<vector<int>>）`函数。测试所用的资料位于文件夹：`徘徊视频和数据//单人徘徊2.mp4` 和 `allall.txt` 。

**包含文件：**`wander_rect.h`、`wander_rect.cpp`、`wander.cpp`
**wander_rect.h：**

包含了一个类 wander_rect，该类的目的是将 wander 函数中的接口变量与该程序需要用到的一些计数值封装起来。另外该程序用到的函数声明都放在该头文件中。

**wander_rect.cpp:**

 wander_rect类中所包含函数的定义，其中：

`void wander_rect::same_obj_of(wander_rect last_rect)` 函数是在该对象在上一帧中也找到相同label的目标时所调用的，将上一帧中该目标的计数值都赋给当前帧的相同目标，并且当前帧目标的坐标信息不受影响。

`void wander_rect::clear()` 函数是将目标的计数值都置零，这是当目标在上一帧中找不到相同 label 的对象时所调用的函数。

**wander.cpp：**

`void wander(vector<vector<int>> current, IplImage* image)` ：两个参数分别为代表当前帧中所有矩形框信息的二维 vector 和一个图像（大小要跟检测部分一致，目前为360\*200）。在该函数中，有一个以队列的形式保存前 12 帧信息的静态变量 `frame_queue` ,这样可以确保第n帧都可以与第 n-11 帧的信息进行比较（n >11）。该函数的目的是判断出该目标是否处于徘徊状态。

`void walk(vector<wander_rect>& cur_frame, vector<wander_rect>& last_frame)` ：该函数的两个参数都必须使用引用的方式，因为这两个参数都是要保存在 wander 函数中的 frame_queue 中的，而这两个参数的内容会在 walk 函数中发生变化，不使用引用方式的话无法将他们同步到 wander 函数。

其余的函数都是判断行走所用到的条件的计算。

在行人的徘徊判定中，主要是根据行人的向前走和向后走次数的差值来判断的

![image-20210907164613916](http://ltzunanimage.oss-cn-shenzhen.aliyuncs.com/img/image-20210907164613916.png)

但是当行人向同一个方向走了一段时间后，向该方向行走的计数值会增大，导致行人需要往回走一段较长的时间才能判断出徘徊的状态，并且无法连续的判断徘徊。所以设定了计数值的上限，确保了两个方向的计数值差值不会太大。

![image-20210907164624199](http://ltzunanimage.oss-cn-shenzhen.aliyuncs.com/img/image-20210907164624199.png)

由于检测模块效果不佳，影响行为判断效果的测试，故在测试判断效果时可以使用另一种方法，将人体矩形框数据转换为txt文档数据，输入行为判断程序中进行测试：
1. 首先将 `main()` 函数中未注释掉的函数全部注释掉，再将原先注释掉 `video2image()` 函数取消注释，修改参数：其中的第一个路径参数改为测试视频的路径，（如源代码中为`"X:\\车厢内异常行为识别\\视频材料\\有轨电车内摔倒徘徊视频\\视频\\单人徘徊2.mp4"`），数字参数代表隔几帧截取一次（源代码中为 10，可以修改），截取的图片将保存在第二个路径参数所指定的文件夹中（如源代码中为`"X:\\车厢内异常行为识别\\整合\\待截帧\\"`）。

2. 将 `main()` 函数中 “截取图像中矩形框信息” 下的代码取消注释，修改相应路径和i值（i值范围为图片起始名称序号至结束名称序号，如源代码中为 17-401），然后运行程序。程序会弹出图片，操作者可以在图片中对人体进行截图，截出的矩形框数据将保存在 txt 文档中（如源代码中的`"X:\\车厢内异常行为识别\\整合\\allall.txt"`）

3. 在 `wander.cpp` 注释掉的部分有一个 `main` 函数，该函数就是用于单独测试这部分代码的效果，矩形框的数据将从上述txt文档导入（当然，其中的路径需要改成相应的路径），txt 文档的五列数字代表的含义分别为矩形框左上角的 x 和 y, 宽和高，目标编号。

**目前测试效果（单个目标）：**

![image-20210907164827691](http://ltzunanimage.oss-cn-shenzhen.aliyuncs.com/img/image-20210907164827691.png)![image-20210907164834987](http://ltzunanimage.oss-cn-shenzhen.aliyuncs.com/img/image-20210907164834987.png)![image-20210907164843579](http://ltzunanimage.oss-cn-shenzhen.aliyuncs.com/img/image-20210907164843579.png)

测试的时候发现，阈值的设置会影响判断的效果。目前测试所使用的阈值未经过推敲，仅仅是为了验证算法的可行性而设置的值。而且实际的测试视频中的人是一直在行走的，并没有停下，所以在这段视频中也存在误判的情况。如果有徘徊跟停留交错进行的视频进行测试会更好。

目前该部分代码对检测的精度要求比较高，因为一旦有一次没有检测到该目标，它的 label 就会发生变化，那么他在之前的帧进行判断时所保存的信息就会清零，又要重新开始判断。

### 17. 越界行为判断

行人越界算法：

![图 3-18-禁区逗留算法流程图](http://ltzunanimage.oss-cn-shenzhen.aliyuncs.com/img/image-20210907164941294.png)

**算法说明：**

- 预设相关参数有：

  - 视频帧率 f = 25；

  - 判断时间 t 1；

  - 判断概率 p1；

  - 禁区顶点数 m = 4；

  - 禁区顶点坐标；

  - 读入的（x, y, l, h）是由中间层提供外接矩形框的参数。
    $$
    \begin{cases}
    x=upl\_x\\
    y=upl\_y\\
    l=rect\_l\\
    h=rect\_h\\
    \end{cases}
    $$

  - 点C为外接矩形框底边中点。
    $$
    \begin{cases}
    c\cdot x=x+\frac{l}{2}\\
    c\cdot y=y+h
    \end{cases}
    $$

**代码说明：**

**文件：**`cross.cpp`

**包含内容:**

`int main()`
`void crossDectect(vector<int>,IplImage*)`
`bool rayCasting(vector<int>, vector<vector<int>>)`

**main 函数** 是用于测试该部分算法的效果，需要一个测试视频，和一个代表该视频中用于圈出人物的矩形框信息的txt文本。

**rayCasting函数** 是点射法判断该点是否位于我们设置的禁区之内。判断的点是矩形框下边框的中点

![image-20210907165809448](http://ltzunanimage.oss-cn-shenzhen.aliyuncs.com/img/image-20210907165809448.png)

**crossDectect函数** 判断行人是否处于禁区之内，其中的变量 poly 是禁区的区域，该变量中的元素必须为多边形中连续的点；变量judge是一个用于记录在过去的十二帧中行人位于禁区内次数的队列。最后根据落入禁区的频率是否达到阈值来判断行人是否越界。

![image-20210907165841212](http://ltzunanimage.oss-cn-shenzhen.aliyuncs.com/img/image-20210907165841212.png)


当主程序需要进行越界检测时，直接调用 `crossDectect` 函数即可。

### 18. 输出处理后图像（原图+带标号矩形框）

![image-20210907165108063](http://ltzunanimage.oss-cn-shenzhen.aliyuncs.com/img/image-20210907165108063.png)


