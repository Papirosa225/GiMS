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

//Заголовок файла BMP 
typedef struct tBITMAPFILEHEADER
{
	WORD bfType;
	DWORD bfSize;
	WORD bfReserved1;
	WORD bfReserved2;
	DWORD bfOffBits;
}sFileHead;

//Заголовок BitMap's
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
//Заголовок собственного файла Stupak 
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

//Пиксель
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

//Размер 1-го пикселя
int pixel_size = sizeof(Color);


//1 - BMP, 2 - CMP
int img_type = 0;

//Исходное изображение
std::vector<Color> src_image;
//Результативное изображение
std::vector<Color> dst_image;
//Отфильтрованное изображение
std::vector<Color> ftr_image;
//Размер изображения
int width = 0;
int height = 0;

//Вывести заголовок BMP файла
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

//Функция для загрузки изображения
bool OpenImage(string path)
{
	ifstream img_file;
	Color temp;
	char buf[3];

	//Открыть файл на чтение
	img_file.open(path.c_str(), ios::in | ios::binary);
	if (!img_file)
	{
		cout << "File isn`t open!" << endl;
		return false;
	}

	//Считать заголовки BMP
	img_file.read((char*)&FileHead, sizeof(FileHead));
	img_file.read((char*)&InfoHead, sizeof(InfoHead));

	img_type = 1;
	ShowBMPHeaders(FileHead, InfoHead);
	//Присвоить длину и ширину изображения
	width = InfoHead.biWidth;
	height = InfoHead.biHeight;


	//Выделить место под изображение
	src_image.resize(width * height);

	int i, j;
	for (i = 0; i < height; i++)
	{
		for (j = 0; j < width; j++)
		{
			img_file.read((char*)&temp, pixel_size);
			src_image[i * width + j] = temp;
		}
		//Дочитать биты используемые для выравнивания до двойного слова
		img_file.read((char*)buf, j % 4);
	}
	img_file.close();

	return true;
}

//Функция для сохранения изображение c моим расширением
bool SaveStupakImage(const string path, const std::vector<Color>& image, tBITMAPFILEHEADER fh, tBITMAPINFOHEADER ih)
{
	ofstream img_file;
	char buf[3];

	//Открыть файл на запись
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
		fh.bfSize + sizeof(MyFileHead) - sizeof(sFileHead) - sizeof(sInfoHead)+1, //fileOffset
	};

	img_file.write((char*)&fHead, sizeof(MyFileHead));

	//Запись файл
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

//Открыть и вывести заголовок stupak файла
void OpenAndShowStupakHeaders(string path)
{
	ifstream img_file;
	Color temp;
	char buf[3];

	//Открыть файл на чтение
	img_file.open(path.c_str(), ios::in | ios::binary);
	if (!img_file)
	{
		cout << "File isn`t open!" << endl;
		return;
	}

	MyFileHead fHead;

	//Считать заголовки BMP
	img_file.read((char*)&fHead, sizeof(MyFileHead));

	cout << "\n\nFile type identifier: " << (CHAR)fHead.fileTypeIdentifier << "\n"
		<< "File offset: " << fHead.fileOffset << "\n"
		<< "File size in bytes: " << fHead.fileSizeInBytes << "\n"
		<< "Header size in bytes: " << fHead.headerSizeInBytes << "\n"
		<< "Raster size in bytes: " << fHead.rasterSizeInBytes << "\n"
		<< "Color depth: " << (int)fHead.colorDepth << "\n"
		<< "Image width in pixels: " << fHead.imageWidthInPixels << "\n";
}

//Функция для сохранения изображение
bool SaveImage(string path)
{
	ofstream img_file;
	char buf[3];

	//Открыть файл на запись
	img_file.open(path.c_str(), ios::out | ios::binary);
	if (!img_file)
	{
		return false;
	}

	img_file.write((char*)&FileHead, sizeof(FileHead));
	img_file.write((char*)&InfoHead, sizeof(InfoHead));

	//Скопировать из исходного в результирующее изображение
	dst_image.resize(width * height);
	for (int i = 0; i < dst_image.size(); ++i)
		dst_image[i] = src_image[i];

	//Записать файл
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

//Зашумление изображения с заданной долей вероятности
void AddNoise(double probability)
{
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
		2, 4, 6, 4, 2,
		3, 6, 9, 6, 3,
		2, 4, 6, 4, 2,
		1, 2, 3, 2, 1
	};
	for (int i = 0; i < 25; i++)
	{
		for (int j = 0; j < weights[i]; ++j) {
			// Применение весов к каждому пикселю
			output.push_back(input[i]);
		}
	}
	
}

void MedianFilter()
{
	int size = width * height;

	//Скопировать из исходного в результирующее изображение
	ftr_image.resize(size);
	for (int i = 0; i < ftr_image.size(); ++i)
		ftr_image[i] = dst_image[i];


	for (int x = 0; x < width-5; ++x)
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
		

			Vzvesivaniye(buff_collors, buff_collors2);

			ftr_image[(y + 2) * width + (x+2)] = Color(SortGreen(buff_collors2), SortBlue(buff_collors2), SortRed(buff_collors2));

		}
	}
}

//Функция для сохранения отфильтрованного изображения
bool SaveFilteredImage(string path)
{
	ofstream img_file;
	char buf[3];

	//Открыть файл на запись
	img_file.open(path.c_str(), ios::out | ios::binary);
	if (!img_file)
	{
		return false;
	}

	img_file.write((char*)&FileHead, sizeof(FileHead));
	img_file.write((char*)&InfoHead, sizeof(InfoHead));

	//Записать файл
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

//Отобразить текущее изображение с помощью вызова стандартного просмотрщика
void ShowImage(string path)
{
	ShowBMPHeaders(FileHead, InfoHead);
	system(path.c_str());
}

int main()
{
	srand((unsigned)time(NULL));

	//Путь к текущему изображению
	string path_to_image = "E:\\Example_source\\SimpleBitmap\\Astronaut.bmp";
	string temp = "E:\\Example_source\\SimpleBitmap\\Astronaut2.bmp";
	string filteredTemp = "E:\\Example_source\\SimpleBitmap\\Astronaut3.bmp";
	string filteredStupak = "E:\\Example_source\\SimpleBitmap\\Astronaut3.stupak";
	OpenImage(path_to_image);
	ShowImage(path_to_image);

	AddNoise(20);
	SaveImage(temp);
	ShowImage(temp);

	MedianFilter();
	SaveFilteredImage(filteredTemp);
	ShowImage(filteredTemp);

	SaveStupakImage(filteredStupak, ftr_image, FileHead, InfoHead);
	OpenAndShowStupakHeaders(filteredStupak);
	cout << "END!" << endl;
	return 0;
}