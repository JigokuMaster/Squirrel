/*
   ============================================================================
Name		: SquirrelAppView.cpp
Author	  : JigokuMaster
Copyright   : Your copyright notice
Description : Application view implementation
============================================================================
*/

// INCLUDE FILES
#include <coemain.h>
#include <Squirrel.rsg>
#include <AknQueryDialog.h>
#include "SquirrelAppUi.h"
#include "SquirrelAppView.h"
#include "Squirrel.hrh"
#include "Squirrel.mbg"
#include "SquirrelDocument.h"
#include "SquirrelApplication.h"

#define KNumberOfItems 5

_LIT(KMainViewIconsFile, "\\resource\\apps\\Squirrel_ui.mif");


CMainViewGrid::CMainViewGrid()
{
    iFont = NULL;
    iGrid = NULL;
}


CMainViewGrid::~CMainViewGrid()
{
    if(iGrid) delete iGrid;
    iGrid = NULL;
}

void CMainViewGrid::ConstructL(const TRect& aRect)
{
    CreateWindowL();
    iGrid = new (ELeave)CAknGrid;
    iGrid->SetContainerWindowL(*this);
    TResourceReader reader;
    CEikonEnv::Static()->CreateResourceReaderLC(reader, R_MAINVIEW_GRID);
    iGrid->ConstructFromResourceL(reader);
    CleanupStack::PopAndDestroy(); // reader
    SetupGridIconsL();
    SetRect(aRect);
    ActivateL();
}

TInt CMainViewGrid::CountComponentControls() const
{
    return iGrid ? 1 : 0;
}


CCoeControl* CMainViewGrid::ComponentControl(TInt aIndex) const
{
    switch ( aIndex )
    {
	case 0:
	    return iGrid;
	default:
	    return NULL;
    }
}

void CMainViewGrid::SizeChanged()
{
    if (iGrid)
    {
	iGrid->SetRect(Rect());
	SetupGrid();
    }
}

TKeyResponse CMainViewGrid::OfferKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType)
{
    
    if (iGrid) return iGrid->OfferKeyEventL(aKeyEvent,aType);

    return CCoeControl::OfferKeyEventL(aKeyEvent,aType) /*EKeyWasNotConsumed*/;

}

void CMainViewGrid::HandlePointerEventL(const TPointerEvent &aPointerEvent)
{
    CCoeControl::HandlePointerEventL(aPointerEvent);

    if (iGrid) iGrid->HandlePointerEventL(aPointerEvent);

}

void CMainViewGrid::AddIconL(const TDesC& aFilePath, TInt aBitmapId, TInt aMaskId)
{
    CArrayPtr<CGulIcon>* iconArray = iGrid->ItemDrawer()->FormattedCellData()->IconArray();

    if (!iconArray) return;
    if (aMaskId == -1) aMaskId = aBitmapId+1;

   
    CFbsBitmap *bm, *mask;
    AknIconUtils::CreateIconL(bm, mask, aFilePath, aBitmapId, aMaskId);
    iconArray->AppendL(CGulIcon::NewL(bm, mask));
}

void CMainViewGrid::SetFont(const CFont *aFont)
{
    iFont = aFont;
}

