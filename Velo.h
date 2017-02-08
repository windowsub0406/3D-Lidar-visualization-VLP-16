#include <opencv2\imgcodecs.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <array>
#include <string>
#include <stdio.h>

//#define PI 3.1415926535
#define BUFFER_SIZE 1206

using namespace cv;
using namespace std;

struct sPoint {
	double x, y, z;
};
struct sPlane {
	double a,b,c,d;
};

class Velo{
private:
	double CalAzimuth(int n);
	void AzimuthArray();
	float CalDist(int d);
	array<double, 24> Aziarray;
	void MakePseudoColorLUT(int val);
	void MakePseudoColorLUT_2d(int val);
	void cvtPseudoColorImage(float dist);
	void cvtPseudoColorImage_2d(float dist);
	int colordata[3];
	int colordata_2d[3];
	int m_pseudoColorLUT[256][3]; ///< RGB pseudo color
	int m_pseudoColorLUT_2d[256][3]; ///< RGB pseudo color
public:
	Velo();
	~Velo();
	vector<Point3f> depthPts;
	vector<Vec3b> colorPts;
	Mat XYZcloud(Mat image);
	char Buffer[ BUFFER_SIZE ];
	string intToString(int n);
};
