#ifndef __UICOMMONS_H__
#define __UICOMMONS_H__

#include <aknlists.h>
#include <eiktxlbm.h>
#include <eikclb.h>
#include <eikclbd.h>
#include <eiklbo.h>
#include <akniconarray.h>
#include <akniconutils.h>
#include <gulicon.h>
#include <barsread.h>
#include <akngrid.h> // CAknGrid
#include <eiklbx.h>
#include <coecntrl.h>
#include <aknsdrawutils.h>
#include <aknscontrolcontext.h>
#include <aknsconstants.h>
#include <aknsbasicbackgroundcontrolcontext.h>
#include <aknutils.h>
#include <s32mem.h>
#include <txtfrmat.h>         // Text
#include <txtfmlyr.h>         // Text
			      //#include <eikgted.h>          // Text
#include <eikrted.h>          // RText
#include <txtrich.h>
#include <akninfopopupnotecontroller.h>

// TODO: add some comments


TBool DriveSelectionL(TDes &aDrivePath, const TInt aTitleResId,
	const TBool aShowDZ=EFalse, const TBool aShowReadOnly=EFalse);

void ErrorNoteL(const TDesC *aText, TInt aResourceId=0);

void SetAppTitleL(const TDesC *aTitle, TInt aResourceId=0);

class CBitmapDrawer
{
public:
	// Constructor
	CBitmapDrawer();

	~CBitmapDrawer();
	
	void ConstructL(TSize aSize);
	
	void ConstructL(CFbsBitmap* aBitmap);

	static CBitmapDrawer* NewL(TSize aSize);

	static CBitmapDrawer* NewL(CFbsBitmap* aBitmap);

public:
	// Functions
	void SetBitmap(CFbsBitmap* aBitmap);
	void SetBrushColorL(const TRgb &aColor, TBool aNoBorder=EFalse);
	void ClearL(const TRgb &aColor);
	void ClearL();
	void DrawRectL(const TRect &aRect);
	void DrawRectL(const TRect &aRect, const TRgb &aColor);
	void ResizeL(const TSize &aSize);
	CFbsBitmap* Bitmap();
	CGraphicsContext* Gc();
	TSize Size();
private:
	void Reset();
private:
	CGraphicsContext *iGc;
	CFbsBitmapDevice *iBitmapDevice;
	CFbsBitmap *iBitmap;
	TSize iSize;
	TBool iOwnBitmap;
};



class CViewContainer: public CCoeControl
{

    public:
	CViewContainer();
	~CViewContainer();
	void ConstructL(const TRect& aRect, const TBool aSkinned=EFalse);
	void AddControlL(CCoeControl* aControl);
	void UpdateControl(const TInt &aIndex, const TBool &aUpdateRect=ETrue);
	/**
	 * From CoeControl 
	 **/

	void SizeChanged();

    private:

	void SetupSkinBgContext();
	CCoeControl*  FocusedControl();
	/**
	 * From CoeControl 
	 *
	 **/
	void Draw(const TRect& aRect) const;
	TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType);
	void HandlePointerEventL(const TPointerEvent &aPointerEvent);
	TTypeUid::Ptr MopSupplyObject(TTypeUid aId);
	void HandleResourceChange(TInt aType);

    private:
	TBool iSkinned;
	CAknsBasicBackgroundControlContext *iBgContext;
};


//class CTextEdit : public CEikGlobalTextEditor

class CTextEdit : public CEikRichTextEditor
{

    public:
	CTextEdit() {}

	void ConstructL(const TRect& aRect, const CCoeControl* aParent);

	~CTextEdit();

	TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType);


	void SetSkinnedL();

	TBool IsEmpty();

	TPtrC Read(const TInt &aPos=0);

	void ReadL(TDes &aBuf, const TInt &aPos=0, TInt aLen=-1);

	void WriteL(const TDesC& aText);

	void ClearL();

	void DelL(TInt aPos, TInt aLen);

	TInt SetPos(TInt aPos);

	void SetTextColor(TRgb color);

	void SetSkinnedTextColor();

	void SetFontL(TFontSpec&);

	//void SetSize(const TSize &aSize);

    private:


	void HandleChangeL(TInt aNewCursorPos);

	static TBool CheckRange(const TInt &aPos, TInt &aLen, TInt aDocLen);


    protected:

	TTypeUid::Ptr MopSupplyObject(TTypeUid aId);

	void SizeChanged();



    private:

	TCharFormatMask iCharFormatMask;

	TCharFormat iCharFormat;

	CAknsBasicBackgroundControlContext *iBgContext; 

};

// Implementation of TListBox


#define  SINGLE_STYLE CAknSingleStyleListBox
#define  SINGLE_GFX_STYLE CAknSingleGraphicStyleListBox
#define  SINGLE_LARGE_STYLE CAknSingleLargeStyleListBox
#define  DOUBLE_STYLE CAknDoubleStyleListBox

template <class T>

class TListBox: public CCoeControl
{
    public:
	TListBox()
	{
	    iListbox = new T;
	}

	~TListBox()
	{
	    if (iListbox) delete iListbox;
	    iListbox = NULL;
	}

