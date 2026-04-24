#include "uicommons.h"
#include <aknnotewrappers.h>
#include <eikfrlbd.h>

#include <avkon.mbg>
#include <aknconsts.h>
#include <aknlists.h>
#include <aknpopup.h>
#include <bautils.h>
// needed by SetAppTitle
#include <akntitle.h>
#include <eikspane.h> 

static void DoFormatSize(TDes &aDes, TInt64 aSize)
{
    wchar_t* sizeSnits[] = {L"B", L"KB", L"MB", L"GB", L"TB"};
    TReal size = (TReal)aSize;
    for (TInt i=0; i < 5; i++){
	if (size < 1024.0)
	{
            aDes.Format(_L("%.2f%s"), size, sizeSnits[i]);
	    break;
	}
        size /= 1024.0;
    } 
}

TBool DriveSelectionL(TDes &aDrivePath, const TInt aTitleResId,
	const TBool aShowDZ, const TBool aShowReadOnly)
{

    CAknDoubleLargeGraphicPopupMenuStyleListBox* list;

    list = new(ELeave) CAknDoubleLargeGraphicPopupMenuStyleListBox;  
    CleanupStack::PushL(list);

    CAknPopupList* popupList;
    popupList = CAknPopupList::NewL(list, R_AVKON_SOFTKEYS_SELECT_CANCEL, AknPopupLayouts::EMenuDoubleLargeGraphicWindow);   
    CleanupStack::PushL(popupList);

    HBufC* title = CCoeEnv::Static()->AllocReadResourceLC(aTitleResId);

    popupList->SetTitleL(*title);
    CleanupStack::PopAndDestroy(title);

    TInt flags = 0; // Initialize flag
    list->ConstructL(popupList, flags);
    list->CreateScrollBarFrameL(ETrue);
    list->ScrollBarFrame()->SetScrollBarVisibilityL(CEikScrollBarFrame::EOff,CEikScrollBarFrame::EAuto);
    list->ItemDrawer()->FormattedCellData()->EnableMarqueeL(ETrue);
    CDesCArrayFlat* items = new CDesCArrayFlat(3);
    CleanupStack::PushL(items);
 
    CArrayPtr<CGulIcon>* icons =  new ( ELeave ) CAknIconArray(3);
    CleanupStack::PushL(icons);

    // Add drives
    RFs& rfs = CCoeEnv::Static()->FsSession();
    CDesCArrayFlat* driveList = new CDesCArrayFlat(3);
    BaflUtils::GetDiskListL(rfs, *driveList);
 
    //CleanupStack::PushL(driveList); // KERN-EXEC3 when this function returns!!
   
    TInt i;
    TInt iconIndex = 0;
    if (!aShowDZ)
    {
	if ( driveList->Find(_L("D"), i) == 0 )
	{
	    driveList->Delete(i);
	}
	if ( driveList->Find(_L("Z"), i) == 0 )
	{
	    driveList->Delete(i);
	}
    }

    for (i = 0; i < driveList->MdcaCount(); i++)
    {

	TPtrC d = driveList->MdcaPoint(i); 	
	TBuf<3> root(d);
	root.Append(_L(":\\"));
	TBool rootOK(EFalse);

	if (!aShowReadOnly && BaflUtils::PathExists(rfs, root))
	{
	    
	    TBool isRO(EFalse); 
	    if (BaflUtils::DiskIsReadOnly(rfs, root, isRO) == KErrNone)
	    {
		rootOK = !isRO;	
	    }
    
	}

	if (!rootOK)
	{
	    driveList->Delete(i);
	    continue;
	}

	TBuf<KMaxFileName+64> item;
	TBuf<64> sizeInfo1;
	//TBuf<64> sizeInfo2;
	TVolumeInfo driveInfo;
	TInt driveNumber;

 	rfs.CharToDrive(d[0], driveNumber); 
        if (rfs.Volume(driveInfo, driveNumber) != KErrNone)
	{
	    driveList->Delete(i);
	    continue;
	}
	
	TBuf<KMaxFileName> driveName = driveInfo.iName; // Optional name of the volume
	
	if (driveName.Length() == 0)
	{
	    TInt mediaType = driveInfo.iDrive.iType;
            if (mediaType == EMediaNANDFlash){ 
		if (d[0] == 'C') driveName = _L("Phone memory");
		else  driveName = _L("NAND flash");
	    }

	    if (mediaType == EMediaHardDisk) driveName = _L("Mass storage");
	    else if (mediaType == EMediaRemote) driveName = _L("Remote");
	}

	DoFormatSize(sizeInfo1, driveInfo.iFree);
        //DoFormatSize(sizeInfo2, driveInfo.iSize);
	item.Format(_L("%d\t%S: %S\tFree: %S\t"), iconIndex, &d, &driveName, &sizeInfo1);
	items->AppendL(item);
	iconIndex++;

	CFbsBitmap *bm, *mask;
	if (d[0] == 'F' || d[0] == 'E')
	{
	    AknIconUtils::CreateIconL(bm, mask, AknIconUtils::AvkonIconFileName(), EMbmAvkonQgn_prop_mmc_memc_large, EMbmAvkonQgn_prop_mmc_memc_large_mask);
  
	}

	else
	{
    
	    AknIconUtils::CreateIconL(bm, mask, AknIconUtils::AvkonIconFileName(), EMbmAvkonQgn_prop_phone_memc_large, EMbmAvkonQgn_prop_phone_memc_mask);
    
	}
	icons->AppendL(CGulIcon::NewL(bm, mask));	
    }

#if 0
    for (i = 0; i < driveList->MdcaCount(); i++)
    {

	TPtrC d = driveList->MdcaPoint(i);
	TBuf<3> dPath;
	dPath.Format(_L("%S:\\"), &d);
	if (!BaflUtils::PathExists(rfs, dPath)){
	    driveList->Delete(i);
	    continue;
	}

	TBuf<10> item;
	item.Format(_L("%d\t%S\t"), iconIndex, &d);
	
	iconIndex++;
	items->AppendL(item);
	
	CFbsBitmap *bm, *mask;
	if (d[0] == 'F' || d[0] == 'E')
	{
	    AknIconUtils::CreateIconL(bm, mask, AknIconUtils::AvkonIconFileName(), EMbmAvkonQgn_prop_mmc_memc_large, EMbmAvkonQgn_prop_mmc_memc_large_mask);
  
	}

	else
	{
    
	    AknIconUtils::CreateIconL(bm, mask, AknIconUtils::AvkonIconFileName(), EMbmAvkonQgn_prop_phone_memc_large, EMbmAvkonQgn_prop_phone_memc_mask);
    
	}
	icons->AppendL(CGulIcon::NewL(bm, mask));	
    }
#endif

CTextListBoxModel* model = list->Model();
    model->SetItemTextArray(items);
    model->SetOwnershipType(ELbmOwnsItemArray);
    CleanupStack::Pop(items);

    list->ItemDrawer()->FormattedCellData()->SetIconArrayL(icons);
    CleanupStack::Pop(icons);
    
    TInt popupOk = popupList->ExecuteLD();
    CleanupStack::Pop(popupList);

    if (popupOk)
    {

    	TInt index = list->CurrentItemIndex();
	aDrivePath.Copy(driveList->MdcaPoint(index));
	aDrivePath.Append(_L(":\\"));
	
    }

    CleanupStack::PopAndDestroy(list); // list
    delete driveList;
    //CleanupStack::PopAndDestroy(driveList); 
    return popupOk;   
}


