#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0400
#include <windows.h>
#include <commdlg.h>
#undef _WIN32_WINNT

#include "log_file.h"
#include "regex.h"
#include "alc_file.h"
#include "lib_func.h"
#include "alc_var.h"

#define TITLE_STRING "Analizador de Logs CSI"

static LOGFONT	  alc_logfont;
static HINSTANCE  alc_hinstance = NULL;
static HFONT	    alc_hfont     = NULL;
static HWND       alc_hwnd      = NULL;
static BOOL       alc_terminate = FALSE;
static LogFile   *alc_log_file  = NULL;
static SIZE       alc_font_size;
static int        alc_delta_per_line;
static int        alc_accum_delta;

static ATOM alc_register_class       (HINSTANCE hinstance);
static HWND alc_create_window        (HINSTANCE hinstance);
static void alc_open_file_dialog     (HWND hwnd);
static BOOL alc_wnd_init             ();
static void alc_update_vscroll       ();
static void alc_update_hscroll       ();
static void alc_draw_lines           (HDC hdc);
static void alc_on_vscroll           (UINT code);
static void alc_on_hscroll           (UINT code);
static void alc_on_mouse_wheel       (short delta, int x, int y);

static LRESULT CALLBACK alc_wnd_proc(HWND hwnd, UINT message, WPARAM w_param, LPARAM l_param);

void FormatMessageError(char* str_error, unsigned int len, unsigned long error)
{
  char* msg;
  int size = FormatMessage( 
    FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,
    NULL,
    error,
    0,
    (LPTSTR)&msg,
    0,
    NULL);
  size = min(size, (int)len);
  strcpy(str_error, msg);
  LocalFree((void *)msg);
}

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
  MSG msg;
  ULONG scroll_lines;
  List* list_var = NULL;
  AlcFilter *filter = NULL;

  alc_delta_per_line = 0;
  alc_accum_delta = 0;

  filter = alc_filter_new();
  alc_file_load(filter, "icc");

  alc_wnd_init();

  alc_log_file = alc_filter_load_lines(filter, "\\icc.err");
  if(alc_log_file == NULL)
  {
    return -1;
  }

  ShowWindow(alc_hwnd, SW_SHOW);
  UpdateWindow(alc_hwnd);
  alc_update_vscroll();
  alc_update_hscroll();
  
	SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &scroll_lines, 0);
  if(scroll_lines)
    alc_delta_per_line = WHEEL_DELTA / scroll_lines;
  else
    alc_delta_per_line = 0;

	while(!alc_terminate) 
  {
    if(PeekMessage(&msg,NULL,0,0,PM_REMOVE))
    {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
    Sleep(10);
	}

  DestroyWindow(alc_hwnd);
  log_file_clear(alc_log_file);

	return 0;
}

BOOL alc_wnd_init()
{
  HDC hdc;

  /* registrando aplicacao */
  if(alc_hinstance == NULL)
  {
    alc_hinstance = (HINSTANCE)GetModuleHandle(NULL);
    alc_register_class(alc_hinstance);
  }

  /* criando janela */
  if(alc_hwnd == NULL)
  {
    alc_hwnd = alc_create_window(alc_hinstance);
    if(alc_hwnd == NULL) 
      return FALSE;
  }

  hdc = GetWindowDC(alc_hwnd);
  GetTextExtentPoint(hdc, "Åy", 2, &alc_font_size);
  ReleaseDC(alc_hwnd, hdc);

  return TRUE;
}

ATOM alc_register_class(HINSTANCE hinstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize         = sizeof(WNDCLASSEX); 
	wcex.style			    = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	  = (WNDPROC)alc_wnd_proc;
	wcex.cbClsExtra		  = 0;
	wcex.cbWndExtra		  = 0;
	wcex.hInstance		  = hinstance;
	wcex.hIcon			    = NULL;
	wcex.hCursor		    = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	  = TITLE_STRING;
	wcex.lpszClassName	= TITLE_STRING;
	wcex.hIconSm		    = NULL;

	return RegisterClassEx(&wcex);
}

