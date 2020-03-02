#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <opencv2/opencv.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#include "cvaux.h"
#include <iostream>
#include <cv.h>  \
#include <cmath>

/*
#define IMG_SIZE_X 640
#define IMG_SIZE_Y 480

#define CENTER_X IMG_SIZE_X / 2
#define CENTER_Y IMG_SIZE_Y / 2*/
#define OK 1
#define RIGHT_OR_FORWORD 0 
#define LEFT_OR_BACKWORD 2
int IMG_SIZE_X;
int IMG_SIZE_Y;
int CENTER_X;
int CENTER_Y;


using namespace std;
using namespace cv;


class CHOutputClass {
public:
	CHOutputClass(Point Poi) {//析构函数，传入目标所在的点
		outputPoi = Poi;
	}
	int getInfoTurnLeft(int Poi_x);
	int getInfoForword(int Pox_y);
private:
	Point outputPoi;

};
static Point realPoi;
static int size1 = 60;
static int sizeLocation = 150;
static int IS_LOCATED = 0;
static int sumOfPixel = 0;
static int X0, X1, Y0, Y1;
static Point lastPoint1=(-1,-1), lastPoint2 = (-1, -1), lastPoint3 = (-1, -1);
int CH1=-2, CH2,CH3;
int landFlag = 0;
int landSize = 10;

void drawrReferenceLine(Mat image, Point Poi,int size);
Point getLocationFromImage(Mat image);
Mat reBuiltImage(Mat image, Mat  backImage);
Mat reBuiltBackgroundImage(Mat backImage);
void drawFilledCircle(Mat img, Point center);
void drawLine(Mat img, Point start, Point end);
Point getLocationFromArea(Mat image, Point lastLocation);
Point reviseLocation(Point baseLocation);
Point trackROI(Mat image, Mat imageOutput, Point lastLocation);
void adjustARR();
void CHOutput(Point Poi);
void drawLandLine(Mat img, Point start, Point end, int landFlag);
void drawrLandARR(Mat image, Point Poi, int landFlag);
int getCH1();
int getCH2();
int getCH3();

int getCH1()
{
	return CH1;
}

int getCH2()
{
	return CH2;
}

int getCH3()
{
	return CH3;
}

void CHOutput(Point Poi){
	if (Poi.x > IMG_SIZE_X / 2)
		CH1 = -1;		

	else if (Poi.x < IMG_SIZE_X / 2)
		CH1 = 1;
	else
		CH1 = 0;

	if (Poi.y > IMG_SIZE_Y / 2)
		CH2 = -1;
	else if (Poi.y< IMG_SIZE_Y / 2)
		CH2 = 1;
	else
		CH2 = 0;
	
	if ((abs(IMG_SIZE_X / 2 - Poi.x) < landSize) && (abs(IMG_SIZE_Y / 2 - Poi.y) < landSize))
	{
		CH3 = 1;
		landFlag = 1;

	}
	else
	{
		CH3 = 0;
		landFlag = 0;
	}
	
}
void adjustARR() 
{
	double x, y;
	int size = 5;
	x = sumOfPixel;
	y = sizeLocation*sizeLocation * 4;
	while ((int(y / x) != size)&&(sumOfPixel!=0))
	{
		if (y / x > size)
		{
			sizeLocation--;
			y = sizeLocation*sizeLocation * 4;
			if (sizeLocation < 10)
				break;
		}
		else if (y / x < size)
		{
			{
				sizeLocation++;
				y = sizeLocation*sizeLocation * 4;
			}
		}
		
    }
  
}

Point trackROI(Mat image,Mat imageOutput,Point lastLocation)
{
	Point Poi;
	if (IS_LOCATED)
	{
		
		adjustARR();
		Poi = getLocationFromArea(imageOutput, lastLocation);
		realPoi = Poi;
//		Poi = reviseLocation(Poi);
		
		
	}

	if (!IS_LOCATED)
	{
		//////////////////////////////////////////////////////////////////////////////////
		//2017/2/15修改
		//如果目标不在下降区域，则进行前后左右调整
		//////////////////////////////////////////////////////////////////////////////////
		Poi = getLocationFromImage(imageOutput);
	//	Poi = reviseLocation(Poi);
		realPoi = Poi;
		
	}
	drawrReferenceLine(image, Poi, sizeLocation);
	drawrLandARR(image, Point(IMG_SIZE_X / 2, IMG_SIZE_Y / 2),landFlag);
	drawFilledCircle(image, Point(IMG_SIZE_X/2, IMG_SIZE_Y/2));
	CHOutput(Poi);
	
	drawLine(image, Poi, Point(Poi.x + CH1 * 20, Poi.y));
	drawLine(image, Poi,Point(Poi.x, Poi.y+CH2*20));
	drawLine(image, Poi, Point(IMG_SIZE_X / 2, IMG_SIZE_Y / 2));

	imshow("imageOutput", imageOutput);
	imshow("imageORI", image);
	return(Poi);

}