	void ConstructL(CCoeControl* aParent, const TRect& aRect, TInt aResourceId=0, TInt flags=EAknListBoxSelectionList)
	{
	    if (!aParent) CreateWindowL();
	    else 
	    {
		SetContainerWindowL(*aParent);
	    }

	    iListbox->SetContainerWindowL(*this);
	    if (aResourceId != 0)
	    {
		TResourceReader reader;

		CEikonEnv::Static()->CreateResourceReaderLC(reader, aResourceId);

		iListbox->ConstructFromResourceL(reader);
		CleanupStack::PopAndDestroy(); // reader 

	    }

	    else{
		iListbox->ConstructL(this, flags);
	    }

	    iListbox->CreateScrollBarFrameL(ETrue);

	    CEikScrollBarFrame *frame = iListbox->ScrollBarFrame();

	    if (frame) frame->SetScrollBarVisibilityL(CEikScrollBarFrame::EOff, CEikScrollBarFrame::EAuto);

	    SetRect(aRect);
	    ActivateL();
	}


    public:

	T* Listbox() {return iListbox;};

	void SetObserver(MEikListBoxObserver *aOberver)
	{
	    if (iListbox) iListbox->SetListBoxObserver(aOberver);

	}

	void AddItemL(const TDesC& aItem)
	{

	    CDesCArray* itemsArray = static_cast<CDesCArray*>(iListbox->Model()->ItemTextArray());

	    if (!itemsArray) return;
	    itemsArray->AppendL(aItem);

	}

	void RemoveItemL(const TInt &aIndex)
	{

	    CDesCArray* itemsArray = static_cast<CDesCArray*>(iListbox->Model()->ItemTextArray());

	    if (!itemsArray) return;
	    if (aIndex < 0 || aIndex >= itemsArray->MdcaCount()) return;

	    itemsArray->Delete(aIndex);
	    iListbox->HandleItemRemovalL();
	}

	void RemoveItemsL()
	{

	    CDesCArray* itemsArray = static_cast<CDesCArray*>(iListbox->Model()->ItemTextArray());

	    if (!itemsArray) return;
	    if (itemsArray->MdcaCount() == 0) return;

	    itemsArray->Reset();
	    iListbox->HandleItemRemovalL();
	}

	CDesCArray*  ItemsArray()
	{
	    return static_cast<CDesCArray*>(iListbox->Model()->ItemTextArray());

	}

	void AddIconL(CArrayPtr<CGulIcon>* aIconArray , const TDesC& aFilePath, TInt aBitmapId, TInt aMaskId=-1)
	{

	    if (!aIconArray) return;

	    if (aMaskId == -1) aMaskId = aBitmapId+1;

	    CFbsBitmap *bm, *mask;
	    AknIconUtils::CreateIconL(bm, mask, aFilePath, aBitmapId, aMaskId);
	    aIconArray->AppendL(CGulIcon::NewL(bm, mask));

	}

    private: // from base classes

	/**
	 * From CoeControl 
	 *
	 * */

	void SizeChanged()
	{
	    if (iListbox) iListbox->SetRect(Rect());

	}

	TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType)
	{
	    if (iListbox) return iListbox->OfferKeyEventL(aKeyEvent,aType);

	    return CCoeControl::OfferKeyEventL(aKeyEvent,aType) /*EKeyWasNotConsumed*/;

	}

	void HandlePointerEventL(const TPointerEvent &aPointerEvent)
	{
	    CCoeControl::HandlePointerEventL(aPointerEvent);
	    if (iListbox) iListbox->HandlePointerEventL(aPointerEvent);

	}

	TInt CountComponentControls() const
	{
	    return iListbox ? 1 : 0;
	}

	CCoeControl* ComponentControl(TInt aIndex) const
	{

	    return ( aIndex == 0) ? iListbox : NULL;
	}

    private: // data

	//CEikListBox* iListbox;
	T* iListbox;
};

class CGridListBox: public CCoeControl
{

    public:
	CGridListBox();

	~CGridListBox();

	void ConstructL(CCoeControl* aParent, const TRect& aRect, TInt aResourceId=0);

public:
	CAknGrid* Grid() {return iGrid;};

	void SetupGrid(TInt aNumOfItems, TInt aGridColumns, TInt aGridRows,TBool aWithIcons=EFalse);
	void SetupGfxGrid(TInt aNumOfItems);

	void SetupGridIconsL(TInt aNumOfItems);	

	void SetupGfxCellL(TInt aIndex=0);	
    
	void SetupTextCell(TInt aIndex=1, TInt aBaseline=-1);
 
	void AddItemL(const TDesC& item, TInt aIndex=-1);

	void AddIconL(const TDesC& aFilePath, TInt aBitmapId, TInt aMaskId=-1);

	void RemoveItemsL();

	void SetCellSize(TInt aWidth, TInt aHeight);

	void SetCellRects(TRect& aTRect, TRect& aGfxRect);

	void SetFont(const CFont *aFont);


    private: // from base classes

       /******
        * From CoeControl 
	*
	******/

	 void SizeChanged();

	 TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType);

         TInt CountComponentControls() const;

         CCoeControl* ComponentControl(TInt aIndex) const;

	 void HandlePointerEventL(const TPointerEvent &aPointerEvent);


    private: // own functions
	void SetDefaultGridLayout(TBool aWithIcons);

    private: // data
	const CFont *iFont;
	CAknGrid* iGrid;
	/**
	 * iNumOfColumns, number of grid columns
	 */
	TInt iNumOfColumns;

	/**
	 * iNumOfRows, number of grid rows
	 */
	TInt iNumOfRows;

	/**
	 * iSizeOfCell, size of grid cell
	 */
	TSize iSizeOfCell;
	TRect iTextRect;
	TRect iGfxRect;
	/**
        * iNumOfItems, number of grid items
        */
    
	TInt iNumOfItems;
};


#endif //__UICOMMONS_H__