void CMainViewGrid::SetupGrid()
{
  
    //TInt cellWidth = iGrid->ColumnWidth();
    //TInt cellHeight = iGrid->ItemHeight();
    
    AknListBoxLayouts::SetupStandardGrid(*iGrid);
    // Set grid layout
    TBool verticalOrientation = EFalse;
    TBool leftToRight = ETrue;
    TBool topToBottom = ETrue;
    TInt columns = 3, rows = 2;

    // Determinate cell size
    TSize mpSize;     // main pane size
    AknLayoutUtils::LayoutMetricsSize(AknLayoutUtils::EMainPane, mpSize);
    //mpSize = Size();
    TInt baseSize = Min(mpSize.iWidth/columns, mpSize.iHeight/rows);
    TSize cellSize(baseSize, baseSize);
   
    iGrid->SetLayoutL(verticalOrientation, 
        leftToRight, topToBottom, 
        columns, rows,  
        cellSize);

    //TRect gfxRect(0,0, cellSize.iWidth, cellSize.iHeight/2);

    TRect gfxRect(0,0, baseSize/2, baseSize/2);

    AknListBoxLayouts::SetupFormGfxCell(
       *iGrid,			// Reference to grid control
       iGrid->ItemDrawer(),	// Pointer to the item drawer
       0,			// Column index
       gfxRect.Width()/2,	// Left position
       gfxRect.Height()/2,	// Top position
       0,			// Right - unused
       0,			// Bottom - unused
       gfxRect.Width(),		// Width
       gfxRect.Height(),	// Height
       gfxRect.iTl,		// Start position
       gfxRect.iBr);		// End position   

    if (!iFont) iFont = const_cast<CFont*>(AknLayoutUtils::FontFromId(EAknLogicalFontPrimaryFont, NULL));

    // Deretminate the baseline for the text
    TInt baseline = (cellSize.iHeight) - iFont->DescentInPixels()-1;
 
    //TRect textRect(0,0, 0, 0);
    AknListBoxLayouts::SetupFormTextCell(
       *iGrid,					// Reference to grid
       iGrid->ItemDrawer(),			// Pointer to the item drawer
       1,					// Column index, no idea how to find the correct index ????
       iFont,					// Font
       0,					// Color index
       0,					// Left margin
       0,					// Right margin - unused
       baseline,				// Baseline
       cellSize.iWidth,				// Text width
       CGraphicsContext::ECenter, 		// Text alignment
       TPoint(0, cellSize.iHeight/2), 		// Start position
       TPoint(cellSize.iWidth, cellSize.iHeight));  // End position


    TRgb textColor; 
    AknsUtils::GetCachedColor(AknsUtils::SkinInstance(), textColor, KAknsIIDQsnTextColors, EAknsCIQsnTextColorsCG6);

    // iGrid->ItemDrawer()->SetTextColor(textColor); // obvious function, but it has no effect!!!
    // took me ages to find out how to set skinned text color correctly ...
    CFormattedCellListBoxData::TColors colors = iGrid->ItemDrawer()->FormattedCellData()->SubCellColors(1);
    colors.iText = textColor;
    iGrid->ItemDrawer()->ColumnData()->SetSubCellColorsL(1, colors);
    // try to center the grid
    TRect viewRect = iGrid->View()->ViewRect(); 
    TInt gap = 3;
    TInt viewWidth = (cellSize.iWidth*columns) + gap;
    TInt x = mpSize.iWidth-viewWidth;
    viewRect.Move(TPoint(x/2, 0));   
    iGrid->View()->SetViewRect(viewRect);
    CEikScrollBarFrame* frame = iGrid->ScrollBarFrame();
    if (frame) 
	TRAP_IGNORE(frame->SetScrollBarVisibilityL(CEikScrollBarFrame::EOff,CEikScrollBarFrame::EOff));
    //iGrid->UpdateScrollBarsL();
    
    //TInt iconBaseSize = Min(cellSize.iWidth, cellSize.iHeight/2);
    //CArrayPtr<CGulIcon>* iconArray = iGrid->ItemDrawer()->FormattedCellData()->IconArray();
    //AknIconUtils::SetSize((*iconArray)[0]->Bitmap(), TSize(iconBaseSize, iconBaseSize));


}

void CMainViewGrid::SetupGridIconsL()
{
    CArrayPtr<CGulIcon>* icons = new (ELeave) CAknIconArray(KNumberOfItems);
    iGrid->ItemDrawer()->FormattedCellData()->SetIconArray(icons);

    TInt icon_ids[] = {
	    EMbmSquirrelScan,
	    EMbmSquirrelGenerate,	
	    EMbmSquirrelDecode,
	    EMbmSquirrelHistory,
	    EMbmSquirrelAbout
    };
    for (TInt i = 0; i < KNumberOfItems; i++)
    {
	AddIconL(KMainViewIconsFile, icon_ids[i]);
    }
}



// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CSquirrelAppView::NewL()
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CSquirrelAppView* CSquirrelAppView::NewL()
{
    CSquirrelAppView* self = CSquirrelAppView::NewLC();
    CleanupStack::Pop(self);
    return self;
}

// -----------------------------------------------------------------------------
// CSquirrelAppView::NewLC()
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CSquirrelAppView* CSquirrelAppView::NewLC()
{
    CSquirrelAppView* self = new (ELeave) CSquirrelAppView;
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
}