void ErrorNoteL(const TDesC *aText, TInt aResourceId)
{
    
    CAknErrorNote* errorNote;

    errorNote = new (ELeave) CAknErrorNote(ETrue);
    
    if (aText){
	errorNote->ExecuteLD(*aText);
    }

    else if (aResourceId != 0)
    {
	HBufC *textResource = CCoeEnv::Static()->AllocReadResourceLC(aResourceId);
	
	errorNote->ExecuteLD(*textResource);
	CleanupStack::PopAndDestroy(textResource);
    }
}

void SetAppTitleL(const TDesC *aTitle, TInt aResourceId)
{
    CEikStatusPane* statusPane = CEikonEnv::Static()->AppUiFactory()->StatusPane();

    if (statusPane)
    {
	CAknTitlePane* titlePane = static_cast<CAknTitlePane*>(statusPane->ControlL(TUid::Uid(EEikStatusPaneUidTitle)));
	if (!titlePane) return;
	if (aTitle) titlePane->SetTextL(*aTitle);
	else if (aResourceId != 0)
	{
	    HBufC *textResource = CCoeEnv::Static()->AllocReadResourceLC(aResourceId);
	    titlePane->SetTextL(*textResource);
	    CleanupStack::PopAndDestroy(textResource);
	}
    }
}

