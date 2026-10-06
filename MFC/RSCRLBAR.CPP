// /home/rerun/Projects/migalley/RowanUI/RSCRLBAR.CPP
/*
	 MiG Alley
	 Copyright (C) 1998, 1999, 2000, 2001 Empire Interactive (Europe) Ltd,
	 677 High Road, North Finchley, London N12 0DA

	 Please see the document licence.doc for the full licence agreement

2. LICENCE
 2.1 	
 	Subject to the provisions of this Agreement we now grant to you the 
 	following rights in respect of the Source Code:
  2.1.1 
  	the non-exclusive right to Exploit  the Source Code and Executable 
  	Code on any medium; and 
  2.1.2 
  	the non-exclusive right to create and distribute Derivative Works.
 2.2 	
 	Subject to the provisions of this Agreement we now grant you the
	following rights in respect of the Object Code:
  2.2.1 
	the non-exclusive right to Exploit the Object Code on the same
	terms and conditions set out in clause 3, provided that any
	distribution is done so on the terms of this Agreement and is
	accompanied by the Source Code and Executable Code (as
	applicable).

3. GENERAL OBLIGATIONS
 3.1 
 	In consideration of the licence granted in clause 2.1 you now agree:
  3.1.1 
	that when you distribute the Source Code or Executable Code or
	any Derivative Works to Recipients you will also include the
	terms of this Agreement;
  3.1.2 
	that when you make the Source Code, Executable Code or any
	Derivative Works ("Materials") available to download, you will
	ensure that Recipients must accept the terms of this Agreement
	before being allowed to download such Materials;
  3.1.3 
	that by Exploiting the Source Code or Executable Code you may
	not impose any further restrictions on a Recipient's subsequent
	Exploitation of the Source Code or Executable Code other than
	those contained in the terms and conditions of this Agreement;
  3.1.4 
	not (and not to allow any third party) to profit or make any
	charge for the Source Code, or Executable Code, any
	Exploitation of the Source Code or Executable Code, or for any
	Derivative Works;
  3.1.5 
	not to place any restrictions on the operability of the Source 
	Code;
  3.1.6 
	to attach prominent notices to any Derivative Works stating
	that you have changed the Source Code or Executable Code and to
	include the details anddate of such change; and
  3.1.7 
  	not to Exploit the Source Code or Executable Code otherwise than
	as expressly permitted by  this Agreement.

questions about this file may be asked at http://www.simhq.com/
*/

// RSCRLBAR.CPP : implementation of the CRScrlBar class
// Replaced ActiveX wrapper with native implementation for MFC Stub / SDL

#include "WIN32_COMPAT.H"
#include "MFC_stub.h"
#include "DLGITEM.H"
#include "RDIALOG.H"
#include "RSCRLBAR.H"
#include "RESOURCE.H"
#include "MIGVIEW.H"
#include "MESSAGES.H"

#include <map>
#include <algorithm>

#ifndef SB_THUMBPOSITION
#define SB_THUMBPOSITION 4
#endif

#ifndef SB_THUMBTRACK
#define SB_THUMBTRACK 5
#endif

// Internal state storage since we can't modify the header
struct ScrlBarData {
	long minVal = 0;
	long maxVal = 100;
	long stepSize = 1;
	long pageSize = 10;
	long scrollPos = 0;
	BOOL horzAlign = FALSE;
	long parentPtr = 0;
	BOOL useMessages = FALSE;
	long fileNumOffset = 0;

	bool dragging = false;
	CPoint dragStartPt;
	long dragStartPos = 0;
};

static std::map<CWnd*, ScrlBarData> g_BarData;

static ScrlBarData& GetData(CWnd* pWnd) {
	return g_BarData[pWnd];
}

// Callers store either an HWND (BoB) or a CWnd* (MiG Alley) through this
// OCX-era property. HWNDs are registered tokens, so resolve that form
// authoritatively before treating the value as a CWnd*.
static CWnd* ResolveParent(ScrlBarData& data, CWnd* self)
{
	CWnd* pParent = nullptr;
	if (data.parentPtr)
	{
		if (WindowBackend* pbe = backend_from_hwnd((HWND)data.parentPtr))
			pParent = pbe->owner;
		else
			pParent = (CWnd*)data.parentPtr;
	}
	return pParent ? pParent : self->GetParent();
}

static void Draw3dRect(CDC* pDC, const CRect& rect, COLORREF clrTopLeft, COLORREF clrBottomRight)
{
	pDC->FillSolidRect(rect.left, rect.top, rect.Width() - 1, 1, clrTopLeft);
	pDC->FillSolidRect(rect.left, rect.top, 1, rect.Height() - 1, clrTopLeft);
	pDC->FillSolidRect(rect.right - 1, rect.top, 1, rect.Height(), clrBottomRight);
	pDC->FillSolidRect(rect.left, rect.bottom - 1, rect.Width(), 1, clrBottomRight);
}

