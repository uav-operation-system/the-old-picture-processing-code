#pragma once
/*class getIofo {
public:
	getInfo(int northOrWest) {
		northOrWestFlag = northOrWest;
	}

	//�Ƿ�����ص�ı��ߣ�����ǰ���ƶ�ָ��
	//�������int CHOutput()
	int NorthHeadingFlag(int northFlag);
	//�Ƿ�����ص�����ߣ����������ƶ�ָ��
	//�������int CHOutput()
	int WestHeadingFlag(int westFlag);
private:
	int northOrWestFlag

}*/
//����һ���ź������
class CHOutput {
public:
	CHOutput(Point Poi) {//��������������Ŀ�����ڵĵ�
		outputPoi = Poi;
	}
	int getInfoTurnLeft(int Poi_x);
	int getInfoForword(int Pox_y);
private:
	int outputPoi;
	
};