// Implementation of CBitmapDrawer

CBitmapDrawer::CBitmapDrawer()
{
    iBitmap = NULL;
    iBitmapDevice = NULL;
    iGc = NULL;  
    iOwnBitmap = ETrue;
}

CBitmapDrawer::~CBitmapDrawer()
{
    Reset();
}


CBitmapDrawer* CBitmapDrawer::NewL(TSize aSize)
{

    CBitmapDrawer*  self = new (ELeave) CBitmapDrawer;
    CleanupStack::PushL(self);
    self->ConstructL(aSize);
    CleanupStack::Pop(self);
    return self;
}


CBitmapDrawer* CBitmapDrawer::NewL(CFbsBitmap* aBitmap)
{

    CBitmapDrawer*  self = new (ELeave) CBitmapDrawer;
    CleanupStack::PushL(self);
    self->ConstructL(aBitmap);
    CleanupStack::Pop(self);
    return self;
}


void CBitmapDrawer::Reset()
{
    if (iBitmap && iOwnBitmap){
	delete iBitmap;
	iBitmap = NULL;
    }
    if (iBitmapDevice){
	delete iBitmapDevice;
	iBitmapDevice = NULL;
    }
    if (iGc){
	delete iGc;
	iGc = NULL;
    }
}

void CBitmapDrawer::ConstructL(TSize aSize)
{
    Reset();
    iSize = aSize;
    iBitmap = new (ELeave) CFbsBitmap;
    iOwnBitmap = ETrue;
    User::LeaveIfError(iBitmap->Create(aSize, EColor16M));
    CleanupStack::PushL(iBitmap);

    iBitmapDevice = CFbsBitmapDevice::NewL(iBitmap);
    CleanupStack::PushL(iBitmapDevice);
    User::LeaveIfError(iBitmapDevice->CreateContext(iGc));
    CleanupStack::PushL(iGc);
    ClearL();
    CleanupStack::Pop(3); // iBitmap, iBitmapDevice, iGc
}


void CBitmapDrawer::ConstructL(CFbsBitmap* aBitmap)
{

    Reset();
    iSize = aBitmap->SizeInPixels();
    iBitmap = aBitmap;
    iOwnBitmap = EFalse;

    CleanupStack::PushL(iBitmap);
    iBitmapDevice = CFbsBitmapDevice::NewL(iBitmap);
    CleanupStack::PushL(iBitmapDevice);
    User::LeaveIfError(iBitmapDevice->CreateContext(iGc));

    CleanupStack::Pop(2); // iBitmap, iBitmapDevice

}


void CBitmapDrawer::SetBitmap(CFbsBitmap* aBitmap)
{
    Reset();
    iBitmap = aBitmap;
    iOwnBitmap = EFalse;
}

void CBitmapDrawer::SetBrushColorL(const TRgb &aColor, TBool aNoBorder)
{
    User::LeaveIfNull(iBitmap);
    User::LeaveIfNull(iGc);
    iGc->SetBrushColor(aColor);
    iGc->SetBrushStyle(CGraphicsContext::ESolidBrush);
}

void CBitmapDrawer::ClearL(const TRgb &aColor)
{
    SetBrushColorL(aColor);
    iGc->DrawRect(TRect(iBitmap->SizeInPixels()));
}

void CBitmapDrawer::ClearL()
{
    ClearL(KRgbWhite);
}

void CBitmapDrawer::DrawRectL(const TRect &aRect)
{
    User::LeaveIfNull(iGc);
    iGc->DrawRect(aRect);
}


void CBitmapDrawer::DrawRectL(const TRect &aRect, const TRgb &aColor)
{
    User::LeaveIfNull(iGc);

    iGc->SetBrushColor(aColor);
    iGc->SetBrushStyle(CGraphicsContext::ESolidBrush);
    iGc->DrawRect(aRect);
}