/////////////////////////////////////////////////////////////////////////////
// CRScrlBar

IMPLEMENT_DYNCREATE(CRScrlBar, CWnd)

BEGIN_MESSAGE_MAP(CRScrlBar, CWnd)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_ERASEBKGND()
	ON_WM_DESTROY()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRScrlBar properties

long CRScrlBar::GetMinValue()
{
	return GetData(this).minVal;
}

void CRScrlBar::SetMinValue(long propVal)
{
	GetData(this).minVal = propVal;
	Invalidate();
}

long CRScrlBar::GetMaxValue()
{
	return GetData(this).maxVal;
}

void CRScrlBar::SetMaxValue(long propVal)
{
	GetData(this).maxVal = propVal;
	Invalidate();
}

long CRScrlBar::GetStepSize()
{
	return GetData(this).stepSize;
}

void CRScrlBar::SetStepSize(long propVal)
{
	GetData(this).stepSize = propVal;
}

BOOL CRScrlBar::GetHorzAlign()
{
	return GetData(this).horzAlign;
}

void CRScrlBar::SetHorzAlign(BOOL propVal)
{
	GetData(this).horzAlign = propVal;
	Invalidate();
}

long CRScrlBar::GetParentPointer()
{
	return GetData(this).parentPtr;
}

void CRScrlBar::SetParentPointer(long propVal)
{
	GetData(this).parentPtr = propVal;
}

long CRScrlBar::GetPageSize()
{
	return GetData(this).pageSize;
}

void CRScrlBar::SetPageSize(long propVal)
{
	GetData(this).pageSize = propVal;
	Invalidate();
}

long CRScrlBar::GetScrollPos()
{
	return GetData(this).scrollPos;
}

void CRScrlBar::SetScrollPos(long propVal)
{
	ScrlBarData& data = GetData(this);
	if (propVal < data.minVal) propVal = data.minVal;
	if (propVal > data.maxVal) propVal = data.maxVal;
	
	if (data.scrollPos != propVal) {
		data.scrollPos = propVal;
		Invalidate();
	}
}

BOOL CRScrlBar::GetUseMessagesInsteadOfEvents()
{
	return GetData(this).useMessages;
}

void CRScrlBar::SetUseMessagesInsteadOfEvents(BOOL propVal)
{
	GetData(this).useMessages = propVal;
}

long CRScrlBar::GetFileNumOffset()
{
	return GetData(this).fileNumOffset;
}

void CRScrlBar::SetFileNumOffset(long propVal)
{
	GetData(this).fileNumOffset = propVal;
}

/////////////////////////////////////////////////////////////////////////////
// CRScrlBar operations

void CRScrlBar::Move(long left, long top, long right, long bottom)
{
	MoveWindow(left, top, right - left, bottom - top);
}

/////////////////////////////////////////////////////////////////////////////
// CRScrlBar message handlers

void CRScrlBar::OnDestroy()
{
	g_BarData.erase(this);
	//CWnd::OnDestroy();
}

BOOL CRScrlBar::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}

void CRScrlBar::OnPaint()
{
	CPaintDC dc(this);
	ScrlBarData& data = GetData(this);
	CRect rect;
	GetClientRect(rect);

	// Draw background track
	dc.FillSolidRect(rect, RGB(60, 60, 60));

	long range = data.maxVal - data.minVal;
	if (range <= 0) return;

	// Calculate thumb geometry
	CRect thumbRect = rect;
	int trackLen, thumbLen, thumbPos;

	if (data.horzAlign) {
		trackLen = rect.Width();
		// Simple proportional thumb
		thumbLen = std::max<int>(20, (int)((double)data.pageSize / (range + data.pageSize) * trackLen));
		int availableTrack = trackLen - thumbLen;
		thumbPos = (int)((double)(data.scrollPos - data.minVal) / range * availableTrack);
		
		thumbRect.left += thumbPos;
		thumbRect.right = thumbRect.left + thumbLen;
		thumbRect.DeflateRect(0, 2);
	} else {
		trackLen = rect.Height();
		thumbLen = std::max<int>(20, (int)((double)data.pageSize / (range + data.pageSize) * trackLen));
		int availableTrack = trackLen - thumbLen;
		thumbPos = (int)((double)(data.scrollPos - data.minVal) / range * availableTrack);

		thumbRect.top += thumbPos;
		thumbRect.bottom = thumbRect.top + thumbLen;
		thumbRect.DeflateRect(2, 0);
	}

	// Draw thumb
	dc.FillSolidRect(thumbRect, RGB(120, 120, 120));
	Draw3dRect(&dc, thumbRect, RGB(160, 160, 160), RGB(40, 40, 40));
}

