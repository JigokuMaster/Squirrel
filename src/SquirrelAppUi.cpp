/*
   ============================================================================
Name		: SquirrelAppUi.cpp
Author	  : JigokuMaster
Copyright   : Your copyright notice
Description : CSquirrelAppUi implementation
============================================================================
*/

// INCLUDE FILES
#include <avkon.hrh>
#include <Squirrel.rsg>

#ifdef _HELP_AVAILABLE_
#include <hlplch.h>
#include "Squirrel.hlp.hrh"
#endif
#include "Squirrel.hrh"
#include "Squirrel.pan"
#include "SquirrelApplication.h"
#include "SquirrelAppView.h"
#include "SquirrelDecoderView.h"
#include "SquirrelHistoryView.h"
#include "SquirrelScannerView.h"
#include "SquirrelGeneratorView.h"
#include "SquirrelAboutView.h"
#include "SquirrelDummyView.h"
#include "SquirrelAppUi.h"

// ============================ MEMBER FUNCTIONS ===============================


// -----------------------------------------------------------------------------
// CSquirrelAppUi::ConstructL()
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CSquirrelAppUi::ConstructL()
{
    // Initialise app UI with standard value.
    BaseConstructL(CAknAppUi::EAknEnableSkin/* | CAknAppUi::EAppUiOrientationPortrait*/);
    // Create view object
    iMainView = CSquirrelAppView::NewL();
    AddViewL(iMainView);
    
    iDecoderView = CSquirrelDecoderView::NewL();  
    AddViewL(iDecoderView);

    iHistoryView = CSquirrelHistoryView::NewL();  
    AddViewL(iHistoryView);

    iScannerView = CSquirrelScannerView::NewL();
    AddViewL(iScannerView);


    iGeneratorView = CSquirrelGeneratorView::NewL();   
    AddViewL(iGeneratorView);

    iAboutView = CSquirrelAboutView::NewL();
    AddViewL(iAboutView);
    iDummyView = CSquirrelDummyView::NewL();
    AddViewL(iDummyView);

    SetDefaultViewL(*iMainView);
}
// -----------------------------------------------------------------------------
// CSquirrelAppUi::CSquirrelAppUi()
// C++ default constructor can NOT contain any code, that might leave.
// -----------------------------------------------------------------------------
//
CSquirrelAppUi::CSquirrelAppUi()
{
    // No implementation required
}

// -----------------------------------------------------------------------------
// CSquirrelAppUi::~CSquirrelAppUi()
// Destructor.
// -----------------------------------------------------------------------------
//
CSquirrelAppUi::~CSquirrelAppUi()
{

}



// -----------------------------------------------------------------------------
// CSquirrelAppUi::HandleCommandL()
// Takes care of command handling.
// -----------------------------------------------------------------------------
//
void CSquirrelAppUi::HandleCommandL(TInt aCommand)
{
    switch (aCommand)
    {
	case EAknSoftkeyBack:
	    ActivateLocalViewL(iMainView->Id());
	    break;

	case EEikCmdExit:
	case EAknSoftkeyExit:
	    Exit();
	    break;
	default: 
	    break;
    }
}


void CSquirrelAppUi::HandleWsEventL(const TWsEvent& aEvent, CCoeControl* aDestination)
{
  
    if(aEvent.Type() == EEventKey)
    {
	switch (aEvent.Key()->iCode)
	{
	    case EKeyDelete:
	    case EKeyBackspace:
		if (IsViewActivated(iHistoryView))
		{
		    iHistoryView->HandleCommandL(ECmdDeleteHistory);
		}
		break;

	    case EKeyYes:
		if (IsViewActivated(iGeneratorView))
		{
		    iGeneratorView->HandleCommandL(ECmdSendImage);
		}
		else if (IsViewActivated(iDecoderView))
		{
		    iDecoderView->HandleCommandL(ECmdSendPayload);
		}
		break;
	    case 'f':
	    case '4':
		if (IsViewActivated(iGeneratorView))
		{
		    iGeneratorView->ToggleFullScreenMode();
		}
		break;
	    default:
		break;
	}
    }

    CAknViewAppUi::HandleWsEventL(aEvent, aDestination);

}

void CSquirrelAppUi::HandleResourceChangeL(TInt aType)
{
    CAknAppUi::HandleResourceChangeL(aType);
    if ( aType == KEikDynamicLayoutVariantSwitch)
    {
	TVwsViewId viewId;
	if ( GetActiveViewId(viewId) == KErrNone)
	{
	    View(viewId.iViewUid)->HandleViewRectChange();
	    if ((viewId.iViewUid == TUid::Uid(EMainView)) || (viewId.iViewUid == TUid::Uid(EScannerView)) ){
	    	ActivateLocalViewL(viewId.iViewUid);
	    }
	}
    }
}
  
void CSquirrelAppUi::ActivateDecoderViewL(CFbsBitmap* aBitmap)
{
    iDecoderView->iDecoderModel->SetBitmap(aBitmap);
    ActivateLocalViewL(TUid::Uid(EDecoderView), TUid::Uid(ECmdDecodeBitmap), KNullDesC8);
}


TBool CSquirrelAppUi::IsViewActivated(CAknView* aView)
{
    if (!aView) return EFalse;

    TVwsViewId viewId;
    TInt err = GetActiveViewId(viewId);
    return (!err && viewId.iViewUid == aView->Id());
}

CHistoryStore* CSquirrelAppUi::HistoryStore()
{
    if (!iHistoryView) return NULL;
    return iHistoryView->iHistoryStore;
}

// End of File
