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
#include "SquirrelAppUi.h"
#include "SquirrelScannerView.h"
#include "Squirrel.rsg"
#include "Squirrel.hrh"
#include "uicommons.h"


/*** Some stuff here were copied from the PyS60 camera module (MisoPhotoTaker)
***/

static TInt CopyBitmap(CFbsBitmap* aFrame, CFbsBitmap* &aFrameCopy)
{
    TInt err = KErrNone;
    // CFbsBitmap copying from http://forum.newlc.com/index.php/topic,10093.0.html
  
    TRAP(err, {
    aFrameCopy = new (ELeave) CFbsBitmap();
    CleanupStack::PushL(aFrameCopy);
    aFrameCopy->Create(aFrame->SizeInPixels(), aFrame->DisplayMode());
    
    CFbsBitmapDevice* fbsDev = CFbsBitmapDevice::NewL(aFrameCopy);
    CleanupStack::PushL(fbsDev);
    CFbsBitGc* fbsGc = CFbsBitGc::NewL();
    CleanupStack::PushL(fbsGc);
    fbsGc->Activate(fbsDev);
    fbsGc->BitBlt(TPoint(0,0), aFrame);
    
    CleanupStack::PopAndDestroy(2); // fbsDev, fbsGc
    CleanupStack::Pop(1); // aFrameCopy 
    });
  return err;
}

// implementation of CCamScanView

CCamScanView* CCamScanView::NewL(const TRect& aRect, const CCoeControl* aParent)
{
    CCamScanView* self = CCamScanView::NewLC(aRect, aParent);
    CleanupStack::Pop(self);
    return self;
}

CCamScanView* CCamScanView::NewLC(const TRect& aRect,const CCoeControl* aParent)
{
    CCamScanView* self = new (ELeave) CCamScanView;
    CleanupStack::PushL(self);
    self->ConstructL(aRect, aParent);
    return self;
}

void CCamScanView::ConstructL(const TRect& aRect ,const CCoeControl* aParent)
{

    const TInt KTimerInterval(3*1000000); // 3 seconds
    iPeriodic = CPeriodic::NewL(CActive::EPriorityIdle);
    iPeriodic->Start(KTimerInterval, KTimerInterval, TCallBack(CCamScanView::TimerCallBack, this));

    iViewFinderSize = aRect.Size();
    if (aParent) CreateWindowL(aParent);
    else CreateWindowL();

    // Set the windows size
    SetRect(aRect);  
    // Activate the window, which makes it ready to be drawn
    ActivateL();


}


CCamScanView::CCamScanView()
{
    iCameraIndex = 0;
    iError = KErrNone;
    iFoundCode = EFalse;
    iDoFindCode = EFalse;
    iViewFinderActivated = EFalse;
}

CCamScanView::~CCamScanView()
{
    delete iPeriodic;

    CleanupCamera();
}


CCamera::TFormat CCamScanView::GetImageMaxFormat()
{

    if (iCameraInfo.iImageFormatsSupported & CCamera::EFormatFbsBitmapColor16M)
    {
	return CCamera::EFormatFbsBitmapColor16M;
    }

    else if (iCameraInfo.iImageFormatsSupported & CCamera::EFormatFbsBitmapColor64K)
    {
	return CCamera::EFormatFbsBitmapColor64K;
    }

    else if (iCameraInfo.iImageFormatsSupported & CCamera::EFormatFbsBitmapColor4K)
    {
	return CCamera::EFormatFbsBitmapColor4K;
    }
}

TInt CCamScanView::GetImageMaxSize()
{
    return iCameraInfo.iNumImageSizesSupported;
}


TInt CCamScanView::SetupCamera(TInt aIndex)
{
    CleanupCamera();
    if (!CCamera::CamerasAvailable())
    {
	iError = KErrHardwareNotAvailable;
    }
    else
    {

	TRAP(iError, {
	    
	    iCamera = CCamera::NewL(*this, aIndex);
	        
	    iCamera->CameraInfo(iCameraInfo);

	    CreateBackBufferL();

	    iCameraIndex = aIndex;

	    });
    }

    if (iError != KErrNone) StartViewFinder();
    else{
	iCameraIndex = -1;
	ShowError();
    }

    return iError;
}


void CCamScanView::CleanupCamera()
{
    iCameraIndex = 0;
    iFoundCode = EFalse;
    if (iCamera)
    {
	StopViewFinder();
	delete iCamera;
	iCamera = NULL;
    }

    ReleaseBackBuffer();
    iError = KErrNone;
}