HWND alc_create_window(HINSTANCE hinstance)
{
  /* configurando a fonte padrao */
  strcpy(alc_logfont.lfFaceName, "Courier New");
  alc_logfont.lfHeight   = -12;
  alc_logfont.lfWidth    = 0;
  alc_logfont.lfWeight   = FW_NORMAL;
  alc_hfont = (HFONT)CreateFontIndirect(&alc_logfont);

  /* Criando Janela */
  return CreateWindow(
    TITLE_STRING, 
    TITLE_STRING, 
    WS_OVERLAPPEDWINDOW,
    CW_USEDEFAULT,
    CW_USEDEFAULT,
    CW_USEDEFAULT,
    CW_USEDEFAULT,
    NULL, 
    NULL, 
    hinstance, 
    NULL);
}

LRESULT CALLBACK alc_wnd_proc(HWND hwnd, UINT message, WPARAM w_param, LPARAM l_param)
{
	switch (message) 
	{
    case WM_CREATE:
    break;
		case WM_COMMAND:
      break;
    case WM_KEYDOWN:
      break;
    case WM_PAINT:
      {
        PAINTSTRUCT ps;
        HDC hdc;

        hdc = BeginPaint(hwnd, &ps);
        alc_draw_lines(hdc);
        EndPaint(hwnd, &ps);
      }
      break;
    case WM_HSCROLL:
      alc_on_hscroll(w_param);
      break;
    case WM_VSCROLL:
      alc_on_vscroll(w_param);
      break;
    case WM_MOUSEWHEEL:
      alc_on_mouse_wheel(HIWORD(w_param), LOWORD(l_param), HIWORD(l_param));
      break;
    case WM_DESTROY:
      alc_open_file_dialog(hwnd);
      alc_terminate = TRUE;
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hwnd, message, w_param, l_param);
   }
   return 0;
}

void alc_update_vscroll()
{
	SCROLLINFO scrollinfo;
  RECT rect;
	int pct;

  if(alc_hwnd == NULL || alc_log_file == NULL)
    return;
  
  GetClientRect(alc_hwnd, &rect);

  scrollinfo.cbSize = sizeof(SCROLLINFO);
	scrollinfo.fMask  = SIF_ALL;

  pct = 0;
  if(GetScrollInfo(alc_hwnd, SB_VERT, &scrollinfo) && log_file_count_line(alc_log_file))
		pct = ((scrollinfo.nPos + scrollinfo.nPage)*100)/log_file_count_line(alc_log_file);
	
	scrollinfo.cbSize = sizeof(SCROLLINFO);
	scrollinfo.fMask  = SIF_RANGE | SIF_PAGE;
	scrollinfo.nMin   = 0;
	scrollinfo.nMax   = log_file_count_line(alc_log_file) - 1;
	scrollinfo.nPage  = (unsigned int)((rect.bottom * 0.7) / alc_font_size.cy);
	SetScrollInfo(alc_hwnd,SB_VERT, &scrollinfo, TRUE);
	
	scrollinfo.cbSize = sizeof(SCROLLINFO);
	scrollinfo.fMask  = SIF_ALL;
	GetScrollInfo(alc_hwnd, SB_VERT, &scrollinfo);

	if(pct >= 100)
		scrollinfo.nPos = (scrollinfo.nMax - scrollinfo.nPage)+1;
		
	scrollinfo.fMask = SIF_POS;
	SetScrollInfo(alc_hwnd, SB_VERT, &scrollinfo, TRUE);
}


