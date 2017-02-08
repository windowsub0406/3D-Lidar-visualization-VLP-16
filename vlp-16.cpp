#include <opencv2\imgcodecs.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/viz.hpp>
#include <winsock2.h>
#include <iostream>
#include <thread>
//#include "Velo.h"

#define PI 3.1415926535
#pragma comment (lib,"ws2_32.lib")
#define BUFFER_SIZE 1206
#define thread_size 5

//#ifndef _DEBUG
//#pragma comment(lib, "./opencv_viz310.lib")
//#endif

using namespace cv;

WSADATA wsaData;
SOCKET ServerSocket;
SOCKADDR_IN ServerInfo;
SOCKADDR_IN FromClient;
bool UDPinit();
void UDPbind();
void UDPrecData(int no);
void UDPdisConnect();
short Portnum = 2368;
char Buffer[thread_size][BUFFER_SIZE];
double Aziarray[24];
void XYZcloud(int no);
float CalDist(int d, int no);
double CalAzimuth(int n, int k);
void AzimuthArray(int no);
void Distarray();
void Timestamp();
void countProc(int no);
int colordata[3];
std::vector<Point3f> depthPts;
std::vector<Vec3b> colorPts;
int m_pseudoColorLUT[256][3]; ///< RGB pseudo color
void MakePseudoColorLUT(int val);
void cvtPseudoColorImage(float dist);
int main()
{
	//VideoCapture capture(2);

	depthPts.clear();
	colorPts.clear();
	viz::Viz3d plot3d("Coordinate Frame");
	plot3d.showWidget("Coordinate Widget", viz::WCoordinateSystem());
	UDPinit();
	UDPbind();
	int check = 0;
	while (1){// (!plot3d.wasStopped()){
		//		Mat frame;
		//		capture >> frame;

		for (int a = 0; a<15; a++)
		{
			std::thread thread1(&countProc, 0);
			std::thread thread2(&countProc, 1);
			std::thread thread3(&countProc, 2);
			std::thread thread4(&countProc, 3);
			std::thread thread5(&countProc, 4);

			//	velo.UDPrecData(a);	
			//	velo.XYZcloud(a);
			//	//velo.Timestamp();			

			thread1.join();
			thread2.join();
			thread3.join();
			thread4.join();
			thread5.join();
			while (depthPts.size() != colorPts.size())
			{
				if (depthPts.size() > colorPts.size())
				{
					depthPts.erase(depthPts.end() - 1);
				}
				if (depthPts.size() < colorPts.size())
				{
					colorPts.erase(colorPts.end() - 1);
				}
			}
		}
		viz::WCloud cloud_widget = viz::WCloud(depthPts, colorPts);
		cloud_widget.setRenderingProperty(cv::viz::POINT_SIZE, 2);

		plot3d.showWidget("ref_cloud", cloud_widget);
		//check++;
		//if(check>50)
		//{

		//	check =0;
		depthPts.clear();
		colorPts.clear();
		//}

		/*int64 t = getTickCount();
		t = getTickCount() - t;
		printf("Time elapsed: %fms\n", t * 1000 / getTickFrequency());
		*/
		//	resize(frame,frame,Size(480,360));
		//	imshow("original",frame);
		if (waitKey(1) == 27) return 1;
		plot3d.spinOnce(1, true);
	}
	UDPdisConnect();
	return 0;
}

void countProc(int no)
{
	UDPrecData(no);
	XYZcloud(no);
}

bool UDPinit()
{
	if (WSAStartup(0x202, &wsaData) == SOCKET_ERROR)
	{
		std::cout << "WinSock ÃÊ±âÈ­ºÎºÐ¿¡¼­ ¹®Á¦ ¹ß»ý " << std::endl;
		WSACleanup();
	}

	memset(&ServerInfo, 0, sizeof(ServerInfo));
	memset(&FromClient, 0, sizeof(FromClient));
	memset(Buffer[0], 0, BUFFER_SIZE);
	memset(Buffer[1], 0, BUFFER_SIZE);
	memset(Buffer[2], 0, BUFFER_SIZE);
	memset(Buffer[3], 0, BUFFER_SIZE);
	memset(Buffer[4], 0, BUFFER_SIZE);
	return true;
}