void CCamScanView::StartViewFinder()
{

    if(iCamera && !iError && !iViewFinderActivated/*!iCamera->ViewFinderActive()*/){
	iCamera->Reserve();
	iViewFinderActivated = ETrue;
    }
}


void CCamScanView::StopViewFinder()
{

    if(iCamera/* && iCamera->ViewFinderActive()*/)
    {
	if (iViewFinderActivated) iCamera->StopViewFinder();
	iCamera->PowerOff();                   
	iCamera->Release();
    }
    iViewFinderActivated = EFalse; 
}

void CCamScanView::Draw(const TRect& aRect) const
{

    CWindowGc& gc = SystemGc();
  
    if(iBackBuffer)
    {
	// Draw backbuffer that has camera picture
	gc.BitBlt(TPoint(0, 0), iBackBuffer);
    }
    else
    {
	gc.SetPenStyle(CGraphicsContext::ENullPen);
	gc.SetBrushColor(KRgbWhite);
	gc.SetBrushStyle(CGraphicsContext::ESolidBrush);
	gc.DrawRect(aRect);
    }

}


void CCamScanView::SizeChanged()
{
    CCoeControl::SizeChanged();
    TSize size = Size(); 
    /*if (size.iWidth > size.iHeight)
    {
        iViewFinderSize = TSize(size.iWidth, size.iWidth);
    }
    else
    {
        iViewFinderSize = TSize(size.iHeight, size.iHeight);
    }*/

    iViewFinderSize = size;
    if(iCameraIndex > (-1) && !iError)
    {	
	// reset  
	SetupCamera(iCameraIndex);
    }
}

void CCamScanView::CreateBackBufferL()
{
    // create back buffer bitmap
    iBackBuffer = new (ELeave) CFbsBitmap;
    
    User::LeaveIfError(iBackBuffer->Create(Size(),EColor16M));
    
    // create back buffer graphics context
    iBackBufferDevice = CFbsBitmapDevice::NewL(iBackBuffer);
    User::LeaveIfError(iBackBufferDevice->CreateContext(iBackBufferContext));
    iBackBufferContext->SetPenStyle(CGraphicsContext::ESolidPen);
    
    iBackBufferContext->SetBrushColor(KRgbBlack);
    iBackBufferContext->Clear();
    iBackBufferContext->UseFont(iEikonEnv->TitleFont());

    iScanHelper.iGc = iBackBufferContext;
}

void CCamScanView::ReleaseBackBuffer()
{
    if (iBackBufferContext)
    {
        delete iBackBufferContext;
        iBackBufferContext = NULL;
    }
    if (iBackBufferDevice)
    {
        delete iBackBufferDevice;
        iBackBufferDevice = NULL;
    }

    if (iBackBuffer)
    {
        delete iBackBuffer;
        iBackBuffer = NULL;
    }
}


void CCamScanView::ShowError()
{
   
    if(iError != KErrNone)
    {
	TRAPD(err, ShowErrorL(iError));
	if (err != KErrNone){
	    CleanupCamera();
	}
    }
}

void CCamScanView::FindCode(CFbsBitmap* aBitmap)
{
    iDoFindCode = EFalse; // reset
    TInt ret = iScanHelper.FindCode(aBitmap);
    if (ret == -1)
    {
	// memory error probably ...
	const TUint16* error = iScanHelper.GetError();
	if (error)
	{
	    TPtrC errorPtr(error);
	    ErrorNoteL(&errorPtr);
	}
 
    }

    iFoundCode = (ret > 0);
    if (iFoundCode)
    {

	CFbsBitmap* bitmapCopy = NULL;
	iError = CopyBitmap(aBitmap, bitmapCopy);
	if (iError == KErrNone)
	{
	    // send bitmap to the decoder.
	    TRAP(iError, STATIC_CAST(CSquirrelAppUi*, iEikonEnv->EikAppUi())->ActivateDecoderViewL(bitmapCopy));
	}
	ShowError();
    }	
}

TInt CCamScanView::TimerCallBack(TAny* aInstance)
{
    CCamScanView* scanner = (CCamScanView*)aInstance;
    scanner->iDoFindCode = ETrue;
    // keep the screen light on.
    User::ResetInactivityTime();
    return 0;
}


