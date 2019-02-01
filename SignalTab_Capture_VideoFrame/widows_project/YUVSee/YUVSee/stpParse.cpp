#include "stdafx.h"
#include "stpParse.h"
#include "LineReadFile.h"
#include "bitmap_image.h"
#include <commdlg.h>

vi_sync_gen_xy signaltab_data[SIGNAL_TAP_MEMORY_DEEP];									//解析stp文件，存入此结构，一行
unsigned short YUV[SIGNAL_TAP_VIDEO_MAX_HEIGHT][SIGNAL_TAP_VIDEO_MAX_WIDTH] = {0};		//一帧图像存入此结构 YUV422
unsigned int gRGB[SIGNAL_TAP_VIDEO_MAX_HEIGHT][SIGNAL_TAP_VIDEO_MAX_WIDTH] = {0};		//一帧图像的RGB

//本次解析的视频的 宽度和高度
unsigned int video_Width = 0;
unsigned int video_Height = 0;

bool saveBMP(char *strFileName)
{
	bool bRet = false;

	if ( !video_Width || !video_Height )
		return false;

	bitmap_image fractal(video_Width,video_Height);

	fractal.clear();

	for (unsigned int y = 0; y < fractal.height(); ++y)
	{
		for (unsigned int x = 0; x < fractal.width(); ++x)
		{
			fractal.set_pixel(x, y, GetRValue(gRGB[y][x]), GetGValue(gRGB[y][x]), GetBValue(gRGB[y][x]));
		}
	}
	fractal.save_image(strFileName);

	return true;
}

static unsigned int *gNewRGB = NULL; ////为了进行SetDIBitsToDevice的显示，构造一个RGB数组，用于GUI显示
bool changeStpYUV2RGB()
{
	bool bRet = false;
	int i,j;
	do
	{
		if ( !video_Height || !video_Width )
		{
			break;
		}

		if ( gNewRGB )
		{
			delete []gNewRGB;
			gNewRGB = NULL;
		}
		gNewRGB = new unsigned int[video_Height*video_Width];

		unsigned char cr,cb,y0,y1;
		for ( i = 0; i < video_Height; i++ )
		{
			for ( j = 0; j < video_Width; j+=2 )
			{
				y0 = YUV[i][j]&0xFF;
				y1 = YUV[i][j+1]&0xFF;
				cr = (YUV[i][j]&0xFF00)>>8;
				cb = (YUV[i][j+1]&0xFF00)>>8;
				gRGB[i][j] = YUV2RGB(y0, cr, cb);
				gRGB[i][j+1] = YUV2RGB(y1, cr, cb);

				//构造一个RGB数组，用于GUI显示，对于32bit下面的RGB需要翻转下用于SetDIBitsToDevice的显示
				*(gNewRGB + i*video_Width + j) = RGB( GetBValue(gRGB[i][j]), GetGValue(gRGB[i][j]), GetRValue(gRGB[i][j]));
				*(gNewRGB + i*video_Width + j + 1) = RGB( GetBValue(gRGB[i][j+1]), GetGValue(gRGB[i][j+1]), GetRValue(gRGB[i][j+1]));
			}
		}
		bRet = true;
	}while(false);

	return bRet;
}

