/*
   ============================================================================
Name		: SquirrelGeneratorView.cpp
Author	  : JigokuMaster
Copyright   : Your copyright notice
Description : Application view implementation
============================================================================
*/

// INCLUDE FILES
#include <coemain.h>
#include <npdapi.h>
#include <es_sock.h>
#include <es_enum.h>
#include <in_sock.h>
#include <caknfileselectiondialog.h>
#include "SquirrelAppUi.h"
#include "SquirrelGeneratorView.h"
#include "SquirrelDlgs.h"
#include "Squirrel.rsg"
#include "Squirrel.hrh"



class CAsyncWait: public CActive
{

public:
    CAsyncWait():CActive(CActive::EPriorityStandard)
    {
	iWait = new (ELeave) CActiveSchedulerWait;
	CActiveScheduler::Add(this);

    }
    virtual ~CAsyncWait()
    {
	Cancel();
	delete iWait;
    }
    TInt Start()
    {
	SetActive();
	iWait->Start();
	return iStatus.Int();
    }
    void RunL()
    {
	iWait->AsyncStop();
    }
    void DoCancel(){}

private:
    CActiveSchedulerWait* iWait;
};


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CSquirrelGeneratorView::NewL()
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CSquirrelGeneratorView* CSquirrelGeneratorView::NewL()
{
    CSquirrelGeneratorView* self = CSquirrelGeneratorView::NewLC();
    CleanupStack::Pop(self);
    return self;
}

// -----------------------------------------------------------------------------
// CSquirrelGeneratorView::NewLC()
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CSquirrelGeneratorView* CSquirrelGeneratorView::NewLC()
{
    CSquirrelGeneratorView* self = new (ELeave) CSquirrelGeneratorView;
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
}

// -----------------------------------------------------------------------------
// CSquirrelGeneratorView::ConstructL()
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CSquirrelGeneratorView::ConstructL()
{
    BaseConstructL(R_GENERATOR_VIEW);
}


void CSquirrelGeneratorView::SetupControlsL()
{
    iImageView = new (ELeave) CEikImage;
    iImageView->SetPictureOwnedExternally(ETrue);
    iImageView->SetContainerWindowL(*iContainer);
    iContainer->AddControlL(iImageView);
    
    iTextEdit = new (ELeave) CTextEdit;
    iTextEdit->ConstructL(ClientRect(), iContainer);
    iTextEdit->SetBorder(TGulBorder::EDeepSunken);
    TRgb textColor;
    AknsUtils::GetCachedColor(AknsUtils::SkinInstance(), textColor, KAknsIIDQsnTextColors, EAknsCIQsnTextColorsCG6);
    iTextEdit->SetTextColorL(textColor);

    iTextEdit->WriteL(_L("text string"));
    iTextEdit->SetMaxLength(QRCEncoder::MaxBufferSize);
    iTextEdit->SetFocus(ETrue);
    iContainer->AddControlL(iTextEdit);


    LayoutControls();
    if (!iEncoderModel)
    {
	iEncoderModel = new (ELeave) CQRCEncoderModel(iImageView->Size());
	iEncoderModel->CreateSettingsL();
    }
    
    iImageView->SetBitmap(iEncoderModel->Bitmap());
    iContainer->UpdateControl(1, EFalse);
}



void CSquirrelGeneratorView::LayoutControls()
{

    TInt edge = 10;
    TRect r = ClientRect();  
    TInt rw = r.Width();
    TInt rh = r.Height();

    TInt editorMinHeight = rh*0.3;
    TInt imgBaseSize = (rh-editorMinHeight) - (edge*2);
    TRect rect1(TPoint(edge, 0), TSize(imgBaseSize, imgBaseSize));
    iImageView->SetRect(rect1);

    TInt editorWidth = rw-edge;  
    TInt editorHeight = rh-imgBaseSize;   
    TRect rect2(TPoint(edge, edge+imgBaseSize), TSize(editorWidth, editorHeight));
    iTextEdit->SetRect(rect2);

}

void CSquirrelGeneratorView::DoGenerateL()
{
    TPtrC utf16Text = iTextEdit->Read();
    iEncoderModel->EncodeL(utf16Text);
    iContainer->UpdateControl(0, EFalse);
}


