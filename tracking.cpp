#include "tracking.h"

//画目标轨迹
void draw_line(IplImage *image, bboxlist *t)
{
	bboxlist *t1 = t;
	while (t1 != NULL)   //描述目标运动的轨迹
	{
		if (t1->next != NULL)
		{
			cvLine(image, cvPoint(t1->x + t1->width / 2, t1->y + t1->height / 2), cvPoint(t1->next->x + t1->width / 2, t1->next->y + t1->height / 2), cvScalar(255, 0, 255), 2, 8, 0);
		}
		t1 = t1->next;
	}
}

//计算质心距离
int cal_dist(int x1, int y1, int x2, int y2)
{
	int distance;
	double res;
	res = (x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2);
	distance = (int)(sqrt(res));
	return distance;
}

//清除链接
void release_obj_link(tracks *link)
{
	tracks *p = link, *q = NULL;
	bboxlist *t1 = NULL, *t2 = NULL;
	while (p != NULL)
	{
		q = p;
		p = p->next;
		t1 = q->bbox;
		q->bbox = NULL;
		while (t1 != NULL)
		{
			t2 = t1;
			t1 = t1->next;
			delete t2;
		}
		delete q;
	}
}

//保存因融合丢失的轨迹
void restore(tracks *prehead)
{
	tracks *t1, *t2;
	t1 = losstrack->next;
	while (t1)
	{
		t1 = t1->next;
	}
	t1 = prehead;
	t2 = prehead->next;
	t1->next = NULL;
	prehead = t2;
}

//取出因融合丢失的轨迹
void takeout(tracks *prehead)
{
	tracks *t1, *t2;
	t1 = losstrack;
	t2 = prehead->next;
	while (t2)
	{
		t2 = t2->next;
	}
	t2 = t1->next;
}

//将连通域各个目标串起来
void buildlist(int x, int y, int wid, int hei, tracks *head)     //将当前帧所有目标串起来
{
	tracks *p = head, *t = NULL;
	while (p->next != NULL)
	{
		p = p->next;
	}
	t = new tracks();
	t->bbox = new bboxlist();
	t->label = -1;											//初始化为-1 ，如果目标没有匹配上，则标签为-1输出
	t->age = 1;
	t->combineflag = 0;
	t->divflag = 0;

	t->bbox->x = x;
	t->bbox->y = y;
	t->coorx = x + wid / 2;
	t->coory = y + hei / 2;
	t->next = NULL;

	t->bbox->width = wid;
	t->bbox->height = hei;
	t->bbox->next = NULL;

	p->next = t;
}

//匹配轨迹
void assigntrack(IplImage* image)
{
	int dist; double similarity;
	tracks *q = NULL, *p = NULL;
	CvFont font1;
	cvInitFont(&font1, CV_FONT_HERSHEY_TRIPLEX, 1, 1);
	if (nFrmNum > 1)
	{
		p = curr_head->next;
		while (p != NULL)
		{
			cvRectangle(image, cvPoint(p->coorx - 1, p->coory - 1), cvPoint(p->coorx + 1, p->coory + 1), cvScalar(0, 0, 255), 4, 8, 0);
			p = p->next;
		}

		q = NULL, p = NULL;
		q = curr_head->next;
		while (q)
		{
			for (p = prev_head->next; p; p = p->next)
			{
				dist = cal_dist(p->coorx, p->coory, q->coorx, q->coory);
				if (dist <= MINIDIST)							//大致根据距离进行筛选，
				{												//和所有原目标距离过大就认为是新目标
					if (p->label == -1)
					{
						label_num++;
						p->label = label_num;
					}
					break;
				}
			}
			if (p == NULL)
			{
				label_num++;
				q->label = label_num;
			}
			q = q->next;
		}

		for (q = curr_head->next; q; q = q->next)
		{
			for (p = prev_head->next; p; p = p->next)
			{
				//similarity = 0.7;
				similarity = Similarity(q->imgPrint, p->imgPrint);
				//cout << similarity << endl;
				dist = cal_dist(p->coorx, p->coory, q->coorx, q->coory);
				if (dist <= MINIDIST && similarity > 0.6)
				{
					q->label = p->label;
					//q->age += p->age;
					q->bbox->next = p->bbox;
					p->bbox = NULL;
					break;
				}
			}
			draw_line(image, q->bbox);

			cvRectangle(image, cvPoint(q->bbox->x, q->bbox->y), cvPoint(q->bbox->x + q->bbox->width, q->bbox->y + q->bbox->height), cvScalar(0, 255, 0));

			char buf[20];
			if (outputlabel == q->label && outputlabel != -1)
			{
				saveinfo(q->bbox);
				sprintf_s(buf, 20, "you choose %d", q->label);
			}
			else
			{
				sprintf_s(buf, 20, "%d", q->label);
			}
			//cout << q->label << endl;

			cvPutText(image, buf, cvPoint(q->bbox->x, q->bbox->y + 20), &font1, CV_RGB(255, 255, 0));
		}
		release_obj_link(prev_head->next);
		prev_head->next = curr_head->next;
		curr_head->next = NULL;
	}
	//char FrmNum[30];
	//sprintf_s(FrmNum, 30, "Frame No: %d", nFrmNum);
	//cvPutText(image, FrmNum, cvPoint(1, 25), &font2, CV_RGB(0, 0, 255));
}