static HWND hLocalhWnd = NULL;
bool readStpFile2YUV(char *strFileName)
{
	bool bRet = false;
	CLineReadFile *pStp = NULL;
	int i, j, rd, parsedLine;
	unsigned short *pYUV = NULL, *pLastYUV = NULL;
	char *pData = NULL;
	bool bCheck = false;
	bool bPickData = false;

	do 
	{
		if ( !strFileName || !*strFileName )
		{
			break;
		}

		pStp = new CLineReadFile(strFileName);
		if ( !pStp )
		{
			break;
		}
		//初始化
		video_Width = 0;
		video_Height = 0;
		memset(signaltab_data, SIGNAL_TAP_MEMORY_DEEP, sizeof(vi_sync_gen_xy));
		for ( i = 0; i < SIGNAL_TAP_VIDEO_MAX_HEIGHT; i++ )
			for ( j = 0; j < SIGNAL_TAP_VIDEO_MAX_WIDTH; j++ )
				YUV[i][j] = 0x8010;									//默认填充为黑色
		parsedLine = 0; //已经解析了的行数
	
		while ( (rd = pStp->readline()) > 0)
		{
			pData = pStp->ltrim();
			parsedLine = 0;
			if ( !strncmp(pData, "<log>", 5))
			{
				//开始读取具体的数据
				do
				{
					rd = pStp->readline();
					pData = pStp->ltrim();
					if ( !strncmp(pData, "<data", 5) )	//找到data
					{
						bRet = true;
						char *p = pData, *p2;
						while(*p++ != '>');
						p2 = p;
						while ( *p++ != '<');
						*(p-1) = '\0';
						//对 8192 * 28 的0101数据进行解析和读取 
						for ( i = 0; i < SIGNAL_TAP_MEMORY_DEEP; i++ )
						{
							char *pStr = p2 + i * 28;
							signaltab_data[i].this2out = *pStr-'0';
							signaltab_data[i].y = (*(pStr+18)-'0')*1024 + (*(pStr+27)-'0')*512 + (*(pStr+26)-'0')*256 + (*(pStr+25)-'0')*128 + (*(pStr+24)-'0')*64
								+ (*(pStr+23)-'0')*32 + (*(pStr+22)-'0')*16 + (*(pStr+21)-'0')*8 + (*(pStr+20)-'0')*4 + (*(pStr+19)-'0')*2 + (*(pStr+17)-'0');
							signaltab_data[i].video_data = (*(pStr+7)-'0')*8192*4 + (*(pStr+6)-'0')*8192*2 + (*(pStr+5)-'0')*8192 + (*(pStr+4)-'0')*4096 + (*(pStr+3)-'0')*2048
								+ (*(pStr+2)-'0')*1024 + (*(pStr+16)-'0')*512 + (*(pStr+15)-'0')*256 + (*(pStr+14)-'0')*128 + (*(pStr+13)-'0')*64 + (*(pStr+12)-'0')*32
								+ (*(pStr+11)-'0')*16 + (*(pStr+10)-'0')*8 + (*(pStr+9)-'0')*4 + (*(pStr+8)-'0')*2 + (*(pStr+1)-'0');
						}

						//开始解析本次采集到的数据
						bCheck = false;
						bPickData = false;
						int last_x = 0;
						for ( i = 0; i < SIGNAL_TAP_MEMORY_DEEP; i++ )
						{
							//if ( signaltab_data[i].this2out || bCheck)
							{
								bCheck = true; //从此次之后							
								//找到SAV，则开始提取数据
								if ( i > 4 && signaltab_data[i-3].video_data == 0xFFFF 
											 && signaltab_data[i-2].video_data == 0x0000
											 && signaltab_data[i-1].video_data == 0x0000
											 &&  signaltab_data[i].video_data == 0x8080)
								{
									bPickData = true;
									pYUV = YUV[signaltab_data[i].y];
									pLastYUV = pYUV;
									if ( video_Height < signaltab_data[i].y )//&& signaltab_data[i].y <= 1080 )
									{
										video_Height = signaltab_data[i].y;
									}
									continue;
								}

								//发现EAV，则结束提取数据
								if ( i < (SIGNAL_TAP_MEMORY_DEEP -4) && signaltab_data[i].video_data == 0xFFFF 
											 && signaltab_data[i+1].video_data == 0x0000
											 && signaltab_data[i+2].video_data == 0x0000
											 && signaltab_data[i+3].video_data == 0x9d9d)
								{
									if (bPickData && video_Width < pYUV - pLastYUV )
									{
										video_Width = pYUV - pLastYUV;
									}
									bPickData = false;
					
									continue;
								}

								if ( bPickData )
								{
									*pYUV++ = signaltab_data[i].video_data;
								}
							}
						}
					}
					
					//再读<extradata>
					pStp->readline();
					//读</log>
					pStp->readline();
					//再读到下一个<log
					rd = pStp->readline();
					pData = pStp->ltrim();
					parsedLine++;

					/*
						我需要更新下GUI显示
					*/
					HDC hDC = GetDC(hLocalhWnd);
					gui_dispay_opening_file(hLocalhWnd, hDC);
					ReleaseDC(hLocalhWnd, hDC);

				} while ( !strncmp(pData, "<log>", 5));
				//读取完了所有的Data
				//请确保stp文件中，只有一个日志模块，
				if ( parsedLine > 10 )
					break;
			}
		}
		//完毕，回收资源
		delete pStp;
		bRet = true;
		video_Height++;			//从fpga采集到的数据，高度的下标是从0开始的，0~1079，所以它的高度就，1079+1 = 1080，所以在解析完，这里+1
	} while ( false );

	return bRet;
}

