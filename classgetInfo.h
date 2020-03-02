#pragma once
/*class getIofo {
public:
	getInfo(int northOrWest) {
		northOrWestFlag = northOrWest;
	}

	//是否在落地点的北边，发出前后移动指令
	//传入参数int CHOutput()
	int NorthHeadingFlag(int northFlag);
	//是否在落地点的西边，发出左右移动指令
	//传入参数int CHOutput()
	int WestHeadingFlag(int westFlag);
private:
	int northOrWestFlag

}*/
//定义一个信号输出类
class CHOutput {
public:
	CHOutput(Point Poi) {//析构函数，传入目标所在的点
		outputPoi = Poi;
	}
	int getInfoTurnLeft(int Poi_x);
	int getInfoForword(int Pox_y);
private:
	int outputPoi;
	
};
