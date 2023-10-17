// bmp_editor.cpp : Defines the entry point for the console application.
//
// bmp_editor.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <stdio.h>
#include <windows.h>
#include <time.h>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>



using namespace std;

#pragma pack(2)

//��������� ����� BMP 
typedef struct tBITMAPFILEHEADER
{
	WORD bfType;
	DWORD bfSize;
	WORD bfReserved1;
	WORD bfReserved2;
	DWORD bfOffBits;
}sFileHead;

//��������� BitMap's
typedef struct tBITMAPINFOHEADER
{
	DWORD biSize;
	LONG biWidth;
	LONG biHeight;
	WORD biPlanes;
	WORD biBitCount;
	DWORD biCompression;
	DWORD biSizeImage;
	LONG biXPelsPerMeter;
	LONG biYPelsPerMeter;
	DWORD biClrUsed;
	DWORD biClrImportant;
}sInfoHead;
//��������� ������������ ����� Stupak 
struct MyFileHead
{
	WORD fileTypeIdentifier;
	DWORD fileSizeInBytes;
	WORD headerSizeInBytes;
	DWORD rasterSizeInBytes;
	BYTE colorDepth;
	DWORD imageWidthInPixels;
	WORD fileOffset;
};

sFileHead FileHead;
sInfoHead InfoHead;

//�������
struct Color
{
	BYTE blue;
	BYTE green;
	BYTE red;

	Color() {}

	Color(int green, int blue, int red) {
		this->green = green;
		this->blue = blue;
		this->red = red;
	}
};

//������ 1-�� �������
int pixel_size = sizeof(Color);


//1 - BMP, 2 - CMP
int img_type = 0;

//�������� �����������
std::vector<Color> src_image;
//�������������� �����������
std::vector<Color> dst_image;
//��������������� �����������
std::vector<Color> ftr_image;

std::vector<Color> brd_image;
//������ �����������
int width = 0;
int height = 0;

//������� ��������� BMP �����
void ShowBMPHeaders(tBITMAPFILEHEADER fh, tBITMAPINFOHEADER ih)
{
	cout << "Type: " << (CHAR)fh.bfType << endl;
	cout << "Size: " << fh.bfSize << endl;
	cout << "Shift of bits: " << fh.bfOffBits << endl;
	cout << "Width: " << ih.biWidth << endl;
	cout << "Height: " << ih.biHeight << endl;
	cout << "Planes: " << ih.biPlanes << endl;
	cout << "BitCount: " << ih.biBitCount << endl;
	cout << "Compression: " << ih.biCompression << endl;
}

//������� ��� �������� �����������
bool OpenImage(string path)
{
	ifstream img_file;
	Color temp;
	char buf[3];

	//������� ���� �� ������
	img_file.open(path.c_str(), ios::in | ios::binary);
	if (!img_file)
	{
		cout << "File isn`t open!" << endl;
		return false;
	}

	//������� ��������� BMP
	img_file.read((char*)&FileHead, sizeof(FileHead));
	img_file.read((char*)&InfoHead, sizeof(InfoHead));

	img_type = 1;
	ShowBMPHeaders(FileHead, InfoHead);
	//��������� ����� � ������ �����������
	width = InfoHead.biWidth;
	height = InfoHead.biHeight;


	//�������� ����� ��� �����������
	src_image.resize(width * height);

	int i, j;
	for (i = 0; i < height; i++)
	{
		for (j = 0; j < width; j++)
		{
			img_file.read((char*)&temp, pixel_size);
			src_image[i * width + j] = temp;
		}
		//�������� ���� ������������ ��� ������������ �� �������� �����
		img_file.read((char*)buf, j % 4);
	}
	img_file.close();

	return true;
}