void CSquirrelGeneratorView::DoGenerateFromPhoneIMEIL(const TBool aRetry)
{

#ifdef __WINS__ 
  ShowErrorL(KErrNotSupported);
#else
  CTelephony::TPhoneIdV1 phoneId;    
  CTelephony::TPhoneIdV1Pckg phoneIdPkg(phoneId);
  if (!iTelephony) iTelephony = CTelephony::NewL();
  
  CAsyncWait* aWait = new (ELeave) CAsyncWait;
  
  TRequestStatus status = aWait->iStatus;
  iTelephony->GetPhoneId(status, phoneIdPkg);
  TInt err = aWait->Start();
  if (!err)
  {
      if (phoneId.iSerialNumber.Length() == 0)
      {
	  delete aWait;
	  DoGenerateFromPhoneIMEIL(EFalse);
	  return;
      }
      iTextEdit->ClearL();
      iTextEdit->WriteL(phoneId.iSerialNumber);
      iEncoderModel->EncodeL(phoneId.iSerialNumber);
      iContainer->UpdateControl(0, EFalse);
  }
  else   ShowErrorL(err); 
  delete aWait;

#endif /*__WINS__*/
}


void CSquirrelGeneratorView::DoGenerateFromIPAddressL()
{
    RSocketServ rss;
    RSocket sock;

    User::LeaveIfError(rss.Connect());
    User::LeaveIfError(sock.Open(rss,KAfInet,KSockStream,KProtocolInetTcp));
    // Copied from PyS60 e32socketmodule: list network interfaces and find wlan ip.      
    TInt error;
    TSoInetInterfaceInfo ifinfo;
    TPckg<TSoInetInterfaceInfo> ifinfopkg(ifinfo);
    TSoInetIfQuery ifquery;
    TPckg<TSoInetIfQuery> ifquerypkg(ifquery);
    error = sock.SetOpt(KSoInetEnumInterfaces, KSolInetIfCtrl);
    if (error == KErrNone)
    {
	iTextEdit->ClearL();
	while(sock.GetOpt(KSoInetNextInterface, KSolInetIfCtrl, ifinfopkg) == KErrNone)
	{
	    TSoInetInterfaceInfo &info = ifinfopkg();
	    ifquerypkg().iName = info.iName;
	    error = sock.GetOpt(KSoInetIfQueryByName, KSolInetIfQuery, ifquerypkg);
	    
	    if (error != KErrNone) break;
	    if (info.iState == EIfUp)
	    {
		TBuf<40> ifAddress;
		ifAddress.FillZ(40);
		ifinfo.iAddress.Output(ifAddress);
		if ((ifinfo.iName.FindF(_L("wlan")) != KErrNotFound) && (ifAddress.Locate(':') == KErrNotFound))
		{
		    /*TBuf <200> infoMsg;
		    infoMsg.Format(_L("IFNAME: %S, ADDR: %S\n"), &ifinfo.iName, &ifAddress);
		    infoMsg.Format(_L("IFNAME: %S, ADDR: %S\n"), &ifinfo.iName, &ifAddress);
		    iTextEdit->WriteL(infoMsg);*/
		    iTextEdit->ClearL();
		    iEncoderModel->EncodeL(ifAddress);
		    iTextEdit->WriteL(ifAddress);
		    break;
		}
	    }
      }
    }
    
    if (error != KErrNone) ShowErrorL(error);
    sock.Close(); 
    rss.Close(); 
}

#include <utf.h> 

void CSquirrelGeneratorView::DoGenerateFromTemplateL(const TInt aCmd)
{
    
    switch(aCmd)
    {
	case ECmdSelectMemo:
	{
	    HBufC* out = CNotepadApi::FetchMemoL();
	    if (out)
	    {
		iEncoderModel->EncodeL(*out);
		iTextEdit->ClearL();
		iTextEdit->WriteL(*out);
		iContainer->UpdateControl(0, EFalse);
		delete out;
		out = NULL;
	    }
	    break;
	}

	case ECmdSelectContact:
	{
	    CContactSelectionDlg* sel = CContactSelectionDlg::NewL();
	    TInt index = sel->ShowL();
	    if (index >= 0) {
	     	iTextEdit->ClearL();
		iEncoderModel->EncodeL(sel->GetVCardL(index));	
		/*TPtr8 p = sel->GetVCardL(index);
		HBufC16* out = CnvUtfConverter::ConvertToUnicodeFromUtf8L(p);
		iTextEdit->WriteL(*out);*/
	    }

	    CleanupStack::PopAndDestroy(sel);
	    break;
	}
	case ECmdSelectCalendar:
	    break;
	case ECmdSelectIMEI:
	    DoGenerateFromPhoneIMEIL(ETrue);
	    break;
	case ECmdSelectIP:
	    DoGenerateFromIPAddressL();
	    break;
	default: break;
    }
}



