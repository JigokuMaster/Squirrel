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
#include <bautils.h>
#include "SquirrelAppUi.h"
#include "SquirrelAboutView.h"
#include "Squirrel.rsg"
#include "Squirrel.hrh"
#include "SquirrelModel.h"

#include <eikenv.h>
#include <f32file.h>

#ifdef __WINSCW__
_LIT(KHTMLFile, "C:\\System\\Apps\\Squirrel\\about.html");
#else
_LIT(KHTMLFile, "about.html");
#endif

CWebView::CWebView()
{
    iBrCtlInterface = NULL;
}

CWebView::~CWebView()
{
    if (iBrCtlInterface) delete iBrCtlInterface;

}

CWebView* CWebView::NewLC(const TRect& aRect)
{
    CWebView* self = new (ELeave) CWebView();
    CleanupStack::PushL(self);
    self->ConstructL(aRect);
    return self;
}

CWebView* CWebView::NewL(const TRect& aRect)
{
    CWebView* self = CWebView::NewLC(aRect);
    CleanupStack::Pop(); // self;
    return self;
}

void CWebView::ConstructL(const TRect& aRect)
{
    CreateWindowL();
    iBrCtlInterface = CreateBrowserControlL(this, 
	aRect, 
	0/*TBrCtlDefs::ECapabilityDisplayScrollBar*/,
	TBrCtlDefs::ECommandIdBase, 
        NULL, 
        NULL, 
        NULL,
        NULL,
	NULL
    );
    //iBrCtlInterface->SetBrowserSettingL(TBrCtlDefs::ESettingsSmallScreen, 1);
    iBrCtlInterface->SetBrowserSettingL(TBrCtlDefs::ESettingsTextWrapEnabled, 1);
#ifdef __WINSCW__
    if (BaflUtils::FileExists(iEikonEnv->FsSession(), KHTMLFile))
    {
	iBrCtlInterface->LoadFileL(KHTMLFile);
    }
#else
    // load from private folder
    TFileName fp; // full path
    TFileName privatePath;
    iEikonEnv->FsSession().PrivatePath(privatePath);
    fp.Append(RProcess().FileName().Left(2)); // C: or E: etc...
    fp.Append(privatePath);     
    fp.Append(KHTMLFile);
    if (BaflUtils::FileExists(iEikonEnv->FsSession(), fp))
    {
	iBrCtlInterface->LoadFileL(fp);
    }
   
#endif

    SetRect(aRect);
    SetBlank();
    ActivateL();
}


TInt CWebView::CountComponentControls() const
{
    if (iBrCtlInterface)
    {
        return 1;
    }
    return 0;

}

CCoeControl* CWebView::ComponentControl(TInt aIndex) const
{
    switch ( aIndex )
    {
        case 0:
            return iBrCtlInterface; // Could be NULL
        default:
            return NULL;
    }
}

void CWebView::SizeChanged()
{
    if (iBrCtlInterface)
    {
        iBrCtlInterface->SetRect(Rect());
    }
}

void CWebView::HandlePointerEventL(const TPointerEvent& aPointerEvent)
{

    CCoeControl::HandlePointerEventL(aPointerEvent);
    if (iBrCtlInterface) iBrCtlInterface->HandlePointerEventL(aPointerEvent);

}

TKeyResponse CWebView::OfferKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType)
{
    if (iBrCtlInterface)
    {
	return iBrCtlInterface->OfferKeyEventL(aKeyEvent, aType);
    }
    return EKeyWasConsumed;
}


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CSquirrelAboutView::NewL()
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CSquirrelAboutView* CSquirrelAboutView::NewL()
{
    CSquirrelAboutView* self = CSquirrelAboutView::NewLC();
    CleanupStack::Pop(self);
    return self;
}

// -----------------------------------------------------------------------------
// CSquirrelAboutView::NewLC()
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CSquirrelAboutView* CSquirrelAboutView::NewLC()
{
    CSquirrelAboutView* self = new (ELeave) CSquirrelAboutView;
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
}

// -----------------------------------------------------------------------------
// CSquirrelAboutView::ConstructL()
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CSquirrelAboutView::ConstructL()
{
    BaseConstructL(R_ABOUT_VIEW);
}

// -----------------------------------------------------------------------------
// CSquirrelAboutView::SquirrelAboutView()
// C++ default constructor can NOT contain any code, that might leave.
// -----------------------------------------------------------------------------
//
CSquirrelAboutView::CSquirrelAboutView()
{
    iContainer = NULL;

}

// -----------------------------------------------------------------------------
// CSquirrelAboutView::~SquirrelAboutView()
// Destructor.
// -----------------------------------------------------------------------------
//
CSquirrelAboutView::~CSquirrelAboutView()
{
    if (iContainer)
    {
	delete iContainer;
	iContainer = NULL;
    }
}


TUid CSquirrelAboutView::Id() const 
{ 
    return TUid::Uid(EAboutView); 
}


void CSquirrelAboutView::HandleCommandL( TInt aCommand )
{
    AppUi()->HandleCommandL(aCommand); 
}


void CSquirrelAboutView::HandleViewRectChange()
{
    if (iContainer)
    {
	iContainer->SetRect(ClientRect());
    }   
}


void CSquirrelAboutView::DoActivateL( const TVwsViewId& /*aPrevViewId*/,
	TUid /*aCustomMessageId*/,
	const TDesC8& /*aCustomMessage*/ )
{
    SetAppTitleL(NULL, R_ABOUT_TITLE);

    if (!iContainer) iContainer = CWebView::NewL(ClientRect());
    iContainer->SetMopParent(this);
    AppUi()->AddToStackL(*this, iContainer); 
    Cba()->MakeCommandVisible(EAknSoftkeyOptions, EFalse);
   
}


void CSquirrelAboutView::DoDeactivate()
{
    if ( iContainer )
    {
	AppUi()->RemoveFromViewStack(*this, iContainer);
    }  
}

// End of File