//������� ��� ���������� ����������� c ���� �����������
bool SaveStupakImage(const string path, const std::vector<Color>& image, tBITMAPFILEHEADER fh, tBITMAPINFOHEADER ih)
{
	ofstream img_file;
	char buf[3];

	//������� ���� �� ������
	img_file.open(path.c_str(), ios::out | ios::binary);
	if (!img_file)
	{
		return false;
	}

	MyFileHead fHead
	{
		fh.bfType, // fileTypeIdentifier
		fh.bfSize + sizeof(MyFileHead) - sizeof(sFileHead) - sizeof(sInfoHead), // fileSizeInBytes
		sizeof(MyFileHead), // headerSizeInBytes
		ih.biSizeImage, // rasterSizeInBytes
		ih.biBitCount, // colorDepth
		ih.biWidth, // imageWidthInPixels
		fh.bfSize + sizeof(MyFileHead) - sizeof(sFileHead) - sizeof(sInfoHead) + 1, //fileOffset
	};

	img_file.write((char*)&fHead, sizeof(MyFileHead));

	//������ ����
	int i, j;
	for (i = 0; i < height; i++)
	{
		for (j = 0; j < width; j++)
		{
			img_file.write((char*)&image[i * width + j], pixel_size);
		}
		img_file.write((char*)buf, j % 4);
	}
	img_file.close();

	return true;
}

//������� � ������� ��������� stupak �����
void OpenAndShowStupakHeaders(string path)
{
	ifstream img_file;
	Color temp;
	char buf[3];

	//������� ���� �� ������
	img_file.open(path.c_str(), ios::in | ios::binary);
	if (!img_file)
	{
		cout << "File isn`t open!" << endl;
		return;
	}

	MyFileHead fHead;

	//������� ��������� BMP
	img_file.read((char*)&fHead, sizeof(MyFileHead));

	cout << "\n\nFile type identifier: " << (CHAR)fHead.fileTypeIdentifier << "\n"
		<< "File offset: " << fHead.fileOffset << "\n"
		<< "File size in bytes: " << fHead.fileSizeInBytes << "\n"
		<< "Header size in bytes: " << fHead.headerSizeInBytes << "\n"
		<< "Raster size in bytes: " << fHead.rasterSizeInBytes << "\n"
		<< "Color depth: " << (int)fHead.colorDepth << "\n"
		<< "Image width in pixels: " << fHead.imageWidthInPixels << "\n";
}

//������� ��� ���������� �����������
bool SaveImage(string path)
{
	ofstream img_file;
	char buf[3];

	//������� ���� �� ������
	img_file.open(path.c_str(), ios::out | ios::binary);
	if (!img_file)
	{
		return false;
	}

	img_file.write((char*)&FileHead, sizeof(FileHead));
	img_file.write((char*)&InfoHead, sizeof(InfoHead));

	//����������� �� ��������� � �������������� �����������
	dst_image.resize(width * height);
	for (int i = 0; i < dst_image.size(); ++i)
		dst_image[i] = src_image[i];

	//�������� ����
	int i, j;
	for (i = 0; i < height; i++)
	{
		for (j = 0; j < width; j++)
		{
			img_file.write((char*)&dst_image[i * width + j], pixel_size);
		}
		img_file.write((char*)buf, j % 4);
	}
	img_file.close();

	return true;
}
bool SaveFilteredImage(string path)
{
	ofstream img_file;
	char buf[3];

	//������� ���� �� ������
	img_file.open(path.c_str(), ios::out | ios::binary);
	if (!img_file)
	{
		return false;
	}

	img_file.write((char*)&FileHead, sizeof(FileHead));
	img_file.write((char*)&InfoHead, sizeof(InfoHead));

	//�������� ����
	int i, j;
	for (i = 0; i < height; i++)
	{
		for (j = 0; j < width; j++)
		{
			img_file.write((char*)&ftr_image[i * width + j], pixel_size);
		}
		img_file.write((char*)buf, j % 4);
	}
	img_file.close();

	return true;
}