void CSquirrelGeneratorView::DoSaveImageL()
{
    TFileName fp;
    if (!DriveSelectionL(fp, R_DRIVESELECTION_TITLE)) return;

    CAknFileSelectionDialog* d = CAknFileSelectionDialog::NewL(ECFDDialogTypeSave);//, R_SAVEPATHSELECTION_DIALOG);
    CleanupStack::PushL(d);
    TBool ok = d->ExecuteL(fp);
    CleanupStack::PopAndDestroy(d);
    if (ok) iEncoderModel->SaveImageL(fp);
    else DoSaveImageL();

}


// -----------------------------------------------------------------------------
// CSquirrelGeneratorView::SquirrelGeneratorView()
// C++ default constructor can NOT contain any code, that might leave.
// -----------------------------------------------------------------------------
//
CSquirrelGeneratorView::CSquirrelGeneratorView()
{
    iContainer = NULL;
    iImageView = NULL;
    iEncoderModel = NULL;
    iTextEdit = NULL;
    iTelephony = NULL;
}

// -----------------------------------------------------------------------------
// CSquirrelGeneratorView::~SquirrelGeneratorView()
// Destructor.
// -----------------------------------------------------------------------------
//
CSquirrelGeneratorView::~CSquirrelGeneratorView()
{
    if (iEncoderModel) delete iEncoderModel;
    if (iTelephony) delete iTelephony;
}


TUid CSquirrelGeneratorView::Id() const 
{ 
    return TUid::Uid(EGeneratorView); 
}

void CSquirrelGeneratorView::HandleCommandL( TInt aCommand )
{
    switch( aCommand )
    {
	case ECmdSelectMemo:
	case ECmdSelectContact:
	case ECmdSelectCalendar:
	case ECmdSelectBookmark:
	case ECmdSelectIP:
	case ECmdSelectIMEI:
	    if (iEncoderModel) DoGenerateFromTemplateL(aCommand);
	    break;

	case ECmdGenerateImage:
	    DoGenerateL();
	    break;

	case ECmdSaveImage:
	    DoSaveImageL();
	    break;

	case ECmdSendImage:
	    if (iEncoderModel) iEncoderModel->SendImageL();
	    break;
	case ECmdOpenGeneratorSettings:
	    CGeneratorSettingsDlg* d = CGeneratorSettingsDlg::NewL(iEncoderModel->Settings());
	    d->ShowLD();
	    SetAppTitleL(NULL, R_GENERATOR_TITLE);
	    break;
	default:
	    AppUi()->HandleCommandL(aCommand);  
	    break;
    }
}


void CSquirrelGeneratorView::HandleViewRectChange()
{
    if (iContainer){
	iContainer->SetRect(ClientRect());
	LayoutControls();
	iEncoderModel->RedrawQRCImageL(iImageView->Size());
	iContainer->UpdateControl(0, EFalse);

    }
    
}

void CSquirrelGeneratorView::DoActivateL( const TVwsViewId& /*aPrevViewId*/,
	TUid /*aCustomMessageId*/,
	const TDesC8& /*aCustomMessage*/ )
{
    
    SetAppTitleL(NULL, R_GENERATOR_TITLE);
    iContainer = new (ELeave) CViewContainer;
    iContainer->ConstructL(ClientRect(), ETrue /* Skinned Container */);
    iContainer->SetMopParent(this);
    SetupControlsL();
    AppUi()->AddToStackL(*this, iContainer); 
}

void CSquirrelGeneratorView::DoDeactivate()
{
 
    if (iContainer)
    {
	AppUi()->RemoveFromStack(iContainer);
	delete iContainer;
    }
}



void CSquirrelGeneratorView::DynInitMenuPaneL(TInt aResourceId, CEikMenuPane *aMenuPane)
{
    if (!iTextEdit || !iEncoderModel || aResourceId != R_GENERATORVIEW_MENU) return;

    TBool dimm = iTextEdit->IsEmpty() || !iEncoderModel->Finished();
    aMenuPane->SetItemDimmed(ECmdGenerateImage, dimm);
    dimm = !iEncoderModel->ImageReady();
    aMenuPane->SetItemDimmed(ECmdExport, dimm);
}


// End of File