void CCamScanView::ReserveComplete(TInt aError)
{
    iError = aError;
    iViewFinderActivated = !aError;
    if (!aError)
    {
	iCamera->PowerOn();
    }
    else{

	ShowError();
    }
}

void CCamScanView::PowerOnComplete(TInt aError)
{
    iError = aError;
    iViewFinderActivated = !aError;
    if(!aError)
    {
    
	TRAP(iError, iCamera->StartViewFinderBitmapsL(iViewFinderSize));

    }  
    ShowError();
}


void CCamScanView::ImageReady(CFbsBitmap* aBitmap, HBufC8* aData, TInt aError)
{
    iError = aError;
}

void CCamScanView::ViewFinderFrameReady(CFbsBitmap& aFrame)
{

    if (iBackBufferContext && !iFoundCode)
    {
        TSize bmpSizeInPixels = aFrame.SizeInPixels();
        TInt xDelta = (Rect().Width() - bmpSizeInPixels.iWidth) / 2;
        TInt yDelta = (Rect().Height() - bmpSizeInPixels.iHeight) / 2;
        TPoint pos( xDelta, yDelta );

        // Copy received viewfinder picture to back buffer
        iBackBufferContext->BitBlt( pos, &aFrame, TRect(TPoint( 0, 0 ), bmpSizeInPixels ));

	if (iDoFindCode) FindCode(&aFrame);
        // Update backbuffer into screen 
        DrawNow();	

    }
}

void CCamScanView::FrameBufferReady(MFrameBuffer* aFrameBuffer, TInt aError){}




// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CSquirrelScannerView::NewL()
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CSquirrelScannerView* CSquirrelScannerView::NewL()
{
    CSquirrelScannerView* self = CSquirrelScannerView::NewLC();
    CleanupStack::Pop(self);
    return self;
}

// -----------------------------------------------------------------------------
// CSquirrelScannerView::NewLC()
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CSquirrelScannerView* CSquirrelScannerView::NewLC()
{
    CSquirrelScannerView* self = new (ELeave) CSquirrelScannerView;
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
}

// -----------------------------------------------------------------------------
// CSquirrelScannerView::ConstructL()
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CSquirrelScannerView::ConstructL()
{
    BaseConstructL(R_SCANNER_VIEW);
}

// -----------------------------------------------------------------------------
// CSquirrelScannerView::SquirrelScannerView()
// C++ default constructor can NOT contain any code, that might leave.
// -----------------------------------------------------------------------------
//
CSquirrelScannerView::CSquirrelScannerView()
{

    iCamScanView = NULL;

}

// -----------------------------------------------------------------------------
// CSquirrelScannerView::~SquirrelScannerView()
// Destructor.
// -----------------------------------------------------------------------------
//
CSquirrelScannerView::~CSquirrelScannerView()
{
}

TUid CSquirrelScannerView::Id() const 
{ 
    return TUid::Uid(EScannerView); 
}

void CSquirrelScannerView::HandleCommandL( TInt aCommand )
{
    switch( aCommand )
    {
	default:
	    AppUi()->HandleCommandL(aCommand);  
	    break;
    }
}

void CSquirrelScannerView::HandleViewRectChange()
{
    CAknView::HandleViewRectChange();
    if (iCamScanView)
    {
	iCamScanView->SetRect(ClientRect());
	AppUi()->ActivateLocalViewL(TUid::Uid(EDummyView));
    }   
}

void CSquirrelScannerView::DoActivateL( const TVwsViewId& /*aPrevViewId*/,
	TUid /*aCustomMessageId*/,
	const TDesC8& /*aCustomMessage*/ )
{

    SetAppTitleL(NULL, R_SCANNER_TITLE);
    iCamScanView = CCamScanView::NewL(ClientRect());
    iCamScanView->SetMopParent(this);
    //iCamScanView->SetupCamera();
    AppUi()->AddToStackL(*this, iCamScanView);  
    Cba()->MakeCommandVisible(EAknSoftkeyOptions,  EFalse);

}

void CSquirrelScannerView::DoDeactivate()
{

    if (iCamScanView)
    {
	AppUi()->RemoveFromViewStack(*this, iCamScanView);
	delete iCamScanView;
	iCamScanView = NULL;
    }   
}

void CSquirrelScannerView::HandleForegroundEventL(TBool aForeground)
{
    if (!iCamScanView) return;

    if (aForeground)
    {
	iCamScanView->StartViewFinder();
    }
    else
    {
	iCamScanView->StopViewFinder();
    }
}

// End of File