void CBitmapDrawer::ResizeL(const TSize &aSize)
{
    iSize = aSize;
    User::LeaveIfNull(iBitmap);
    User::LeaveIfNull(iBitmapDevice);

    User::LeaveIfError(iBitmap->Resize(aSize));
    User::LeaveIfError(iBitmapDevice->Resize(aSize));
    if (iGc)
    {
	delete iGc;
	iGc = NULL;
    }
    User::LeaveIfError(iBitmapDevice->CreateContext(iGc));
}

CFbsBitmap* CBitmapDrawer::Bitmap()
{
    return iBitmap;
}

CGraphicsContext* CBitmapDrawer::Gc()
{
    return iGc;
}

TSize CBitmapDrawer::Size()
{
    return iSize;
}



// Implementation of CViewContainer

CViewContainer::CViewContainer()
{
    iSkinned = EFalse;
}

CViewContainer::~CViewContainer()
{
    if (iBgContext) delete iBgContext;
}

void CViewContainer::ConstructL(const TRect& aRect, const TBool aSkinned)
{
    iSkinned = aSkinned;
    InitComponentArrayL();
    CreateWindowL();
    SetRect(aRect);
    //if (iSkinned) SetupSkinBgContext();
    ActivateL();
}

void CViewContainer::SetupSkinBgContext()
{
    TSize size = Size();
    TRect rect;
   
    AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::EMainPane, rect);

    /* full or normal screen */
    TAknsItemID bgId = (size.iHeight > rect.Height()) ? KAknsIIDQsnBgScreen : KAknsIIDQsnBgAreaMain;

    if (!iBgContext)
    {
	/* without parent absolute layout */
	iBgContext = CAknsBasicBackgroundControlContext::NewL(bgId, Rect(), EFalse/*The skinned background won't be drawn or updated correctly if ETrue is set here. */);
    }
    else
    {
	iBgContext->SetRect(Rect());
	iBgContext->SetBitmap(bgId);
    }	
}

void CViewContainer::AddControlL(CCoeControl* aControl)
{
    if (!ComponentArrayExists()) return;

    Components().AppendLC(aControl);
    CleanupStack::Pop();
}

void CViewContainer::UpdateControl(const TInt &aIndex, const TBool &aUpdateRect)
{
    if (!ComponentArrayExists()) return;
    if (aIndex < 0 || aIndex >= Components().Count()) return;


    CCoeControl* c = Components().At(aIndex).iControl;
    if (c){
	if (aUpdateRect) c->SetRect(Rect());
	c->DrawNow();
    }
    DrawNow();
}

void CViewContainer::SizeChanged()
{

    if (iSkinned) SetupSkinBgContext();

#if 0
    if (!ComponentArrayExists()) return;
    for (TInt i = 0; i < Components().Count(); i++)
    {

	CCoeControl* c = Components().At(i).iControl;

	if (c/* && c->IsFocused()*/) c->SetRect(Rect());
    }
#endif
}

CCoeControl*  CViewContainer::FocusedControl()
{
    if (!ComponentArrayExists()) return NULL;

    CCoeControl* c = NULL; 
    for (TInt i = 0; i < Components().Count(); i++)
    {

	c = Components().At(i).iControl;
	if (c && c->IsFocused()) break;
    }
    return c;
}

void CViewContainer::Draw(const TRect& aRect) const
{
    CWindowGc& gc = SystemGc();
    if (!iSkinned)
    {

	gc.SetBrushStyle(CGraphicsContext::ESolidBrush);
	gc.SetBrushColor(KRgbWhite);
	gc.Clear();
	gc.DrawRect(aRect);
	return;
    }

    MAknsSkinInstance* skin = AknsUtils::SkinInstance();

    if (!skin || !AknsDrawUtils::Background(skin, iBgContext, this, gc, aRect))
    {

	gc.SetBrushStyle(CGraphicsContext::ESolidBrush);
	gc.SetBrushColor(KRgbBlack);
	gc.Clear();
	gc.DrawRect(aRect);
	return;
    }

}