// -----------------------------------------------------------------------------
// CSquirrelAppView::ConstructL()
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CSquirrelAppView::ConstructL()
{
    iGridListbox = NULL;
    iListbox = NULL;
    BaseConstructL(R_MAIN_VIEW);
    iUseGrid = EFalse;
}

// -----------------------------------------------------------------------------
// CSquirrelAppView::CSquirrelAppView()
// C++ default constructor can NOT contain any code, that might leave.
// -----------------------------------------------------------------------------
//
CSquirrelAppView::CSquirrelAppView()
{
    // No implementation required
}

// -----------------------------------------------------------------------------
// CSquirrelAppView::~CSquirrelAppView()
// Destructor.
// -----------------------------------------------------------------------------
//
CSquirrelAppView::~CSquirrelAppView()
{
   
    if (iGridListbox)
    {
	AppUi()->RemoveFromStack( iGridListbox );
	delete iGridListbox;
	iGridListbox = NULL;
    } 

    if (iListbox)
    {
	AppUi()->RemoveFromStack(iListbox);
	delete iListbox;
	iListbox = NULL;
    } 
}


TUid CSquirrelAppView::Id() const 
{ 
    return TUid::Uid(EMainView); 
}

void CSquirrelAppView::DynInitMenuPaneL(TInt aResourceId ,CEikMenuPane* aMenuPane)
{
    if (aResourceId == R_LANGUAGE_SUBMENU)
    {
	TInt item = SelectedLanguageItemL();
	if (item) aMenuPane->SetItemButtonState(item, EEikMenuItemSymbolOn);
    }
    else AppUi()->DynInitMenuPaneL(aResourceId , aMenuPane);

}

void CSquirrelAppView::HandleCommandL( TInt aCommand )
{

    switch(aCommand)
    {
	case ECmdSelectLanguage1:
	case ECmdSelectLanguage14:
	case ECmdSelectLanguage27:
	case ECmdSelectLanguage40:
	case ECmdSelectLanguage96:
	case ECmdSelectLanguage16:
	    ChangeLanguageL(aCommand);
	    break;
	default:
	    AppUi()->HandleCommandL(aCommand);
	    break;
    }
}

void CSquirrelAppView::HandleViewRectChange()
{
    if (iUseGrid && iGridListbox)
    {
	iGridListbox->SetRect(ClientRect());
	AppUi()->ActivateLocalViewL(TUid::Uid(EDummyView));
    }
    else if (iListbox) iListbox->SetRect(ClientRect());
}


void CSquirrelAppView::DoActivateL( const TVwsViewId& /*aPrevViewId*/,
	TUid /*aCustomMessageId*/,
	const TDesC8& /*aCustomMessage*/ )
{
    TSize size = AppUi()->ApplicationRect().Size();
    iUseGrid = (size.iWidth == 320) && (size.iHeight == 240);
    SetAppTitleL(NULL, R_APP_TITLE);

    if (iUseGrid)
    {
	iGridListbox = new (ELeave) CMainViewGrid;
	iGridListbox->ConstructL(ClientRect());
	iGridListbox->SetFont(iEikonEnv->DenseFont());
	iGridListbox->Grid()->SetListBoxObserver(this);
	iGridListbox->SetMopParent(this);
	AppUi()->AddToStackL(*this, iGridListbox);
	iGridListbox->SetRect(ClientRect());
    }
    else
    {
	iListbox = new (ELeave) TListBox<SINGLE_LARGE_STYLE>;
	iListbox->ConstructL(NULL, ClientRect(), R_MAINVIEW_LISTBOX);
	CArrayPtr<CGulIcon>* iconArray = new (ELeave) CAknIconArray(KNumberOfItems);
	iListbox->Listbox()->ItemDrawer()->ColumnData()->SetIconArray(iconArray);
	iListbox->Listbox()->ItemDrawer()->ColumnData()->EnableMarqueeL(ETrue);
	CEikScrollBarFrame* frame = iListbox->Listbox()->ScrollBarFrame();
	if (frame) TRAP_IGNORE(frame->SetScrollBarVisibilityL(CEikScrollBarFrame::EOff,CEikScrollBarFrame::EAuto));

	TInt icon_ids[] = {
	    EMbmSquirrelScan,
	    EMbmSquirrelGenerate,	
	    EMbmSquirrelDecode,
	    EMbmSquirrelHistory,
	    EMbmSquirrelAbout
	};
	for (TInt i = 0; i < KNumberOfItems; i++)
	{
	    iListbox->AddIconL(iconArray, KMainViewIconsFile, icon_ids[i]);
	}
	iListbox->SetObserver(this);
	iListbox->SetMopParent(this);
	AppUi()->AddToStackL(*this, iListbox);

    }
}