unsigned int YUV2RGB(UINT8 y, UINT8 u, UINT8 v)
{
        //unsigned int pixel32 = 0;
        //unsigned char *pixel = (unsigned char *)&pixel32;
        int r, g, b;
        r = y + (1.370705 * (v-128));
        g = y - (0.698001 * (v-128)) - (0.337633 * (u-128));
        b = y + (1.732446 * (u-128));
        if(r > 255) r = 255;
        if(g > 255) g = 255;
        if(b > 255) b = 255;
        if(r < 0) r = 0;
        if(g < 0) g = 0;
        if(b < 0) b = 0;
		return RGB(r,g,b);
}

/********************************* windows 窗体相关函数 ************************************/
int g_displayMode = 1;			//WM_PAINT消息时，显示的模式；三种，1，无操作，空白；2，正在打开stp文件；3，打开stp文件完毕，显示图片；

void gui_dispay_opening_file(HWND hWnd, HDC hDC)
{
	static unsigned int opening_i = 0;
	static TCHAR tc[][2] = {L"-", L"/", L"|", L"\\"};
	RECT r;
	GetClientRect(hWnd, &r);
	TCHAR strDisplay[32];
	wsprintf(strDisplay, L"正在解析stp文件，请等待(%s).", tc[opening_i++%4]);
	DrawText(hDC, strDisplay, -1, &r, DT_VCENTER|DT_SINGLELINE|DT_CENTER);
}

void gui_display_video(HWND hWnd, HDC hDC)
{
	BITMAPINFO bmi;   
	ZeroMemory(&bmi,   sizeof(BITMAPINFO));   
	bmi.bmiHeader.biSize   =   sizeof(BITMAPINFOHEADER);   
	bmi.bmiHeader.biWidth   =  video_Width;   
	bmi.bmiHeader.biHeight   = -video_Height; //当图像是倒立显示的时候，把biHeight改为对应的负值
	//bmi.bmiHeader.biSizeImage = video_Width *video_Height;
	bmi.bmiHeader.biPlanes   =   1;   
	bmi.bmiHeader.biBitCount   =   32;   
	bmi.bmiHeader.biCompression   =   BI_RGB;
#if 1
	SetDIBitsToDevice(hDC,0,0,video_Width,video_Height,
		0,0,
		0,	//first scan line
		video_Height,	//number of scan lines
		gNewRGB,
		&bmi,DIB_RGB_COLORS);
#else
	StretchDIBits(hDC,0,0,video_Width,video_Height,
		0,0,video_Width,video_Height,gNewRGB, &bmi,DIB_RGB_COLORS,SRCCOPY);
#endif
}

//解析 stp文件的线程
DWORD WINAPI ThreadParseStpFile(LPVOID p)
{
	char *strFileName = (char *)p;
	
	bool bRet = readStpFile2YUV(strFileName);
	if ( bRet )
	{
		g_displayMode = 3; 
	}
	else
	{
		g_displayMode = 1;
	}
	bRet = changeStpYUV2RGB();
	/*更新显示 */
	HDC hDC = GetDC(hLocalhWnd);
	gui_display_video(hLocalhWnd, hDC);
	ReleaseDC(hLocalhWnd, hDC);
	
	return 0;
}