bool SaveBorderImage(string path)
{
	ofstream img_file;
	char buf[3];

	//������� ���� �� ������
	img_file.open(path.c_str(), ios::out | ios::binary);
	if (!img_file)
	{
		return false;
	}

	img_file.write((char*)&FileHead, sizeof(FileHead));
	img_file.write((char*)&InfoHead, sizeof(InfoHead));

	//�������� ����
	int i, j;
	for (i = 0; i < height; i++)
	{
		for (j = 0; j < width; j++)
		{
			img_file.write((char*)&brd_image[i * width + j], pixel_size);
		}
		img_file.write((char*)buf, j % 4);
	}
	img_file.close();

	return true;
}
///���������� ������� ����������� � ������� ������ ������������ ������������
void ShowImage(string path)
{
	ShowBMPHeaders(FileHead, InfoHead);
	system(path.c_str());
}

void AddNoise()
{
	string temp = "E:\\Trash\\ЛР1\\Example_source\\SimpleBitmap\\Astronaut2.bmp";
	int count1 = 0;
	std::cout << "Enter noize: ";
	std::cin >> count1;

	int size = width * height;
	int count = (int)(size * count1) / 100;
	int x, y;
	long pos;
	for (int i = 0; i < count; ++i)
	{
		x = rand() % width;
		y = rand() % height;
		pos = y * width + x;
		src_image[pos].blue = rand() % 256;
		src_image[pos].green = rand() % 256;
		src_image[pos].red = rand() % 256;
	}
	cout << "Point was added: " << count << endl;
	SaveImage(temp);
	ShowImage(temp);
}

int SortBlue(std::vector<Color>& collors)
{
	for (int i = 0; i < collors.size(); ++i)
	{
		for (int j = 0; j < collors.size() - 1; ++j)
		{
			if (collors[j].blue < collors[j + 1].blue)
			{
				swap(collors[j], collors[j + 1]);
			}
		}
	}
	return collors[(collors.size() / 2 + 1)].blue;
}
int SortRed(std::vector<Color>& collors)
{
	for (int i = 0; i < collors.size(); ++i)
	{
		for (int j = 0; j < collors.size() - 1; ++j)
		{
			if (collors[j].red < collors[j + 1].red)
			{
				swap(collors[j], collors[j + 1]);
			}
		}
	}
	return collors[(collors.size() / 2 + 1)].red;
}
int SortGreen(std::vector<Color>& collors)
{
	for (int i = 0; i < collors.size(); ++i)
	{
		for (int j = 0; j < collors.size() - 1; ++j)
		{
			if (collors[j].green < collors[j + 1].green)
			{
				swap(collors[j], collors[j + 1]);
			}
		}
	}
	return collors[(collors.size() / 2 + 1)].green;
}
void Vzvesivaniye(std::vector<Color>& input, std::vector<Color>& output)
{
	vector<int> weights = {
		1, 2, 3, 2, 1,
		2, 4, 5, 4, 2,
		3, 5, 8, 5, 3,
		2, 4, 5, 4, 2,
		1, 2, 3, 2, 1
	};
	for (int i = 0; i < 25; i++)
	{
		for (int j = 0; j < weights[i]; ++j) {
			// ���������� ����� � ������� �������
			output.push_back(input[i]);
		}
	}

}

void MedianFilter()
{
	string filteredTemp = "E:\\Trash\\ЛР1\\Example_source\\SimpleBitmap\\Astronaut3.bmp";
	int size = width * height;

	//����������� �� ��������� � �������������� �����������
	ftr_image.resize(size);
	for (int i = 0; i < ftr_image.size(); ++i)
		ftr_image[i] = dst_image[i];


	for (int x = 0; x < width - 5; ++x)
	{
		for (int y = 0; y < height - 5; ++y)
		{
			std::vector<Color> buff_collors;
			std::vector<Color> buff_collors2;

			for (int i = 0; i < 5; i++)
			{
				for (int j = 0; j < 5; j++)
				{
					buff_collors.push_back(Color(dst_image[(y + i) * width + (x + j)]));
				}
			}


			//Vzvesivaniye(buff_collors, buff_collors2);

			//ftr_image[(y + 2) * width + (x + 2)] = Color(SortGreen(buff_collors2), SortBlue(buff_collors2), SortRed(buff_collors2));
			ftr_image[(y + 2) * width + (x + 2)] = Color(SortGreen(buff_collors), SortBlue(buff_collors), SortRed(buff_collors));


		}
	}
	SaveFilteredImage(filteredTemp);
	ShowImage(filteredTemp);
}