void CRScrlBar::OnLButtonDown(UINT nFlags, CPoint point)
{
	SetCapture();
	ScrlBarData& data = GetData(this);
	
	// Check if clicked on thumb
	CRect rect;
	GetClientRect(rect);
	long range = data.maxVal - data.minVal;
	if (range <= 0) return;

	int trackLen, thumbLen, thumbPos;
	if (data.horzAlign) {
		trackLen = rect.Width();
		thumbLen = std::max<int>(20, (int)((double)data.pageSize / (range + data.pageSize) * trackLen));
		int availableTrack = trackLen - thumbLen;
		thumbPos = (int)((double)(data.scrollPos - data.minVal) / range * availableTrack);
		
		CRect thumbRect = rect;
		thumbRect.left += thumbPos;
		thumbRect.right = thumbRect.left + thumbLen;

		if (thumbRect.PtInRect(point)) {
			data.dragging = true;
			data.dragStartPt = point;
			data.dragStartPos = data.scrollPos;
		} else {
			// Page up/down logic could go here
			long newPos = data.scrollPos;
			if (point.x < thumbRect.left) newPos -= data.pageSize;
			else if (point.x > thumbRect.right) newPos += data.pageSize;
			SetScrollPos(newPos);
			
			// Notify parent
			CWnd* pParent = ResolveParent(data, this);
			if (pParent) {
				if (data.useMessages)
					pParent->SendMessage(WM_ACTIVEXSCROLL, data.scrollPos, data.horzAlign);
				else
					pParent->SendMessage(data.horzAlign ? WM_HSCROLL : WM_VSCROLL, 
						MAKELPARAM(SB_THUMBPOSITION, 0), (LPARAM)this);
			}
		}
	} else {
		trackLen = rect.Height();
		thumbLen = std::max<int>(20, (int)((double)data.pageSize / (range + data.pageSize) * trackLen));
		int availableTrack = trackLen - thumbLen;
		thumbPos = (int)((double)(data.scrollPos - data.minVal) / range * availableTrack);

		CRect thumbRect = rect;
		thumbRect.top += thumbPos;
		thumbRect.bottom = thumbRect.top + thumbLen;

		if (thumbRect.PtInRect(point)) {
			data.dragging = true;
			data.dragStartPt = point;
			data.dragStartPos = data.scrollPos;
		} else {
			// Page up/down
			long newPos = data.scrollPos;
			if (point.y < thumbRect.top) newPos -= data.pageSize;
			else if (point.y > thumbRect.bottom) newPos += data.pageSize;
			SetScrollPos(newPos);

			// Notify parent
			CWnd* pParent = ResolveParent(data, this);
			if (pParent) {
				if (data.useMessages)
					pParent->SendMessage(WM_ACTIVEXSCROLL, data.scrollPos, data.horzAlign);
				else
					pParent->SendMessage(data.horzAlign ? WM_HSCROLL : WM_VSCROLL, 
						MAKELPARAM(SB_THUMBPOSITION, 0), (LPARAM)this);
			}
		}
	}
}

void CRScrlBar::OnMouseMove(UINT nFlags, CPoint point)
{
	ScrlBarData& data = GetData(this);
	if (data.dragging) {
		CRect rect;
		GetClientRect(rect);
		long range = data.maxVal - data.minVal;
		if (range <= 0) return;

		int trackLen, thumbLen;
		long deltaPos = 0;

		if (data.horzAlign) {
			trackLen = rect.Width();
			thumbLen = std::max<int>(20, (int)((double)data.pageSize / (range + data.pageSize) * trackLen));
			int availableTrack = trackLen - thumbLen;
			if (availableTrack > 0) {
				int pixelDelta = point.x - data.dragStartPt.x;
				deltaPos = (long)((double)pixelDelta / availableTrack * range);
			}
		} else {
			trackLen = rect.Height();
			thumbLen = std::max<int>(20, (int)((double)data.pageSize / (range + data.pageSize) * trackLen));
			int availableTrack = trackLen - thumbLen;
			if (availableTrack > 0) {
				int pixelDelta = point.y - data.dragStartPt.y;
				deltaPos = (long)((double)pixelDelta / availableTrack * range);
			}
		}

		long newPos = data.dragStartPos + deltaPos;
		SetScrollPos(newPos);

		// Notify parent
		CWnd* pParent = ResolveParent(data, this);
		if (pParent) {
			if (data.useMessages)
				pParent->SendMessage(WM_ACTIVEXSCROLL, data.scrollPos, data.horzAlign);
			else
				pParent->SendMessage(data.horzAlign ? WM_HSCROLL : WM_VSCROLL, 
						MAKELPARAM(SB_THUMBTRACK, 0), (LPARAM)this);
		}
	}
}

void CRScrlBar::OnLButtonUp(UINT nFlags, CPoint point)
{
	ReleaseCapture();
	GetData(this).dragging = false;
}