void UDPbind()
{
	ServerInfo.sin_family = AF_INET;
	ServerInfo.sin_addr.s_addr = inet_addr("192.168.1.77");
	ServerInfo.sin_port = htons(Portnum);

	// ¼ÒÄÏ »ý¼º
	ServerSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (ServerSocket == INVALID_SOCKET)
	{
		std::cout << "¼ÒÄÏÀ» »ý¼ºÇÒ¼ö ¾ø½À´Ï´Ù." << std::endl;
		UDPdisConnect();
		exit(0);
	}

	if (::bind(ServerSocket, (struct sockaddr*)&ServerInfo,
		sizeof(ServerInfo)) == SOCKET_ERROR)
	{
		std::cout << "Can't bind" << std::endl;
		UDPdisConnect();
		exit(0);
	}
}

void UDPrecData(int no)
{
	int FromClient_Size;
	int Recv_Size;

	FromClient_Size = sizeof(FromClient);

	Recv_Size = recvfrom(ServerSocket, Buffer[no], sizeof(Buffer[no]), 0,
		(struct sockaddr*) &FromClient, &FromClient_Size);

	if (Recv_Size < 0)
	{
		std::cout << "recvfrom() error!" << std::endl;
		exit(0);
	}

	//  cout<<"client ip is : "<<inet_ntoa( FromClient.sin_addr )<<endl;
	//  cout<<"Packet data is : " <<Buffer <<endl;	
	//cout<<(float)CalAzimuth(2)<<endl;
}

void UDPdisConnect()
{
	closesocket(ServerSocket);
	WSACleanup();
}

float CalDist(int d, int no)
{
	double Dist1 = unsigned int(Buffer[no][d + 1]);
	if (Dist1 >= 4294967040)
		Dist1 = Dist1 - 4294967040;
	double Dist2 = unsigned int(Buffer[no][d]);
	if (Dist2 >= 4294967040)
		Dist2 = Dist2 - 4294967040;
	double ka = Dist1 / 16;
	double kb = Dist1 - ka * 16;
	double kc = Dist2 / 16;
	double kd = Dist2 - kc * 16;

	float Caldist = (ka * 16 * 16 * 16 + kb * 16 * 16 + kc * 16 + kd) * 2 / 1000;
	//cout<<hex<<(int)Buffer[8]<<endl;
	return Caldist;
}

double CalAzimuth(int n, int k)
{
	double Azimuth1 = unsigned int(Buffer[k][n + 1]);
	if (Azimuth1 >= 4294967040)
		Azimuth1 = Azimuth1 - 4294967040;
	double Azimuth2 = unsigned int(Buffer[k][n]);
	if (Azimuth2 >= 4294967040)
		Azimuth2 = Azimuth2 - 4294967040;
	double ka = Azimuth1 / 16;
	double kb = Azimuth1 - ka * 16;
	double kc = Azimuth2 / 16;
	double kd = Azimuth2 - kc * 16;

	double Calazi = (double)((ka * 16 * 16 * 16 + kb * 16 * 16 + kc * 16 + kd) / 100);
	//cout<<(double)Calazi<<endl;
	return Calazi;
}

void AzimuthArray(int no)
{
	for (int i = 0, n = 2; n<1150; n += 100, i += 2)
	{
		Aziarray[i] = (float)CalAzimuth(n, no);
	}

	//Azimuth interpolation
	for (int a = 1; a<22; a += 2)
	{
		if ((float)Aziarray[a - 1] <= (float)Aziarray[a + 1])
			Aziarray[a] = (float)(Aziarray[a - 1] + (Aziarray[a + 1] - Aziarray[a - 1]) / 2);
		if ((float)Aziarray[a - 1] >(float)Aziarray[a + 1])
			Aziarray[a] = (float)((360 - Aziarray[a - 1] + Aziarray[a + 1]) / 2);
	}
	if ((float)Aziarray[20] <= (float)Aziarray[22])
	{
		Aziarray[23] = (float)(Aziarray[22] + (Aziarray[22] - Aziarray[20]) / 2);
		if (Aziarray[23] >= 360)
			Aziarray[23] = Aziarray[23] - 360;
	}
	if ((float)Aziarray[20] > (float)Aziarray[22])
		Aziarray[23] = (float)(Aziarray[22] + (360 + Aziarray[22] - Aziarray[20]) / 2);

	//for(int k=0;k<24;k++)
	//{
	//	cout<<k<<"    "<<Aziarray[k]<<endl;
	//}
}