void alc_update_hscroll()
{
	SCROLLINFO scrollinfo;
  RECT rect;

  if(alc_hwnd == NULL || alc_log_file == NULL)
    return;

  GetClientRect(alc_hwnd, &rect);

  scrollinfo.cbSize = sizeof(SCROLLINFO);
  scrollinfo.fMask  = SIF_RANGE | SIF_PAGE;
  scrollinfo.nMin   = 0;
  scrollinfo.nMax   = alc_log_file->max_car_lines-1;
  scrollinfo.nPage  = (unsigned int)((rect.right * 0.4) / alc_font_size.cx);
  SetScrollInfo(alc_hwnd, SB_HORZ, &scrollinfo, TRUE);
}

void alc_draw_lines(HDC hdc)
{
  SCROLLINFO scrollinfo;
  HFONT hfont_old;
  RECT rect;
  int height;
  unsigned long tot_lines;
  unsigned long paint_beg, paint_end;
  unsigned int line_len;
  unsigned int vert_pos, horz_pos;	
  List *line;
  char *aux;
  
  if(alc_hwnd == NULL || alc_log_file == NULL)
    return;

  GetClientRect(alc_hwnd, &rect);

  /* Posicao vertial da barra de rolagem */
  scrollinfo.cbSize = sizeof(SCROLLINFO);
  scrollinfo.fMask  = SIF_ALL;
  GetScrollInfo(alc_hwnd, SB_VERT, &scrollinfo);
  vert_pos = scrollinfo.nPos ;	
 
  /* Posicao horizontal da barra de rolagem */
  GetScrollInfo(alc_hwnd, SB_HORZ, &scrollinfo);
  horz_pos = scrollinfo.nPos ;

  SetBkMode(hdc, TRANSPARENT);
  SetTextColor(hdc, RGB(0x00, 0x00, 0x00));
  
  height = tot_lines = 0;

  paint_beg = max(0, vert_pos + rect.top / alc_font_size.cy) ;
  paint_end = min((unsigned int)log_file_count_line(alc_log_file) - 1, 
    (unsigned long)(vert_pos + rect.bottom / alc_font_size.cy));

  SetBkMode(hdc, TRANSPARENT);
  SetTextColor(hdc, RGB(0x00, 0x00, 0x00)); /* preto */
  
  if(alc_hfont != NULL)
    hfont_old = (HFONT)SelectObject(hdc, alc_hfont);

  for(line = alc_log_file->lines; line; line = line->next)
  {
    if(tot_lines >= paint_beg && tot_lines <= paint_end)
    {
      line_len = strlen((char*)line->data);
      if(horz_pos < line_len)
      {
        aux = ((char*)line->data)+horz_pos;
        TextOut(hdc, 0, height, (char*)aux, strlen(aux));
      }
      else
      {
        TextOut(hdc, 0, height, " ", 1);
      }
      height += alc_font_size.cy;
    }
    tot_lines++;
    
    if(tot_lines > paint_end)
      break;
 	}

  if(alc_hfont != NULL)
    SelectObject(hdc, hfont_old);
}

void alc_on_hscroll(UINT code) 
{
  int horz_pos;
	SCROLLINFO scrollinfo;
	
  if(alc_hwnd == NULL)
    return;

	scrollinfo.cbSize = sizeof (SCROLLINFO);
	scrollinfo.fMask  = SIF_ALL;
	
	GetScrollInfo(alc_hwnd, SB_HORZ, &scrollinfo);
	horz_pos = scrollinfo.nPos;
	switch(LOWORD(code))
	{
	case SB_LINELEFT:
		scrollinfo.nPos -= 1 ;
		break ;
	case SB_LINERIGHT:
		scrollinfo.nPos += 1 ;
		break ;
	case SB_PAGELEFT:
		scrollinfo.nPos -= scrollinfo.nPage ;
		break ;
	case SB_PAGERIGHT:
		scrollinfo.nPos += scrollinfo.nPage ;
		break ;
	case SB_THUMBPOSITION:
		scrollinfo.nPos = scrollinfo.nTrackPos ;
		break ;
	case SB_THUMBTRACK:
		scrollinfo.nPos = scrollinfo.nTrackPos ;
		break ;
	default :
		break ;
	}
	
	scrollinfo.fMask = SIF_POS ;
	SetScrollInfo(alc_hwnd, SB_HORZ, &scrollinfo,TRUE);
	GetScrollInfo(alc_hwnd, SB_HORZ, &scrollinfo);
	
	if(scrollinfo.nPos != horz_pos)
  {
    RECT rect;
    GetClientRect(alc_hwnd, &rect);
    InvalidateRect(alc_hwnd, &rect, TRUE);
  }
}