int* ToBrightArr() {
		int* brightArr = new int[width * height];
		int i, j, index;
		for (i = 0; i < height; i++)
		{
			for (j = 0; j < width; j++)
			{
				index = i * width + j;
				brightArr[index] = src_image[index].blue * 0.11 + src_image[index].red * 0.3 + src_image[index].green * 0.59;
	
			}
		}
		return brightArr;
	}
	
int* ContrastPrevitt(int* brightArr) {
		int* contrastArr = new int[width * height];
		int i, j;
		int kernel[3][3] = {
			{ 1, 1, 1 },
			{ 1, -2, -1},
			{ 1, -1, -1}
		};
	
	
		for (int y = 1; y < height - 1; y++) {
			for (int x = 1; x < width - 1; x++) {
				int sum = 0;
				for (int i = 0; i < 3; i++) {
					for (int j = 0; j < 3; j++) {
						sum += kernel[i][j] * brightArr[(y - 1 + i) * width + (x - 1 + j)];
					}
				}
				contrastArr[y * width + x] = sum;
			}
		}
		return contrastArr;
	}
	
void ThresholdDetection(int* contrastArr) {
	int size = width * height;

	brd_image.resize(size);
	for (int i = 0; i < ftr_image.size(); ++i)
		brd_image[i] = dst_image[i];

		
		int i, j, treshhold, index;
		cout << "Enter treshhold - ";
		cin >> treshhold;
	
		for (i = 0; i < height; i++)
		{
			for (j = 0; j < width; j++)
			{
				index = i * width + j;
				if (contrastArr[index] > treshhold) {
					brd_image[index].blue = 255;
					brd_image[index].green = 255;
					brd_image[index].red = 255;
				}
				else {
					brd_image[index].blue = 0;
					brd_image[index].green = 0;
					brd_image[index].red = 0;
				}
			}
		}
		SaveBorderImage("E:\\Trash\\ЛР1\\Example_source\\SimpleBitmap\\Astronaut4.bmp");
		ShowImage("E:\\Trash\\ЛР1\\Example_source\\SimpleBitmap\\Astronaut4.bmp");
	}

int main()
{
	srand((unsigned)time(NULL));
	string path_to_image = "E:\\Trash\\ЛР1\\Example_source\\SimpleBitmap\\Astronaut.bmp";
	string filteredStupak = "E:\\Trash\\ЛР1\\Example_source\\SimpleBitmap\\Astronaut3.stupak";
	int value;
	cout << "1-MedianFilter\n2-BorderSource\n3-BorderFilter\n4-BorderNoise\n\nChoose: ";
	cin >> value;
	switch (value) {
		case 1: {
			OpenImage(path_to_image);
			AddNoise();
			MedianFilter();
			SaveStupakImage(filteredStupak, ftr_image, FileHead, InfoHead);
			OpenAndShowStupakHeaders(filteredStupak);
			break;
		};
		case 2: {
			OpenImage("E:\\Trash\\ЛР1\\Example_source\\SimpleBitmap\\Astronaut.bmp");
			ShowImage("E:\\Trash\\ЛР1\\Example_source\\SimpleBitmap\\Astronaut.bmp");
			ThresholdDetection(ContrastPrevitt(ToBrightArr()));
			break;
		};
		case 3: {
			OpenImage("E:\\Trash\\ЛР1\\Example_source\\SimpleBitmap\\Astronaut3.bmp");
			ShowImage("E:\\Trash\\ЛР1\\Example_source\\SimpleBitmap\\Astronaut3.bmp");
			ThresholdDetection(ContrastPrevitt(ToBrightArr()));
			break;
		};
		case 4: {
			OpenImage("E:\\Trash\\ЛР1\\Example_source\\SimpleBitmap\\Astronaut.bmp");
			AddNoise();
			ThresholdDetection(ContrastPrevitt(ToBrightArr()));
			break;
		};
		default: {

		}
	}

	cout << "END!" << endl;
	return 0;
}