TKeyResponse CViewContainer::OfferKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType)
{

    CCoeControl* c =  FocusedControl();

    if (c) return c->OfferKeyEventL(aKeyEvent,aType);

    return CCoeControl::OfferKeyEventL(aKeyEvent,aType) /*EKeyWasNotConsumed*/;

}

void CViewContainer::HandlePointerEventL(const TPointerEvent &aPointerEvent)
{
    CCoeControl::HandlePointerEventL(aPointerEvent);

    CCoeControl* c =  FocusedControl();
    if (c) c->HandlePointerEventL(aPointerEvent); 
}


TTypeUid::Ptr CViewContainer::MopSupplyObject(TTypeUid aId)
{
    if ( aId.iUid == MAknsControlContext::ETypeId)
    {
	return MAknsControlContext::SupplyMopObject(aId, iBgContext);
    }

    return CCoeControl::MopSupplyObject(aId);
}

void CViewContainer::HandleResourceChange(TInt aType)
{

    CCoeControl::HandleResourceChange( aType );
    if ( aType == KEikDynamicLayoutVariantSwitch )
    {
	DrawNow();
    }
}
// Implementation of CTextEdit


CTextEdit::~CTextEdit()
{

    if (iBgContext) delete iBgContext;
}

void CTextEdit::ConstructL(const TRect& aRect, const CCoeControl* aParent)
{
    CEikRichTextEditor::ConstructL(aParent, 0, 0,
	    EEikEdwinInclusiveSizeFixed|
	    EEikEdwinNoAutoSelection|
	    EEikEdwinAlwaysShowSelection, 0, 0);

    SetContainerWindowL(*aParent);
    SetRect(aRect);
    ActivateL();
}

TTypeUid::Ptr CTextEdit::MopSupplyObject(TTypeUid aId)
{
    if ( aId.iUid == MAknsControlContext::ETypeId)
    {
	return MAknsControlContext::SupplyMopObject(aId, iBgContext);
    }

    return CCoeControl::MopSupplyObject(aId);
}


TKeyResponse CTextEdit::OfferKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType)
{

    // allows inserting new line via D-PAD OK Key
    if (!IsReadOnly() && (aKeyEvent.iCode == EKeyDevice3))
    {
	WriteL(_L("\n"));
	return EKeyWasConsumed;
    }

    return CEikRichTextEditor::OfferKeyEventL(aKeyEvent,aType);

}

void CTextEdit::SizeChanged()
{
    if (iBgContext)
    {
	TRect rect;
	AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::EMainPane, rect);
	if (Size().iHeight > rect.Height())
	{

	    // large, full
	    iBgContext->SetBitmap(KAknsIIDQsnBgScreen);
	}
	
	else{
	    // normal
	    iBgContext->SetBitmap(KAknsIIDQsnBgAreaMain);
	}
    iBgContext->SetRect(Rect());
    }
    CEikRichTextEditor::HandleSizeChangedL();
    TextLayout()->RestrictScrollToTopsOfLines(EFalse);
}



void CTextEdit::SetSkinnedL()
{
     if (!iBgContext)
    {

	iBgContext = CAknsBasicBackgroundControlContext::NewL(KAknsIIDQsnBgScreen,
		TRect(0, 0, 0, 0),
		EFalse);
	
	SetSkinBackgroundControlContextL(iBgContext);
	SetSkinnedTextColor();
    }
}


TBool CTextEdit::CheckRange(const TInt &aPos, TInt&aLen, TInt aDocLen)
{
    if (aPos > aDocLen)return EFalse;

    if ((aLen == (-1)) || ((aPos + aLen) > aDocLen)) aLen = aDocLen - aPos;

    return ETrue;
}


TBool CTextEdit::IsEmpty()
{
    return RichText()->DocumentLength() == 0;
}


TPtrC CTextEdit::Read(const TInt& aPos)
{
    TInt len = RichText()->DocumentLength();
    //TPtrC p = RichText()->Read(aPos).Left(len);
    TPtrC p = Text()->Read(aPos).Left(len); // truncate

    TUint16* tp = (TUint16*)p.Ptr();
    while (len--)
    {
	if (*tp == 0x2029) *tp = '\n';
	++tp;
    }
    return p; 
}