void alc_on_vscroll(UINT code) 
{
  int vert_pos;
	SCROLLINFO scrollinfo;
	
  if(alc_hwnd == NULL)
    return;

	scrollinfo.cbSize = sizeof (SCROLLINFO);
	scrollinfo.fMask  = SIF_ALL;
	
	GetScrollInfo(alc_hwnd, SB_VERT, &scrollinfo);
	vert_pos = scrollinfo.nPos;

  switch(code)
  {
  case SB_TOP:
    scrollinfo.nPos = scrollinfo.nMin;
    break ;
  case SB_BOTTOM:
    scrollinfo.nPos = scrollinfo.nMax;
    break ;
  case SB_LINEUP:
    scrollinfo.nPos -= 1;
    break ;
  case SB_LINEDOWN:
    scrollinfo.nPos += 1;
    break ;
  case SB_PAGEUP:
    scrollinfo.nPos -= scrollinfo.nPage;
    break ;
  case SB_PAGEDOWN:
    scrollinfo.nPos += scrollinfo.nPage;
    break ;
  case SB_THUMBTRACK:
    scrollinfo.nPos = scrollinfo.nTrackPos;
    break ;
  default:
    break ;         
  }
  
  scrollinfo.fMask = SIF_POS;
  SetScrollInfo(alc_hwnd, SB_VERT, &scrollinfo, TRUE);
  GetScrollInfo(alc_hwnd, SB_VERT, &scrollinfo);
  
  if(scrollinfo.nPos != vert_pos)
  {
    RECT rect;
    GetClientRect(alc_hwnd, &rect);
    InvalidateRect(alc_hwnd, &rect, TRUE);
  }
}

void alc_on_mouse_wheel(short delta, int x, int y) 
{
  if(alc_hwnd == NULL)
    return;
  
  if(alc_delta_per_line == 0 )
    return;

  alc_accum_delta += delta;
  while(alc_accum_delta >= alc_delta_per_line)
  {               
    SendMessage(alc_hwnd, WM_VSCROLL, SB_LINEUP, 0) ;
    alc_accum_delta -= alc_delta_per_line ;
  }
  
  while (alc_accum_delta <= -alc_delta_per_line)
  {
    SendMessage(alc_hwnd, WM_VSCROLL, SB_LINEDOWN, 0) ;
    alc_accum_delta += alc_delta_per_line ;
  }
}

void
alc_open_file_dialog(HWND hwnd)
{
  OPENFILENAME open_file_dlg;
  char path_file[260];
  
  ZeroMemory(&open_file_dlg, sizeof(OPENFILENAME));
  open_file_dlg.lStructSize = sizeof(OPENFILENAME);
  open_file_dlg.hwndOwner = hwnd;
  open_file_dlg.lpstrFile = path_file;
  open_file_dlg.nMaxFile = sizeof(path_file);
  open_file_dlg.lpstrFilter = "All\0*.*\0Trace ICC\0*.err\0";
  open_file_dlg.nFilterIndex = 1;
  open_file_dlg.lpstrFileTitle = NULL;
  open_file_dlg.nMaxFileTitle = 0;
  open_file_dlg.lpstrInitialDir = NULL;
  open_file_dlg.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

#if 0
  if(GetOpenFileName(&open_file_dlg)==TRUE) 
  {
  }
  else
  {
  }
  return;
#endif
}

