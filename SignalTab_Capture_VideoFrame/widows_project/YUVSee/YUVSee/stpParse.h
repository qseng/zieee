#pragma once

const int SIGNAL_TAP_MEMORY_DEEP = 2048;//8192;
const int SIGNAL_TAP_VIDEO_MAX_WIDTH = 1920;//2048;//1920;
const int SIGNAL_TAP_VIDEO_MAX_HEIGHT = 1080;//1280;//1080;

typedef struct _vi_sync_gen_xy
{
	unsigned char	this2out;
	unsigned short	video_data;
	unsigned short	y;
}vi_sync_gen_xy;

//extern vi_sync_gen_xy signaltab_data[SIGNAL_TAP_MEMORY_DEEP];
//extern unsigned short YUV[SIGNAL_TAP_VIDEO_MAX_HEIGHT+2][SIGNAL_TAP_VIDEO_MAX_WIDTH+2];
//extern unsigned int gRGB[SIGNAL_TAP_VIDEO_MAX_HEIGHT+2][SIGNAL_TAP_VIDEO_MAX_WIDTH+2];
extern int g_displayMode;

bool readStpFile2YUV(char *strFileName);
unsigned int YUV2RGB(UINT8 y, UINT8 u, UINT8 v);
bool changeStpYUV2RGB();
bool saveBMP(char *strFileName);

//windows œ‡πÿ
bool openStpDlg(HWND hWnd, HINSTANCE hInst);
bool saveBmpDlg(HWND hWnd, HINSTANCE hInst);
char *getFileNameByDialog(HWND hWnd, HINSTANCE hInst ,bool isOpenDialog);

void gui_dispay_opening_file(HWND hWnd, HDC hDC);
void gui_display_video(HWND hWnd, HDC hDC);