void CTextEdit::ReadL(TDes &aBuf, const TInt &aPos, TInt aLen)
{
    if (aLen == -1) aLen = RichText()->DocumentLength();
    if(CheckRange(aPos, aLen, RichText()->DocumentLength()) == EFalse) User::Leave(KErrArgument);
    
    RichText()->Extract(aBuf, aPos, aLen);
}


void CTextEdit::WriteL(const TDesC& aText)
{
    TInt pos = CursorPos();

    TInt len_written;

    RMemReadStream source(aText.Ptr(), aText.Size());
    RichText()->ImportTextL(pos, source,
              CPlainText::EOrganiseByParagraph,
              KMaxTInt, KMaxTInt, &len_written);
  
    RichText()->ApplyCharFormatL(iCharFormat, iCharFormatMask, pos, len_written);
  
    iTextView->HandleInsertDeleteL(TCursorSelection(pos,pos+len_written),0,ETrue);
  
    SetCursorPosL(pos+len_written, EFalse);
}

void CTextEdit::ClearL()
{
    RichText()->Reset();
    HandleTextChangedL();
    SetCursorPosL(0, EFalse);
}

void CTextEdit::DelL(TInt aPos, TInt aLen)
{
    if (CheckRange(aPos, aLen, RichText()->DocumentLength()) == EFalse) User::Leave(KErrArgument);
  
    RichText()->DeleteL(aPos, aLen);
  
    iTextView->HandleInsertDeleteL(TCursorSelection(aPos,aPos),aLen,ETrue);
  
    SetCursorPosL(CursorPos()-aLen, EFalse);
}

TInt CTextEdit::SetPos(TInt aPos)
{
    if (aPos > RichText()->DocumentLength())
	aPos = RichText()->DocumentLength();
  
    TRAPD(error, SetCursorPosL(aPos, EFalse)); 
    return error;
}

void CTextEdit::HandleChangeL(TInt aNewCursorPos)
{
    HandleTextChangedL();   
    SetCursorPosL(aNewCursorPos, EFalse);
}

void CTextEdit::SetTextColor(TRgb color)
{
    iCharFormatMask.SetAttrib(EAttColor);
    iCharFormat.iFontPresentation.iTextColor = color;
}

void CTextEdit::SetSkinnedTextColor()
{
    TRgb textColor;
    AknsUtils::GetCachedColor(AknsUtils::SkinInstance(), textColor, KAknsIIDQsnTextColors, EAknsCIQsnTextColorsCG6);
    SetTextColor(textColor);
}



void CTextEdit::SetFontL(TFontSpec& aFontSpec)
{
    iCharFormatMask.SetAttrib(EAttFontHeight);
    iCharFormatMask.SetAttrib(EAttFontPosture); 
    iCharFormatMask.SetAttrib(EAttFontStrokeWeight); 
    iCharFormatMask.SetAttrib(EAttFontPrintPos);  
    iCharFormatMask.SetAttrib(EAttFontTypeface); 
    //iCharFormatMask.SetAll(); 
    iCharFormat.iFontSpec = aFontSpec; 
}

// Implementation of CGridListBox

CGridListBox::CGridListBox()
{
    iFont = NULL;
    iGrid = NULL;
}


CGridListBox::~CGridListBox()
{
    if(iGrid) delete iGrid;
    iGrid = NULL;
}

void CGridListBox::ConstructL(CCoeControl* aParent, const TRect& aRect, TInt aResourceId)
{
    if(!aParent) CreateWindowL();
    else {SetContainerWindowL(*aParent);}


    iGrid = new (ELeave)CAknGrid;
    iGrid->SetContainerWindowL(*this);
    if (aResourceId != 0)
    {
	TResourceReader reader;
	CEikonEnv::Static()->CreateResourceReaderLC(reader, aResourceId);
	iGrid->ConstructFromResourceL(reader);
	CleanupStack::PopAndDestroy(); // reader
    }
    else
    {
	iGrid->ConstructL(this, EAknListBoxSelectionGrid);
    }
    SetRect(aRect);
    ActivateL();
}

TInt CGridListBox::CountComponentControls() const

{
    return iGrid ? 1 : 0;
}


CCoeControl* CGridListBox::ComponentControl(TInt aIndex) const
{
    switch ( aIndex )
    {
	case 0:
	    return iGrid;
	default:
	    return NULL;
    }
}

