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
#include <aknmessagequerydialog.h>
#include <AknQueryDialog.h>
//#include <aknnotewrappers.h>
//#include <stringloader.h>
#include <f32file.h>
#include <s32file.h>
#include <utf.h> // CnvUtfConverter
#include "SquirrelAppUi.h"
#include "SquirrelAboutView.h"
#include "Squirrel.rsg"
#include "Squirrel.hrh"
#include "SquirrelModel.h"


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
    iListbox = NULL;
    iBrowserLauncher = NULL;
    iOSNoticeContent = NULL;

}

// -----------------------------------------------------------------------------
// CSquirrelAboutView::~SquirrelAboutView()
// Destructor.
// -----------------------------------------------------------------------------
//
CSquirrelAboutView::~CSquirrelAboutView()
{
   
    if ( iListbox )
    {
	AppUi()->RemoveFromStack( iListbox );
	delete iListbox;
    } 
    
    if (iBrowserLauncher) delete iBrowserLauncher;
    if (iOSNoticeContent) delete iOSNoticeContent;
    iOSNoticeContent = NULL;
	
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
    if (iListbox)
    {
	iListbox->SetRect(ClientRect());
    }   
}


void CSquirrelAboutView::DoActivateL( const TVwsViewId& /*aPrevViewId*/,
	TUid /*aCustomMessageId*/,
	const TDesC8& /*aCustomMessage*/ )
{
    SetAppTitleL(NULL, R_ABOUT_TITLE);
    iListbox = new (ELeave) TListBox<DOUBLE_STYLE>;
    iListbox->ConstructL(NULL, ClientRect(), R_ABOUTVIEW_LISTBOX);
    iListbox->SetObserver(this);
    iListbox->Listbox()->ItemDrawer()->FormattedCellData()->EnableMarqueeL(ETrue);
    iListbox->SetMopParent(this);
    
    Cba()->MakeCommandVisible(EAknSoftkeyOptions, EFalse);

    AppUi()->AddToStackL(*this, iListbox);    
}




void CSquirrelAboutView::DoDeactivate()
{

    if ( iListbox )
    {
	AppUi()->RemoveFromViewStack( *this, iListbox );
	delete iListbox;
	iListbox = NULL;
    }    
}


void CSquirrelAboutView::HandleListBoxEventL(CEikListBox* aListBox , TListBoxEvent aEventType)
{
    if ((aEventType == EEventEnterKeyPressed) || (aEventType == EEventItemClicked))
    {

	TInt itemIndex = iListbox->Listbox()->CurrentItemIndex();
	if (itemIndex == 3)
	{
	    
	    HBufC* msg = iCoeEnv->AllocReadResourceLC(R_CONTRIBUTOR1_INFO);
	    CAknMessageQueryDialog* dlg = new (ELeave) CAknMessageQueryDialog;
	    TPtr msgPtr = msg->Des();
	    TInt prefix = msgPtr.Find(_L("["));
	    TInt suffix = msgPtr.Find(_L("]"));
	    iLinkBuf.Zero();
	    if (suffix != KErrNotFound && prefix != KErrNotFound)
	    {
		prefix++;
		TInt len = msgPtr.Length() - prefix - 1;
		iLinkBuf.Copy(msgPtr.Mid(prefix, len));
		msgPtr.Delete(prefix-1, 1);
		msgPtr.Delete(suffix-1, 1);
		TCallBack cb(CSquirrelAboutView::OpenLink, this);
		dlg->SetLink(cb);
		dlg->SetLinkTextL(iLinkBuf);
	    }

	    dlg->SetMessageTextL(*msg);
	    CleanupStack::PopAndDestroy(msg);
	    dlg->ExecuteLD(R_ABOUT_QUERY_DIALOG);
	
	    //CAknQueryDialog* q = CAknQueryDialog::NewL();
            /*if (q->ExecuteLD(R_GENERAL_CONFIRMATION_QUERY))
	    {
	    }*/

	}
	else if (itemIndex == 4) ShowOSNoticeL();

    }
}

void CSquirrelAboutView::OpenLinkL()
{
    if (!iBrowserLauncher)
    {
	iBrowserLauncher = CBrowserLauncher::NewL();
    }
    iBrowserLauncher->LaunchBrowserEmbeddedL(iLinkBuf);
}


TInt CSquirrelAboutView::OpenLink(TAny* aArg)
{
    TRAPD(err, STATIC_CAST(CSquirrelAboutView*,aArg)->OpenLinkL());
    if (err != KErrNone) ShowErrorL(err);
    return 1;
}


void CSquirrelAboutView::ShowOSNoticeL()
{

  
    if (!iOSNoticeContent)
    {
	RFile file;
	TInt fileSize;
	TFileName fp;

	GetPrivateFilePathL(iCoeEnv->FsSession(), fp, _L("opensource.txt"));	
	User::LeaveIfError(file.Open(iCoeEnv->FsSession(), fp, EFileRead));
	User::LeaveIfError(file.Size(fileSize));
	
	HBufC8* fileContentBuf = HBufC8::NewLC(fileSize);
	TPtr8 fileContentBufPtr(fileContentBuf->Des());
        User::LeaveIfError(file.Read(fileContentBufPtr));
	file.Close();

	iOSNoticeContent = HBufC::NewL(fileSize*3);
	TPtr buf(iOSNoticeContent->Des());
	TInt err = CnvUtfConverter::ConvertToUnicodeFromUtf8(buf, fileContentBufPtr);
	CleanupStack::PopAndDestroy(fileContentBuf);

	if (err != KErrNone){
	    ShowErrorL(err);
	    return;
	}
	
    }

    CAknMessageQueryDialog* dlg = new (ELeave) CAknMessageQueryDialog;

    dlg->PrepareLC(R_AVKON_MESSAGE_QUERY_DIALOG);
   
    if (iOSNoticeContent && iOSNoticeContent->Length() > 0)
    {

	dlg->SetMessageTextL(*iOSNoticeContent);
    }
    else
    {
	dlg->SetMessageTextL(_L("empty."));
    }

    dlg->ButtonGroupContainer().MakeCommandVisible(EAknSoftkeyCancel, EFalse);
    dlg->RunLD();
}
// End of File
