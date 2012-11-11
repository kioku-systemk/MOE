/*
	TimelineView Control
	2005/6/19 coded by kioku
*/
#pragma once
#include <windows.h>
#include "commoncontrol.h"

class CTimelineView : public CCommonControl
{
	private:
		static const char* SZ_CONTROL_CLASS_NAME;
		static const int   RAW_LINE;
		static const int   COL_LINE;
		void SetSelectRate(int x, int y);
		HDC hBackDC;
		HBITMAP hBackBmp, oldBmp;
		RECT winRect;

	protected:
		void OnControlDraw(HDC hdc);
		float select_timeline_rate;
		
		static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	public:
		CTimelineView(void);
		~CTimelineView(void);
		void Create(HWND OwnerhWnd, int x, int y, int width, int height, const char* szTitle);
		float GetSelectRate();//Timeline�Ō��ݑI�����Ă���rate(0.0 - 1.0)�̒l��Ԃ�
		void ForceDraw();
		float select_keyframe_rate;//���ݑI�����Ă���ҏW�Ώۂ̃A�j���[�V����>anim[n].anim[keyframe]
		void SetSelectRateIndirect(float rate);//newly added. - done by c.r.v.
		HDC GetBackDC();
};
