
// JamBotView.cpp : implementation of the CJamBotView class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "JamBot.h"
#endif

#include "JamBotDoc.h"
#include "JamBotView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CJamBotView

IMPLEMENT_DYNCREATE(CJamBotView, CView)

BEGIN_MESSAGE_MAP(CJamBotView, CView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CJamBotView::OnFilePrintPreview)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
END_MESSAGE_MAP()

// CJamBotView construction/destruction

CJamBotView::CJamBotView()
{
	// TODO: add construction code here

}

CJamBotView::~CJamBotView()
{
}

BOOL CJamBotView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CJamBotView drawing

void CJamBotView::OnDraw(CDC* /*pDC*/)
{
	CJamBotDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: add draw code for native data here
}


// CJamBotView printing


void CJamBotView::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS
	AFXPrintPreview(this);
#endif
}

BOOL CJamBotView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CJamBotView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CJamBotView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

void CJamBotView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CJamBotView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// CJamBotView diagnostics

#ifdef _DEBUG
void CJamBotView::AssertValid() const
{
	CView::AssertValid();
}

void CJamBotView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CJamBotDoc* CJamBotView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CJamBotDoc)));
	return (CJamBotDoc*)m_pDocument;
}
#endif //_DEBUG


// CJamBotView message handlers