void CSquirrelAppView::DoDeactivate()
{

    if (iGridListbox)
    {
	AppUi()->RemoveFromViewStack( *this, iGridListbox );
	delete iGridListbox;
	iGridListbox = NULL;
    }
    if (iListbox)
    {
	AppUi()->RemoveFromStack(iListbox);
	delete iListbox;
	iListbox = NULL;
    } 
}

void CSquirrelAppView::HandleListBoxEventL(CEikListBox* aListBox , TListBoxEvent aEventType)
{
    if ((aEventType == EEventEnterKeyPressed || aEventType == EEventItemClicked))
    {

	TInt viewIndex = -1;
	if (iGridListbox && iUseGrid) viewIndex = iGridListbox->Grid()->CurrentItemIndex() + 2;
	else if (iListbox) viewIndex = iListbox->Listbox()->CurrentItemIndex() + 2;

	if (viewIndex == EDecoderView)
	{
	    AppUi()->ActivateLocalViewL(TUid::Uid(EDecoderView), TUid::Uid(ECmdOpenImage), KNullDesC8);
	}

	else AppUi()->ActivateLocalViewL(TUid::Uid(viewIndex));

    }
}


TInt CSquirrelAppView::SelectedLanguageItemL()
{
    CEikAppUi* appui = reinterpret_cast<CEikAppUi*>(iEikonEnv->AppUi());
    CSquirrelDocument* doc = static_cast<CSquirrelDocument*>(appui->Document());
    TUint32 lang = static_cast<CSquirrelApplication*>(doc->Application())->iCurrentLanguage;
    TInt item = 0;
    switch (lang)
    {
	case ELangEnglish:
	    item = ECmdSelectLanguage1;
	    break;
	case ELangTurkish:
	    item = ECmdSelectLanguage14;
	    break;
	case ELangBelarussian:
	    item = ECmdSelectLanguage40;
	    break;
	case ELangPolish:
	    item = ECmdSelectLanguage27;
	    break;
	case ELangVietnamese:
	    item = ECmdSelectLanguage96;
	    break;
	case ELangRussian:
	    item = ECmdSelectLanguage16;
	    break;

	default: break;
    }
    return item;

}


void CSquirrelAppView::ChangeLanguageL(TInt aCommand)
{

    TLanguage lang = ELangTest;
    switch (aCommand)
    {
	case ECmdSelectLanguage1:
	    lang = ELangEnglish;
	    break;
	case ECmdSelectLanguage14:
	    lang = ELangTurkish;
	    break;
	case ECmdSelectLanguage40:
	    lang = ELangBelarussian;
	    break;
	case ECmdSelectLanguage27:
	    lang = ELangPolish;
	    break;
	case ECmdSelectLanguage96:
	    lang = ELangVietnamese;
	    break;
	case ECmdSelectLanguage16:
	    lang = ELangRussian;
	    break;

	default:
	    break;
    }

    if (lang)
    {

	CEikAppUi* appui = reinterpret_cast<CEikAppUi*>(iEikonEnv->AppUi());
	CSquirrelDocument* doc = static_cast<CSquirrelDocument*>(appui->Document());
	TBool ok = static_cast<CSquirrelApplication*>(doc->Application())->SaveLanguageL(lang);
	if (ok)
	{
	    CAknQueryDialog* exitQuery = CAknQueryDialog::NewL();
	    HBufC* prompt = iCoeEnv->AllocReadResourceLC(R_RESTART_CONFIRMATION_QUERY_PROMPT);
	    exitQuery->SetPromptL(*prompt);  
	    CleanupStack::PopAndDestroy(prompt);
            if (exitQuery->ExecuteLD(R_GENERAL_CONFIRMATION_QUERY)) AppUi()->Exit();
	}
    }
}



// End of File