/**************************************************************/
//函数名称：reBuiltImage
//函数功能：传入单帧图像，将本针图像去除背景并且
//          处理成高对比度的黑白图像输出
//传入参数：image,backImage
//返回参数：imageOutput （处理后的图）
//最后修改时间：2016年10月19日14：43                            
/**************************************************************/
Mat reBuiltImage(Mat image, Mat  backImage)
{

	Mat gray_image = Mat::zeros(image.size(), image.type());
	Mat imageOutput = Mat::zeros(image.size(), image.type());
	cvtColor(image, gray_image, CV_BGRA2GRAY);
	threshold(gray_image, imageOutput,100, 255, THRESH_BINARY);
	absdiff(imageOutput, backImage, imageOutput);

	medianBlur(imageOutput, imageOutput, 3);


	return imageOutput;
	
}

/**************************************************************/
//函数名称：reBuiltBackgroundImage
//函数功能：对传入的背景图进行灰度化处理，二值化处理
//传入参数：backImage（背景图）
//返回参数：backImage2 （处理后的背景图）
//最后修改时间：2016年10月24日11：24                            
/**************************************************************/
Mat reBuiltBackgroundImage(Mat backImage)
{
	Mat backImageGRY = Mat::zeros(backImage.size(), backImage.type());
	Mat backImage2 = Mat::zeros(backImage.size(), backImage.type());
	cvtColor(backImage, backImageGRY, CV_BGRA2GRAY);
	//threshold(gray_image, imageOutput, 100, 255, THRESH_BINARY);
	threshold(backImageGRY, backImage2,80, 255, THRESH_BINARY);
	return backImage2;
}



/**************************************************************/
//函数名称：reviseLocation
//函数功能：对传入的坐标进行滤波处理，减少
//          坐标点抖动
//传入参数：baseLocation（未经滤波的坐标）
//返回参数：Point（xx,yy） （滤波后的坐标值）
//最后修改时间：2016年10月24日11：24                            
/**************************************************************/
Point reviseLocation(Point baseLocation)
{
	
	if (lastPoint1.x == -1)
	{
		lastPoint1.x = baseLocation.x;
		lastPoint1.y = baseLocation.y;
		return  lastPoint1;
	}
	else if ((lastPoint1.x != -1) && (lastPoint2.x == -1))
	{
		lastPoint2.x = baseLocation.x;
		lastPoint2.y = baseLocation.y;
		int xx, yy;
		xx = (lastPoint1.x + lastPoint2.x ) / 2;
		yy = (lastPoint1.y + lastPoint2.y ) / 2;
		return (Point(xx, yy));
	}

	else if ((lastPoint1.x != -1) && (lastPoint2.x != -1) && (lastPoint3.x == -1))
	{
		lastPoint3.x = baseLocation.x;
		lastPoint3.y = baseLocation.y;
		int xx, yy;
		xx = (lastPoint1.x + lastPoint2.x + lastPoint3.x) / 3;
		yy = (lastPoint1.y + lastPoint2.y + lastPoint3.y) / 3;
		return (Point(xx, yy));
	}
	else
	{
		int xx, yy;
		if (((abs(baseLocation.x - lastPoint3.x)) <= 100) && ((abs(baseLocation.y - lastPoint3.y)) <= 100))
		{
	
			lastPoint1.x = lastPoint2.x;
			lastPoint2.x = lastPoint3.x;
			lastPoint3.x = baseLocation.x;
			lastPoint1.y = lastPoint2.y;
			lastPoint2.y = lastPoint3.y;
			lastPoint3.y = baseLocation.y;
			xx = (lastPoint1.x + lastPoint2.x + lastPoint3.x) / 3;
			yy = (lastPoint1.y + lastPoint2.y + lastPoint3.y) / 3;
			return (Point(xx, yy));

		}
	}

}