void XYZcloud(int no)
{
	int ChID[16] = { -15, 1, -13, 3, -11, 5, -9, 7, -7, 9, -5, 11, -3, 13, -1, 15 };
	float z;
	AzimuthArray(no);
	for (int n = 0; n<24; n++)
	{
		double azi = Aziarray[n];
		//cout<<azi<<endl;
		if (n % 2 == 0)
		{
			for (int a = 4, i = 0; a<50; a += 3, i++)
			{
				int channel = ChID[i];
				//cout<<channel<<endl;
				float caldist = CalDist(50 * n + a, no);
				//cout<<caldist<<endl;
				//cout<<CalDist(a)<<endl;
				//if(azi <=90 || azi >=270)
				z = caldist*cos(channel*PI / 180)*cos(azi*PI / 180);
				//else if(azi > 90 && azi < 270)
				//z = - caldist*cos(channel*PI/180)*cos(azi*PI/180);

				Point3f temp_point((float)-caldist*cos(channel*PI / 180)*sin(azi*PI / 180) * 10,
					(float)caldist*sin(channel*PI / 180) * 10, z * 10);
				depthPts.push_back(temp_point);
				int a1 = colordata[0];
				int b1 = colordata[1];
				int c1 = colordata[2];
				//std::cout<<caldist<<std::endl;
				colorPts.push_back(Vec3b(a1, b1, c1));
				//colorPts.push_back((int)(caldist*255/7));
				//cout<< "x : "<< (float)caldist*cos(channel*PI/180)*sin(azi*PI/180)<<"  y : "<< (float)caldist*sin(channel*PI/180)<<"  z :  "<<z<<endl;

			}
		}
		else if (n % 2 == 1)
		{
			for (int b = 2, j = 0; b<48; b += 3, j++)
			{
				int channel = ChID[j];

				float caldist = CalDist(50 * n + b, no);

				cvtPseudoColorImage(caldist);
				//std::cout<<colordata[0]<<std::endl;
				z = caldist*cos(channel*PI / 180)*cos(azi*PI / 180);

				Point3f temp_point((float)-caldist*cos(channel*PI / 180)*sin(azi*PI / 180) * 10,
					(float)caldist*sin(channel*PI / 180) * 10, z * 10);
				depthPts.push_back(temp_point);
				int a1 = colordata[0];
				int b1 = colordata[1];
				int c1 = colordata[2];
				//std::cout<<caldist<<std::endl;
				colorPts.push_back(Vec3b(a1, b1, c1));
				//colorPts.push_back((int)(caldist*255/7));
				//cout<< "x : "<< (float)caldist*cos(channel*PI/180)*sin(azi*PI/180)<<"  y : "<< (float)caldist*sin(channel*PI/180)<<"  z :  "<<z<<endl;

				//cout<< (float)caldist*cos(channel*PI/180)<<endl;
				//cout<<CalDist(b)<<endl;
			}
		}
		//cout<<azi<<endl;
	}
}

void MakePseudoColorLUT(int val)
{
	int b = 125;
	int g = 0;
	int r = 0;

	int mode = val / 20;
	switch (mode)
	{
	case 0:
		b = 255 * val / 20;
		break;
	case 1:
		b = 255;
		g = 255 * val / 20;
		break;
	case 2:
		b = 255 - (255 * val / 20);
		g = 255;
		break;
	case 3:
		b = 0;
		g = 255;
		r = 255 * val / 20;
		break;
	case 4:
		b = 0;
		g = 0;
		r = 255 * val / 20;
		break;
	default:
		b = 0;
		g = 0;
		r = 0;
	}

	m_pseudoColorLUT[val][0] = b;
	m_pseudoColorLUT[val][1] = g;
	m_pseudoColorLUT[val][2] = r;
}

void cvtPseudoColorImage(float dist)
{
	int val = dist * 10;
	//std::cout<<val<<std::endl;
	MakePseudoColorLUT(val);
	if (val != 0)
	{
		// std::cout<<val<<std::endl;
		colordata[0] = m_pseudoColorLUT[val][0]; //b
		colordata[1] = m_pseudoColorLUT[val][1]; //g
		colordata[2] = m_pseudoColorLUT[val][2]; //r
		//std::cout<<colordata[0]<<std::endl;
	}
}