void CGridListBox::SizeChanged()
{
    if (iGrid)
    {
	iGrid->SetRect(Rect());
    }
}

TKeyResponse CGridListBox::OfferKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType)
{
    
    if (iGrid) return iGrid->OfferKeyEventL(aKeyEvent,aType);

    return CCoeControl::OfferKeyEventL(aKeyEvent,aType) /*EKeyWasNotConsumed*/;

}

void CGridListBox::HandlePointerEventL(const TPointerEvent &aPointerEvent)
{
    CCoeControl::HandlePointerEventL(aPointerEvent);

    if (iGrid) iGrid->HandlePointerEventL(aPointerEvent);

}


void CGridListBox::AddItemL(const TDesC& aItem, TInt aIndex)
{
    CDesCArray* itemsArray = static_cast<CDesCArray*>(iGrid->Model()->ItemTextArray());
    if (!itemsArray) return;
    
    TBuf<KMaxFileName*2> cellText;
    
    if (aIndex != -1) cellText.AppendNum(aIndex); // icon index
    cellText.Append(_L("\t"));
    cellText.Append(aItem);
    itemsArray->AppendL(cellText);
}


void CGridListBox::AddIconL(const TDesC& aFilePath, TInt aBitmapId, TInt aMaskId)
{
    CArrayPtr<CGulIcon>* iconArray = iGrid->ItemDrawer()->FormattedCellData()->IconArray();

    if (!iconArray) return;
    if (aMaskId == -1) aMaskId = aBitmapId+1;

    //iconArray->AppendL(iEikonEnv->CreateIconL(aFilePath, aBitmapId, aMaskId));
    
    CFbsBitmap *bm, *mask;
    AknIconUtils::CreateIconL(bm, mask, aFilePath, aBitmapId, aMaskId);
    iconArray->AppendL(CGulIcon::NewL(bm, mask));
}

void CGridListBox::RemoveItemsL()
{

    CDesCArray* itemsArray = static_cast<CDesCArray*>(iGrid->Model()->ItemTextArray());

    if (!itemsArray) return;
    if (itemsArray->MdcaCount() == 0) return;

    itemsArray->Reset();
    iGrid->HandleItemRemovalL();
}



void CGridListBox::SetCellSize(TInt aWidth, TInt aHeight)
{
    iSizeOfCell.iWidth = aWidth;
    iSizeOfCell.iHeight = aHeight;
}


void CGridListBox::SetCellRects(TRect& aTRect, TRect& aGfxRect)
{
    iTextRect.SetRect(aTRect.iTl, aTRect.iBr);
    iGfxRect.SetRect(aGfxRect.iTl, aGfxRect.iBr);
	
}

void CGridListBox::SetFont(const CFont *aFont)
{
    iFont = aFont;
}

void CGridListBox::SetupGrid(TInt aNumOfItems, TInt aGridColumns, TInt aGridRows,TBool aWithIcons)
{	

    AknListBoxLayouts::SetupStandardGrid(*iGrid); 
    TSize mpSize;     // main pane size
    AknLayoutUtils::LayoutMetricsSize(AknLayoutUtils::EMainPane, mpSize);
    
    // Determinate cell size
    iSizeOfCell.iWidth = mpSize.iWidth / aGridColumns;
    iSizeOfCell.iHeight = mpSize.iHeight / aGridRows;
    iTextRect.SetRect(0,0,0,0);
  

    if (aWithIcons){
	iGfxRect.SetRect(0, 0, iSizeOfCell.iWidth, iSizeOfCell.iHeight/2); 
	SetupGridIconsL(aNumOfItems);
    }
     
    TBool verticalOrientation = EFalse;
    TBool leftToRight = ETrue;
    TBool topToBottom = ETrue;
    
    // Set grid layout
    iGrid->SetLayoutL(verticalOrientation, 
        leftToRight, topToBottom, 
        aGridColumns, aGridRows,  
        iSizeOfCell);

    // Set scrolling type
    iGrid->SetPrimaryScrollingType(CAknGridView::EScrollIncrementLineAndLoops);
    iGrid->SetSecondaryScrollingType(CAknGridView::EScrollIncrementLineAndLoops);    
    // Set current index in grid
    iGrid->SetCurrentDataIndex(0);
    if (aWithIcons) SetupGfxCellL();	
    SetupTextCell(); 
    CEikScrollBarFrame* frame = iGrid->ScrollBarFrame();
    if (frame) frame->SetScrollBarVisibilityL(CEikScrollBarFrame::EOff,CEikScrollBarFrame::EAuto);
}