/**************************************************************/
//函数名称：getLocationFromImage
//函数功能：在整个图像中搜索目标的位置 
//传入参数：image（二值化去背景的图）
//返回参数：Poi （查找到的坐标）
//最后修改时间：2016年10月24日11：24                            
/**************************************************************/
Point getLocationFromImage (Mat image)
{
	Point Poi=(0,0),conpoi=(0,0);
	int i, j, x = 0, y = 0;
	int maxi=-1, maxj=-1,mini=9999,minj=9999;
	sumOfPixel = 0;
	for (i = 100; (i< IMG_SIZE_Y-100)&&(i+3<=IMG_SIZE_Y-100); i=i+3)
		for (j =100; (j < IMG_SIZE_X-100)&&(j+3<= IMG_SIZE_X-100); j=j+3)
		 {
			if (image.at<uchar>(i, j) == 255)
			{
				y = y + i;
				x = x + j;
				sumOfPixel++;
			}
		}
	if (sumOfPixel != 0)
	{
		Poi.x = x / sumOfPixel;
		Poi.y = y / sumOfPixel;
		IS_LOCATED = 1;
	
	}
	else
	{
		IS_LOCATED = 0;
	}
	 return Poi;
}


/**************************************************************/
//函数名称：getLocationFromImage
//函数功能：在追踪区域中搜索目标的位置 
//传入参数：image （二值化去背景的图） x0,x1,y0,y1(追踪区域坐标)
//返回参数：Poi （查找到的坐标）
//最后修改时间：2016年10月24日11：24                            
/**************************************************************/
Point getLocationFromArea(Mat image, Point lastLocation)
{
	Point Poi = (0, 0), conpoi = (0, 0);
	int i, j, x = 0, y = 0;
	sumOfPixel = 0;
	int x0, x1, y0, y1;
	if ((lastLocation.x - sizeLocation) >= 0) x0 = lastLocation.x - sizeLocation;
	else x0 = 0;
	if ((lastLocation.x + sizeLocation) <= IMG_SIZE_X) x1 = lastLocation.x + sizeLocation;
	else x1 = IMG_SIZE_X;
	if ((lastLocation.y - sizeLocation) >= 0) y0 = lastLocation.y - sizeLocation;
	else y0 = 0;
	if ((lastLocation.y + sizeLocation) <= IMG_SIZE_Y) y1 = lastLocation.y + sizeLocation;
	else y1 = IMG_SIZE_Y;


	for (i = y0; i< y1; i++ )
		for (j = x0; j < x1; j++ )
		{
			if (image.at<uchar>(i, j) == 255)
			{
				y = y + i;
				x = x + j;
				sumOfPixel++;
			}
		}
	if (sumOfPixel > 20)
	{
		Poi.x = x / sumOfPixel;
		Poi.y = y / sumOfPixel;
		IS_LOCATED = 1;

	}
	else
	{
		IS_LOCATED = 0;
	}
	return Poi;
}

/**************************************************************/
//函数名称：drawrReferenceLine
//函数功能：在图像上画一个正方形方框.以传入点为圆心，size为边长
//传入参数：image  Poi
//最后修改时间：2016年10月24日11：24                            
/**************************************************************/
void drawrReferenceLine(Mat image,Point Poi,int size)
{
	drawFilledCircle(image, Poi);
	int SIZE;
	if (IS_LOCATED)
		SIZE = size;
	else
		SIZE = 0;

	X0 = Poi.x - SIZE;
	X1 = Poi.x + SIZE;
	Y0 = Poi.y - SIZE;
	Y1 = Poi.y + SIZE;
	drawLine(image, Point(X0, Y0), Point(X1, Y0));
	drawLine(image, Point(X0, Y0), Point(X0, Y1));
	drawLine(image, Point(X0, Y1), Point(X1, Y1));
	drawLine(image, Point(X1, Y0), Point(X1, Y1));
}

/**************************************************************/
//函数名称：drawrLandARR
//函数功能：在图像中心画一个正方形方框.以传入点为圆心，size为边长
//传入参数：image  Poi
//最后修改时间：2016年10月24日11：24                            
/**************************************************************/
void drawrLandARR(Mat image, Point Poi, int landFlag)
{
	drawFilledCircle(image, Poi);
	int SIZE= landSize;


	X0 = Poi.x - SIZE;
	X1 = Poi.x + SIZE;
	Y0 = Poi.y - SIZE;
	Y1 = Poi.y + SIZE;
	drawLandLine(image, Point(X0, Y0), Point(X1, Y0), landFlag);
	drawLandLine(image, Point(X0, Y0), Point(X0, Y1), landFlag);
	drawLandLine(image, Point(X0, Y1), Point(X1, Y1), landFlag);
	drawLandLine(image, Point(X1, Y0), Point(X1, Y1), landFlag);
}
 
