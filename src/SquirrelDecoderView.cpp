/*
   ============================================================================
Name		: SquirrelDecoderView.cpp
Author	  : JigokuMaster
Copyright   : Your copyright notice
Description : Application view implementation
============================================================================
*/

// INCLUDE FILES
#include <coemain.h>
#include <caknfileselectiondialog.h>
#include <akncommondialogs.h>
#include <bautils.h>
#include "SquirrelAppUi.h"
#include "SquirrelDecoderView.h"
#include "Squirrel.rsg"
#include "Squirrel.hrh"

#define NULL_VIEWID TVwsViewId(TUid::Null(), TUid::Null())

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CSquirrelDecoderView::NewL()
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CSquirrelDecoderView* CSquirrelDecoderView::NewL()
{
    CSquirrelDecoderView* self = CSquirrelDecoderView::NewLC();
    CleanupStack::Pop(self);
    return self;
}

// -----------------------------------------------------------------------------
// CSquirrelDecoderView::NewLC()
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CSquirrelDecoderView* CSquirrelDecoderView::NewLC()
{
    CSquirrelDecoderView* self = new (ELeave) CSquirrelDecoderView;
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
}

// -----------------------------------------------------------------------------
// CSquirrelDecoderView::ConstructL()
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CSquirrelDecoderView::ConstructL()
{

    BaseConstructL(R_DECODER_VIEW);
    iDecoderModel  = new (ELeave) CDecoderModel;
}


// -----------------------------------------------------------------------------
// CSquirrelDecoderView::SquirrelAboutView()
// C++ default constructor can NOT contain any code, that might leave.
// -----------------------------------------------------------------------------
//
CSquirrelDecoderView::CSquirrelDecoderView()
{
    iPrevViewId = NULL_VIEWID;
}

// -----------------------------------------------------------------------------
// CSquirrelDecoderView::~SquirrelAboutView()
// Destructor.
// -----------------------------------------------------------------------------
//
CSquirrelDecoderView::~CSquirrelDecoderView()
{
    if (iDecoderModel) delete iDecoderModel;
}


TUid CSquirrelDecoderView::Id() const 
{ 
    return TUid::Uid(EDecoderView); 
}

void CSquirrelDecoderView::HandleCommandL( TInt aCommand )
{
    switch( aCommand )
    {
	case ECmdOpenImage:
	    SelectDriveRootL(ETrue);
	    break;

	case ECmdOpenDataMenu:
	    iDecoderModel->ShowDataMenuL(EFalse);

	    break;
	case ECmdSendPayload:
	    if (iDecoderModel->DataAvailable()) iDecoderModel->SendPayloadL();
	    break;

	case EAknSoftkeyBack:  
	    ActivatePrevViewL(); 
	    break;
	default:
	    AppUi()->HandleCommandL(aCommand);  
	    break;
    }
}

void CSquirrelDecoderView::HandleViewRectChange()
{
    if (iDecoderModel)
    {
	iDecoderModel->SetupViewL(ClientRect());
	iDecoderModel->RedrawCodeImageL();
    }   
}

void CSquirrelDecoderView::ActivatePrevViewL()
{
    // ugly ... 
    switch(iPrevViewId.iViewUid.iUid)
    {
	case EMainView:
	case EHistoryView:
	case EScannerView:
	    ActivateViewL(iPrevViewId);
	    break;
	default:
	    // shows the main view
	    AppUi()->HandleCommandL(EAknSoftkeyBack);
	    break;
    }

}

void CSquirrelDecoderView::DoActivateL( const TVwsViewId& aPrevViewId,
	TUid aCustomMessageId,
	const TDesC8& aCustomMessage)
{

    SetAppTitleL(NULL, R_DECODER_TITLE);

    if (aPrevViewId != NULL_VIEWID) iPrevViewId = aPrevViewId;

    CViewContainer* vc = iDecoderModel->SetupViewL(ClientRect());
    vc->SetMopParent(this);
    AppUi()->AddToStackL(*this, vc);
    
    // check caller messages

    if (aCustomMessageId == TUid::Uid(ECmdDecodeImageFile))
    {
	TFileName fn;
	fn.Copy((const TUint16*)aCustomMessage.Ptr(), aCustomMessage.Size()/2);
	iDecoderModel->StartDecodeL(fn);
    }

    else if (aCustomMessageId == TUid::Uid(ECmdDecodeBitmap))
    {
	iDecoderModel->StartDecodeL();
    }

    else if (aCustomMessageId == TUid::Uid(ECmdOpenImage))
    {
	// show filebrowser dialog 
	SelectDriveRootL();
    }

    Redraw();
}

void CSquirrelDecoderView::DoDeactivate()
{

    if (iDecoderModel)
    {
	AppUi()->RemoveFromStack(iDecoderModel->GetView());
    }
}



void CSquirrelDecoderView::DynInitMenuPaneL(TInt aResourceId, CEikMenuPane *aMenuPane)
{
    if (!iDecoderModel || aResourceId != R_DECODERVIEW_MENU) return;
    
    TBool dimm = !iDecoderModel->DataAvailable();
    aMenuPane->SetItemDimmed(ECmdOpenDataMenu, dimm);
}



void CSquirrelDecoderView::SelectDriveRootL(TBool aCalledFromMenu)
{
    TBuf<3> drive;
    if (DriveSelectionL(drive, R_FILESELECTION_TITLE))
    {
	SelectImageL(drive);
    }
    else if (!aCalledFromMenu) { ActivatePrevViewL(); }
}


void CSquirrelDecoderView::SelectImageL(const TDesC& aPath)
{
    
    TFileName filePath;
    filePath.Append(aPath);
    CAknFileSelectionDialog* d = CAknFileSelectionDialog::NewL(ECFDDialogTypeSelect, R_SELECT_IMAGEFILE_DIALOG);
    CleanupStack::PushL(d);

    TBool ok = d->ExecuteL(filePath);
    CleanupStack::PopAndDestroy(d);

    if (ok)
    {
	iDecoderModel->StartDecodeL(filePath);
    }
    else{
	SelectDriveRootL();
    }
}

// End of File