void CGridListBox::SetupGfxGrid(TInt aNumOfItems)
{
  
    TInt cellWidth = iGrid->ColumnWidth();
    TInt cellHeight = iGrid->ItemHeight();
    
    AknListBoxLayouts::SetupStandardGrid(*iGrid); 

    TBool verticalOrientation = EFalse;
    TBool leftToRight = ETrue;
    TBool topToBottom = ETrue;
    
    TRect textRect(0,0,0,0);
    TRect gfxRect(0,0, cellWidth, cellHeight/2);
    SetCellRects(textRect, gfxRect);
    SetupGridIconsL(aNumOfItems);   
    iGrid->SetCurrentDataIndex(0);
    SetupGfxCellL();
    SetupTextCell(); 
    CEikScrollBarFrame* frame = iGrid->ScrollBarFrame();
    if (frame) frame->SetScrollBarVisibilityL(CEikScrollBarFrame::EOff,CEikScrollBarFrame::EAuto);

}

void CGridListBox::SetupGridIconsL(TInt aNumOfItems)
{
    CArrayPtr<CGulIcon>* icons = new (ELeave) CAknIconArray(aNumOfItems);
    iGrid->ItemDrawer()->FormattedCellData()->SetIconArray(icons);
}

void CGridListBox::SetupGfxCellL(TInt aIndex)
{


    AknListBoxLayouts::SetupFormGfxCell(
       *iGrid,              // Reference to grid control
       iGrid->ItemDrawer(),// Pointer to the item drawer
       aIndex,                   // Column index
       0,     // Left position
       iGfxRect.Height()/2,     // Top position
       0,                   // Right - unused
       0,                   // Bottom - unused
       iGfxRect.Width(), // Width
       iGfxRect.Height(),// Height
       iGfxRect.iTl,         // Start position
       iGfxRect.iBr);     // End position
}

void CGridListBox::SetupTextCell(TInt aIndex, TInt aBaseline)
{
    // TInt cellWidth = iGrid->ColumnWidth();
    TInt cellHeight = iGrid->ItemHeight();
    
    if (!iFont) iFont = const_cast<CFont*>(AknLayoutUtils::FontFromId(EAknLogicalFontPrimaryFont, NULL));

 
    // Deretminate the baseline for the text
    if (aBaseline == -1) aBaseline = cellHeight - iFont->DescentInPixels() - 1;

    TInt colorIndex = 0; // no idea how to find the correct index ????

    // Layout of text
    AknListBoxLayouts::SetupFormTextCell(
       *iGrid, // Reference to grid
       iGrid->ItemDrawer(),        // Pointer to the item drawer
       aIndex,                  // Column index
       iFont,           // Font
       colorIndex,              // Color (215 = black)
       0,                // Left margin
       0,                // Right margin - unused
       aBaseline,       // Baseline
       iTextRect.Width(),    // Text width
       CGraphicsContext::ECenter,    // Text alignment
	iTextRect.iTl, // Start position
	iTextRect.iBr);    // End position


    TRgb textColor; //AKN_LAF_COLOR(EColorControlText);
 
    AknsUtils::GetCachedColor(AknsUtils::SkinInstance(), textColor, KAknsIIDQsnTextColors, EAknsCIQsnTextColorsCG6);
 
    // obvious function, but has no effect!!!
    //iGrid->ItemDrawer()->SetTextColor(textColor);

    // took me ages to find out how to set skinned text color correctly ...

    CFormattedCellListBoxData::TColors colors = iGrid->ItemDrawer()->FormattedCellData()->SubCellColors(aIndex);
    colors.iText = textColor;
    //colors.iHighlightedText = textColor;
    iGrid->ItemDrawer()->ColumnData()->SetSubCellColorsL(aIndex, colors);
}