//打开stp文件
bool openStpDlg(HWND hWnd, HINSTANCE hInst)
{
	bool bRet = false;
	char *strFileName = getFileNameByDialog(hWnd, hInst, true);
	if ( strFileName )
	{
		g_displayMode = 2;//表示正在打开文件，创建解析stp的线程
		//进行GUI的刷新
		//开启子线程，解析stp文件
		HANDLE hThread;
		DWORD threadId;
		hLocalhWnd = hWnd;
		hThread = CreateThread(NULL, 0, ThreadParseStpFile, (LPVOID)strFileName, 0, &threadId);
		DWORD wr = 0;
		HDC hDC = GetDC((HWND)hWnd);
		gui_dispay_opening_file((HWND)hWnd, hDC);
		ReleaseDC((HWND)hWnd, hDC);
	}
	return bRet;
}

//保存bmp文件
bool saveBmpDlg(HWND hWnd, HINSTANCE hInst)
{
	bool bRet = false;
	char *strFileName = getFileNameByDialog(hWnd, hInst, false);
	if ( strFileName )
	{
		bRet = saveBMP(strFileName);
	}
	return bRet;
}

//打开文件和保存文件的对话框
static char strDlg_FileName[1024] = {0};	//用于保存文件名，打开文件。。。对主框， 保存文件对话框。。。，最后得到一个文件名
char *getFileNameByDialog(HWND hWnd, HINSTANCE hInst ,bool isOpenDialog)
{
	OPENFILENAME ofn;
	WCHAR wstrFileName[512];
	WCHAR* szFile = new WCHAR[512];
	WCHAR* szFileTitle = new WCHAR[512];
	memset(&ofn, 0, sizeof(ofn));
	memset(szFile, 0, sizeof(WCHAR)*512);
	memset(szFileTitle, 0, sizeof(WCHAR)*512);
 
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = (HWND)hWnd;
	ofn.hInstance =	(HINSTANCE)hInst;
	ofn.lpstrFilter = L"All File\0*.*\0";	
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(WCHAR)*512;
	ofn.lpstrFileTitle = szFileTitle;
	ofn.nMaxFileTitle = sizeof(WCHAR)*512;
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_EXPLORER;
 
	// 按下确定按钮
	if (isOpenDialog )	//打开文件
	{
		if (GetOpenFileName(&ofn))
		{
			lstrcpyW(wstrFileName, ofn.lpstrFile);
			::SetWindowText((HWND)hWnd, ofn.lpstrFile);
			//需要打开文件
			//char *fileName = s_strFileName;
			int    iTextLen = WideCharToMultiByte( CP_ACP, 0, wstrFileName, -1, NULL,0,NULL,NULL );
			char *strFileName = new char[iTextLen+1];
			memset(strFileName, 0, iTextLen+1);
			WideCharToMultiByte( CP_ACP, 0, wstrFileName, -1, strFileName,iTextLen,NULL,NULL );
			memcpy(strDlg_FileName, strFileName, iTextLen+1);
		}
		else
		{
			delete []szFile;
			delete []szFileTitle;
			return NULL;
		}
	}
	else                //保存文件
	{
		if (GetSaveFileName(&ofn))
		{
			lstrcpyW(wstrFileName, ofn.lpstrFile);
			::SetWindowText((HWND)hWnd, ofn.lpstrFile);
			//需要打开文件
			//char *fileName = s_strFileName;
			int    iTextLen = WideCharToMultiByte( CP_ACP, 0, wstrFileName, -1, NULL,0,NULL,NULL );
			char *strFileName = new char[iTextLen+1];
			memset(strFileName, 0, iTextLen+1);
			WideCharToMultiByte( CP_ACP, 0, wstrFileName, -1, strFileName,iTextLen,NULL,NULL );
			memcpy(strDlg_FileName, strFileName, iTextLen+1);
		}
		else
		{
			delete []szFile;
			delete []szFileTitle;
			return NULL;
		}
	}

	delete []szFile;
	delete []szFileTitle;
	
	return strDlg_FileName;
}