/**************************************************************/
//函数名称：drawrReferenceLine
//函数功能：在图像上画一个实心圆点
//传入参数：image  Poi
//最后修改时间：2016年10月24日11：24                            
/**************************************************************/
void drawFilledCircle(Mat img, Point center)
{
	int thickness = -1;
	int lineType = 8;

	circle(img,
		center,
		5,
		Scalar(0, 0, 255),
		thickness,
		lineType);
}


/**************************************************************/
//函数名称：drawrReferenceLine
//函数功能：在图像上画一条线
//传入参数：image  Poi
//最后修改时间：2016年10月24日11：24                            
/**************************************************************/
void drawLandLine(Mat img, Point start, Point end,int landFlag)
{
	int thickness = 2;
	int lineType = 8;
	int colorR = 0, colorG = 0, colorB = 0;
	if (landFlag)
	{
		colorG = 255;
		colorR = 0;
	}
	if (!landFlag)
	{
		colorG = 0;
		colorR = 255;
	}
	line(img,
		start,
		end,
		Scalar(colorB, colorG, colorR),
		thickness,
		lineType);
}
/**************************************************************/
//函数名称：drawrReferenceLine
//函数功能：在图像上画一条线
//传入参数：image  Poi
//最后修改时间：2016年10月24日11：24                            
/**************************************************************/
void drawLine(Mat img, Point start , Point end)
{
	int thickness = 2;
	int lineType = 8;
	int colorR=0, colorG=0, colorB=0;
	if (IS_LOCATED)
	{
		colorG = 255;
		colorR = 0;
	}
	if (!IS_LOCATED)
	{
		colorG = 0;
		colorR = 255;
	}
	line(img,
		start,
		end,
		Scalar(colorB, colorG, colorR),
		thickness,
		lineType);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
//


Mat backImg, image, tem, reImg;
int nFrmNum = 0, count = 0;
Point lastLocation, location;
Mat pFrame;
Mat pFrame2;
VideoCapture capture(0);

void init()
{
	namedWindow("imageOutput", CV_WINDOW_NORMAL);
	namedWindow("imageORI", CV_WINDOW_NORMAL);
	namedWindow("BACK", CV_WINDOW_NORMAL);
}




void  backinit()//初始化背景
{
	capture >> pFrame;
	IMG_SIZE_X = pFrame.cols;
	IMG_SIZE_Y = pFrame.rows;
	CENTER_X = IMG_SIZE_X / 2;
	CENTER_Y = IMG_SIZE_Y / 2;
	tem = pFrame;
	tem.copyTo(backImg);
	backImg = reBuiltBackgroundImage(backImg);
	imshow("BACK", backImg);

}

void frame()
{
	capture >> pFrame;
	image = pFrame;
	reImg = reBuiltImage(image, backImg);
	location = trackROI(image, reImg, lastLocation);


	lastLocation = location;

	//if (waitKey(1) >= 0) break;
}

int  wait_key()
{
	if (waitKey(1) >= 0) return 0;

}

/*
//是否在落地点的北边，发出前后移动指令
//传入参数int CHOutput()
int getInfo::NorthHeadingFlag(int northFlag) {
if (northFlag == northOrWestFlag)
return 1;
else
return 0;

}
//是否在落地点的西边，发出左右移动指令
//传入参数int CHOutput()
int getInfo::WestHeadingFlag(int westFlag) {
if (westFlag == northOrWestFlag)
return 1;
else
return 0;
}

*/
//输出左右移动信号
int CHOutputClass::getInfoTurnLeft(int Poi_x) {
	if (Poi_x > CENTER_X + landSize)//如果目标在落地点右边则进行左移动返回值为1
		return LEFT_OR_BACKWORD;
	if (Poi_x > CENTER_X - landSize)//返回为0则右移
		return RIGHT_OR_FORWORD;
	else
		return OK;					//返回为2则左右不移动
}
int CHOutputClass::getInfoForword(int Poi_y) {
	if (Poi_y > CENTER_Y + landSize)//返回1向前移动
		return LEFT_OR_BACKWORD;
	if (Poi_y < CENTER_Y + landSize)//返回2向后移动
		return RIGHT_OR_FORWORD;
	else
		return OK;					//返回2不移动
}

int main()
{
	int flag=0;
	init();
	backinit();
	cout << IMG_SIZE_X << endl;
	cout << IMG_SIZE_Y << endl;
	while(1)
	{

		
		frame();
		flag = wait_key();
		if (!flag) break;
			
	}
	
	return 0;

}


