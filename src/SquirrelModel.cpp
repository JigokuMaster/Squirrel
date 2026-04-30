#include <utf.h> // CnvUtfConverter
//#include <f32file.h> // TParsePtrC
#include <s32strm.h>
#include <bautils.h>
#include <pathinfo.h>
#include <caknmemoryselectiondialog.h>
#include <apparc.h> // CApaApplication
#include <apgcli.h> // RApaLsSession
#include <aknlistquerydialog.h>
#include <npdapi.h>
#include <cmessagedata.h>
#include <sendui.h>
#include <baclipb.h>
#include <apmstd.h>
#include <icl/imageprocessor.h>
#include <textresolver.h>
#include <wchar.h>
#include "SquirrelAppUi.h"
#include "SquirrelModel.h"
#include "Squirrel.hrh"
#include "Squirrel.rsg"
#include "HistoryStore.h"



_LIT(KGeneratorSettingsFile, "GeneratorSettings.dat");

_LIT(KDefaultImageFileName, "Squirrel");

#define RGB2BGR(rgb) ((rgb & 0xFF) << 16) | (rgb & 0xFF00) | (rgb >> 16)


// utility functions 

void GetPrivateFilePathL(RFs& aRfs, TFileName &aFilePath, const TDesC &aFileName)
{

    User::LeaveIfError(aRfs.PrivatePath(aFilePath));

    // insert the "drive:" to the private path
    /*TParsePtrC parse((CEikonEnv::Static()->EikAppUi()->Application())->AppFullName());*/
    aFilePath.Insert(0, RProcess().FileName().Left(2));
    aFilePath.Append(aFileName);
}

static void CopyTextToClipboardL(const TDesC &aText)
{

    CClipboard* cb = CClipboard::NewForWritingLC( CCoeEnv::Static()->FsSession());
    CPlainText* plainText = CPlainText::NewL();
    CleanupStack::PushL(plainText);
    plainText->InsertL(0, aText);	    
    plainText->CopyToStoreL(cb->Store(), cb->StreamDictionary(), 0, plainText->DocumentLength());
    cb->CommitL();
    CleanupStack::PopAndDestroy(plainText);
    CleanupStack::PopAndDestroy(cb);
}

static void ViewTextWithNotePadL(const TDesC &aText, const TBool &aSaveContent=EFalse)
{
    TInt ret;
    HBufC* content;
    
    content = CNotepadApi::ExecTextEditorL(ret, aText);
    if( (ret == KErrNone) && aSaveContent)
    {
	CNotepadApi::AddContentL(content->Des());
    }
    if(content != NULL) delete content;
	
}


/*static*/ void ShowErrorL(const TInt aErrorCode)
{

    TBuf<256> msg;
    CTextResolver* textResolver;
    textResolver = CTextResolver::NewLC();

    msg.Append(textResolver->ResolveErrorString(aErrorCode));
    CleanupStack::PopAndDestroy();//textResolver  
    ErrorNoteL(&msg);

}


void SendFileL(const TDesC &aFilePath)
{

    CSendUi* sendUi = CSendUi::NewL();
    CleanupStack::PushL(sendUi);
        
    CMessageData* messageData = CMessageData::NewL();
    CleanupStack::PushL(messageData);

    messageData->AppendAttachmentL(aFilePath);

    sendUi->ShowQueryAndSendL(messageData, TSendingCapabilities(0, 0, TSendingCapabilities::ESupportsAttachments));

    CleanupStack::PopAndDestroy(2); // sendUi, messageData
}



// Implementation of CGeneratorSettings

CGeneratorSettings::CGeneratorSettings(QRCEncoder* aEncoder)
{
   iFileStore = NULL;
   iEncoder = aEncoder;
}

CGeneratorSettings::~CGeneratorSettings()
{
    if (iFileStore) delete iFileStore;
}

CGeneratorSettings* CGeneratorSettings::NewL(QRCEncoder* aEncoder)
{
    CGeneratorSettings* self = new(ELeave) CGeneratorSettings(aEncoder);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;    
}

void CGeneratorSettings::ConstructL()
{
    TInt err(KErrNone);
    TBool r(ETrue);
    TRAP(err, r = RestoreL());
    if (!r || err != KErrNone)
    {
	iScale = 8; // N pixels per modules 
	iBorder = 4; // 4 modules 
	iImageFormatIndex = 0; // png
	iColors[0] = 0xFFFFFF;//KRgbWhite.Value();
	iColors[1] = 0x000000;//KRgbBlack.Value();
	CreateStoreL();
	StoreL(ETrue);
    }
}

TBool CGeneratorSettings::RestoreL()
{
    RFs& rfs = CCoeEnv::Static()->FsSession();
    TFileName fp;

    GetPrivateFilePathL(rfs, fp, KGeneratorSettingsFile);

    if (!BaflUtils::FileExists(rfs,  fp)) return EFalse;

    iFileStore = CPermanentFileStore::OpenL(rfs, fp, EFileRead|EFileWrite);
    iFileStore->SetTypeL(iFileStore->Layout());

    RStoreReadStream instream;
    instream.OpenLC(*iFileStore, iFileStore->Root());
    TInt minVersion = instream.ReadInt32L();
    TInt maxVersion = instream.ReadInt32L();
    TInt ecl = instream.ReadInt32L();
    iEncoder->SetParams(ecl, minVersion, maxVersion);

    iScale = instream.ReadInt32L();
    iBorder = instream.ReadInt32L();
    iImageFormatIndex = instream.ReadInt32L();
    iColors[0] = instream.ReadUint32L();
    iColors[1] = instream.ReadUint32L();

    CleanupStack::PopAndDestroy(); // instream
    return ETrue;
}


void CGeneratorSettings::CreateStoreL()
{
    RFs& rfs = CCoeEnv::Static()->FsSession();
    TFileName fp;

    GetPrivateFilePathL(rfs, fp, KGeneratorSettingsFile);
    iFileStore = CPermanentFileStore::ReplaceL(rfs, fp, EFileRead|EFileWrite);   
    iFileStore->SetTypeL(iFileStore->Layout());
}

void CGeneratorSettings::StoreL(TBool aCreate)
{

    RStoreWriteStream outstream;
    if (aCreate){
    
	TStreamId id = outstream.CreateLC(*iFileStore);
	iFileStore->SetRootL(id);

    }
    else {
	outstream.OpenLC(*iFileStore, iFileStore->Root());
    }

       
    outstream.WriteInt32L(iEncoder->MinVersion());
    outstream.WriteInt32L(iEncoder->MaxVersion());
    outstream.WriteInt32L(iEncoder->Ecl());
    outstream.WriteInt32L(iScale); 
    outstream.WriteInt32L(iBorder);
    outstream.WriteInt32L(iImageFormatIndex); 
    outstream.WriteUint32L(iColors[0]);
    outstream.WriteUint32L(iColors[1]);

    outstream.CommitL();
    iFileStore->CommitL();
    CleanupStack::PopAndDestroy(); // outstream
}

// id of output image 
TUid CGeneratorSettings::ImageFormat(TFileName &aFileName) const 
{
    aFileName.Append(KDefaultImageFileName);
    if (iImageFormatIndex == 1)
    {   
	aFileName.Append(_L(".jpg"));
	return KImageTypeJPGUid;
    }
    if (iImageFormatIndex == 2)
    {
	aFileName.Append(_L(".bmp"));
	return KImageTypeBMPUid;
    }
    aFileName.Append(_L(".png"));
    return KImageTypePNGUid; // 0 default
}


// Implementation of CQRCEncoderModel

CQRCEncoderModel::CQRCEncoderModel(TSize aSize):CActive(EPriorityStandard)
{
    iUtf8Text = NULL; 
    iCachedBitmapDrawer = NULL;
    iBitmapDrawer = CBitmapDrawer::NewL(aSize);
    iImageEncoder = NULL;
    iSettings = NULL;
    iWaitDialog = NULL;
    iInfoPopup = NULL;
    iState = EIdle;
    CActiveScheduler::Add(this);
}

CQRCEncoderModel::~CQRCEncoderModel()
{
    if (iBitmapDrawer) delete iBitmapDrawer;
    if (iCachedBitmapDrawer) delete iCachedBitmapDrawer;
    if (iUtf8Text) delete iUtf8Text;
    if (iImageEncoder) delete iImageEncoder;
    if (iSettings) delete iSettings;
    if (iWaitDialog)
    {
	iWaitDialog->ProcessFinishedL();
	delete iWaitDialog;
	iWaitDialog = NULL;
    }
    if (iInfoPopup) delete iInfoPopup;
}

CFbsBitmap* CQRCEncoderModel::Bitmap()
{
    if (!iBitmapDrawer) return NULL;
    return iBitmapDrawer->Bitmap();
}

CFbsBitmap* CQRCEncoderModel::FullBitmap()
{
    if (!iCachedBitmapDrawer) return NULL;
    return iCachedBitmapDrawer->Bitmap();
}


void CQRCEncoderModel::EncodeL(const TDesC8 &aText)
{

    iState = EEncodingText;

    //const char* utf8TextPtr = reinterpret_cast<const char*>(aText.PtrZ());

    const char* utf8TextPtr = reinterpret_cast<const char*>(aText.Ptr());

    TInt qrcSize = iEncoder.EncodeText(utf8TextPtr);

    if (qrcSize > 0)
    {
	DrawQRCImageL(qrcSize);
	// QR code image is ready! 
	ShowInfoMessageL(R_GENERATOR_MSG2);
    }
    else
    {
	// Couldn't generate QR code!
	ErrorNoteL(NULL, R_GENERATOR_MSG1);
    }

    iState = EIdle;
}



void CQRCEncoderModel::EncodeL(const TDesC16 &aText)
{
    if (iUtf8Text) delete iUtf8Text;
    
    iUtf8Text = HBufC8::New(aText.Size());
    if (!iUtf8Text)
    {
	iBitmapDrawer->ClearL(KRgbRed);
	// Show Error
	return;
    }

    TPtr8 p = iUtf8Text->Des();
    TInt err = CnvUtfConverter::ConvertFromUnicodeToUtf8(p/*iUtf8Text->Des()*/, aText);

    if (err != KErrNone)
    {
	ShowErrorL(err);
    }
   
    else EncodeL(p);
}

void CQRCEncoderModel::SaveImageL(TFileName &aFilePath)
{
    CFbsBitmap *bitmap = FullBitmap();
    RFs rfs = CEikonEnv::Static()->FsSession();

    if (iImageEncoder) delete iImageEncoder;

    /*if ( aFilePath[0] == 'C')
    {
	aFilePath.Copy(PathInfo::PhoneMemoryRootPath());
    }
    aFilePath.Append(PathInfo::ImagesPath());
    */
    const TUid format = iSettings->ImageFormat(aFilePath); // appends filename + extention

    CApaApplication::GenerateFileName(rfs, aFilePath); // generates a unique filename

    iImageEncoder = CImageEncoder::FileNewL(rfs, aFilePath, CImageEncoder::EOptionAlwaysThread, format);

    iImageEncoder->Convert(&iStatus, *bitmap);
    iState = EEncodingImage;
    SetActive();

    iWaitDialog = new (ELeave) CAknWaitDialog(NULL, ETrue);     
    if (iWaitDialog->ExecuteLD(R_GENERATOR_WAITDIALOG) == 0)
    {
	DoCancel();
    }
    else
    {
    
	// show other dialogs after the WaitDialog is finished. 
    
	if (iState == EError) ShowErrorL(iStatus.Int());
    
	else ShowInfoMessageL(R_GENERATOR_MSG3);
    }

    iWaitDialog = NULL;
    iState = EIdle;
}

void CQRCEncoderModel::SendImageL()
{
    if (!ImageReady()) return;

    CFbsBitmap *bitmap = FullBitmap();
    RFs rfs = CEikonEnv::Static()->FsSession();

    if (iImageEncoder) delete iImageEncoder;

    TFileName fp(_L("D:\\"));
    const TUid format = iSettings->ImageFormat(fp); // appends filename + extention


    iImageEncoder = CImageEncoder::FileNewL(rfs, fp, CImageEncoder::EOptionAlwaysThread, format);

    iImageEncoder->Convert(&iStatus, *bitmap);
    iState = EEncodingImage;
    SetActive();

    iWaitDialog = new (ELeave) CAknWaitDialog(NULL, ETrue);     
    if (iWaitDialog->ExecuteLD(R_GENERATOR_WAITDIALOG) == 0)
    {
	DoCancel();
    }
    else
    {
    
	// show other dialogs after the WaitDialog is finished. 
    
	if (iState == EError) ShowErrorL(iStatus.Int());
    
	else
	{
	    SendFileL(fp);
	    BaflUtils::DeleteFile(rfs, fp);
	}
    }

    iWaitDialog = NULL;
    iState = EIdle;

}

TBool CQRCEncoderModel::ImageReady()
{
    return Finished() && (FullBitmap() != NULL);
}

TBool CQRCEncoderModel::Finished()
{
    return (iState == EIdle);
}

void CQRCEncoderModel::DrawQRCImageL(const TInt &aSize)
{
    TInt scale = iSettings->iScale;
    TInt e = (iSettings->iBorder)*scale;
    TInt w = (aSize*scale)+(e*2);
    TInt h = (aSize*scale)+(e*2);
    TSize bmSize(w, h);

    if (iCachedBitmapDrawer) delete iCachedBitmapDrawer;

    iCachedBitmapDrawer = CBitmapDrawer::NewL(bmSize);

    TUint32 c0 = RGB2BGR(iSettings->iColors[0]);
    TUint32 c1 = RGB2BGR(iSettings->iColors[1]);

    iCachedBitmapDrawer->ClearL(TRgb(c0));
    iCachedBitmapDrawer->SetBrushColorL(TRgb(c1), ETrue);

    for (TInt y = 0; y < aSize; y++)
    {
	for (TInt x = 0; x < aSize; x++)
	{
	   if (iEncoder.GetModule(x, y)){

	       TRect r(TPoint(e+x*scale, e+y*scale), TSize(scale, scale));
	       iCachedBitmapDrawer->DrawRectL(r);
	   }	   	   
	}
    }

    TRect dst(iBitmapDrawer->Size());
    iBitmapDrawer->Gc()->DrawBitmap(dst, iCachedBitmapDrawer->Bitmap());
}

void CQRCEncoderModel::RedrawQRCImageL(TSize aSize)
{
    if (iCachedBitmapDrawer)
    {
	iBitmapDrawer->ResizeL(aSize);
	iBitmapDrawer->ClearL();
	TRect dst(TPoint(0,0), iBitmapDrawer->Size());
	iBitmapDrawer->Gc()->DrawBitmap(dst, iCachedBitmapDrawer->Bitmap());
    }
}


void CQRCEncoderModel::ShowInfoMessageL(TInt aResourceId, TInt aAlignment, TInt aXpos, TInt aYPos, TInt aShowTime)
{
    if (!iInfoPopup)
    {
	iInfoPopup = CAknInfoPopupNoteController::NewL();
    }
       
    if (aResourceId != 0)
    {
	HBufC *textResource = CCoeEnv::Static()->AllocReadResourceLC(aResourceId);	
	iInfoPopup->SetTextL(*textResource);
	CleanupStack::PopAndDestroy(textResource);
    }

    if (aAlignment == -1) aAlignment = EHLeftVTop;
    
    iInfoPopup->SetPositionAndAlignment(TPoint(aXpos, aYPos), (TGulAlignmentValue)aAlignment);

    if (aShowTime > 0) iInfoPopup->SetTimePopupInView(aShowTime);


    iInfoPopup->ShowInfoPopupNote();
}



void CQRCEncoderModel::RunL()
{
    switch(iState)
    {
	case EEncodingImage:
	
	    if (iWaitDialog) iWaitDialog->ProcessFinishedL();

	    if (iStatus.Int() != KErrNone)
	    {

		iState = EError;
	    }
	    break;
	default:
	    break;
    }
}

void CQRCEncoderModel::DoCancel()
{
    if ( iImageEncoder)
    {
        iImageEncoder->Cancel();
    }
}

void CQRCEncoderModel::CreateSettingsL()
{
    iSettings = CGeneratorSettings::NewL(&iEncoder);
}

CGeneratorSettings* CQRCEncoderModel::Settings()
{
    return iSettings;
}


// Implementation of CDecoderModel

CDecoderModel::CDecoderModel():CActive(EPriorityStandard)
{
    iState = EIdle;
    iViewContainer = NULL;
    iImageView = NULL;
    iImageBitmap = NULL;
    iWaitDialog = NULL;
    iDataAvailable = EFalse;
    CActiveScheduler::Add(this);
}

CDecoderModel::~CDecoderModel()
{
    if (iViewContainer){
	delete iViewContainer;
	iViewContainer = NULL;
    }

    if (iImageHandler){
	delete iImageHandler;
	iImageHandler = NULL;
    }

    if (iImageBitmap){
	delete iImageBitmap;
	iImageBitmap = NULL;
    }

    if (iWaitDialog)
    {
	iWaitDialog->ProcessFinishedL();
	delete iWaitDialog;
	iWaitDialog = NULL;
    }

}


CViewContainer* CDecoderModel::SetupViewL(const TRect &aRect)
{

    if (iViewContainer)
    {
	iViewContainer->SetRect(aRect);
	return iViewContainer;
    }

    iViewContainer = new (ELeave) CViewContainer;
    iViewContainer->ConstructL(aRect, ETrue /* Skinned Container */);

    iImageView = new (ELeave) CEikImage;
    iImageView->SetPictureOwnedExternally(ETrue);
    iImageView->SetContainerWindowL(*iViewContainer);
    iViewContainer->AddControlL(iImageView);
    DrawCodeImageL();
    return iViewContainer;

}

CViewContainer* CDecoderModel::GetView()
{
    return iViewContainer;
}

void CDecoderModel::DrawCodeImageL()
{
    if (!iViewContainer) return;

    TSize viewSize = iViewContainer->Size();
    const CFbsBitmap* bm = iImageView->Bitmap();
    TInt x = 0;
    TInt y = 0;

    if (bm)
    {
	TSize bmSize = bm->SizeInPixels(); // size of the scaled image

	if (iDecoderResult.has_qrcode && iDataAvailable)
	{

	    TInt lineSize = 3;

	    CBitmapDrawer* bitmapDrawer = CBitmapDrawer::NewL((CFbsBitmap*)bm);
	    bitmapDrawer->Gc()->SetPenColor(KRgbGreen);
	    bitmapDrawer->Gc()->SetPenSize(TSize(lineSize, lineSize));
	    
	    TSize origBMSize = iImageBitmap->SizeInPixels(); // size of the original image
	    float scale_x = (float)bmSize.iWidth / (float)origBMSize.iWidth;
	    float scale_y = (float)bmSize.iHeight / (float)origBMSize.iHeight;

	    for (TInt i=0; i < 4; i++)
	    {
		TInt j = (i + 1) % 4;
		TInt x1 = iDecoderResult.code_pos[i][0];
		TInt y1 = iDecoderResult.code_pos[i][1];
	    
		TInt x2 = iDecoderResult.code_pos[j][0];
		TInt y2 = iDecoderResult.code_pos[j][1];
	    
		// Scale points
		x1 *= scale_x; y1 *= scale_y;	
		x2 *= scale_x; y2 *= scale_y;
	      
		TPoint start(x1, y1);	
		TPoint end(x2, y2);
		bitmapDrawer->Gc()->DrawLine(start, end);
	    }
	    delete bitmapDrawer;
	}

	x = Abs(viewSize.iWidth - bmSize.iWidth)/2;
	y = Abs(viewSize.iHeight - bmSize.iHeight)/2;
    
	iImageView->SetPosition(TPoint(x, y)); // center the image
	iImageView->SetSize(bmSize);
    }

    else {
	TRect rect(TPoint(x, y), viewSize);
	iImageView->SetRect(rect);
    }

    iViewContainer->DrawNow();

}

void CDecoderModel::RedrawCodeImageL()
{
   // scale to fit the container size
    iState = ERedrawImage;
    TRAPD(err, iImageHandler->ScaleL( iViewContainer->Size() ));
    if (err != KErrNone)
    {
	ShowErrorL(err);
    }
}


TBool CDecoderModel::DecodeBitmap(CFbsBitmap* aBitmap)
{
    TSize size = aBitmap->SizeInPixels();
    TInt w = size.iWidth, h = size.iHeight;

    TUint8 *data = new TUint8[w*h];
    if (!data)
    {
	StepCompleted(EError, KErrNoMemory);
	return EFalse;
    }

    ScanHelper::BGRBitmapToGray(aBitmap, data);
   
    iState = EDecodingImage;
    
    /*TInt ret = iDecoder.DecodeImageData(data, size.iWidth, size.iHeight); 
    */

    Mem::FillZ(&iDecoderResult, sizeof(DecoderResult));

    TInt ret = iDecoder.ExtractCode(data, size.iWidth, size.iHeight, &iDecoderResult);

    delete data;

    iDataAvailable = ret > 0;
   
    if (!iDataAvailable)
    {
	StepCompleted(EError, (ret == 0) ? KErrNotFound : KErrNone);
	return EFalse;
    }

    StepCompleted(ECodeExtracted);
    return ETrue;
}

void CDecoderModel::SetBitmap(CFbsBitmap* aBitmap)
{
    if (iImageBitmap) delete iImageBitmap;   
    iImageBitmap = aBitmap;
}
	
void CDecoderModel::StartDecodeL(CFbsBitmap* aBitmap)
{
    if (aBitmap) SetBitmap(aBitmap);

    if (!iImageBitmap)
    {
	ShowErrorL(KErrArgument);
	return;
    }
    if (!iImageHandler)
    {

	iImageHandler = NOCRUtils::CImageHandler::NewL(iImageBitmap, *this);
    }
    else
    {
	iImageHandler->SetBitmap(iImageBitmap);
    }

    // scale to fit the container size
    TRAPD(err, iImageHandler->ScaleL( iViewContainer->Size() ));
    if (err != KErrNone)
    {
	ShowErrorL(err);
    }
    else{
 
	iState = ELoadingImage;
	iWaitDialog = new (ELeave) CAknWaitDialog(NULL, ETrue);
        
	if (iWaitDialog->ExecuteLD(R_DECODER_WAITDIALOG) == 0)
	{
            DoCancel();
        }
	else DecodeFinishedL();
	iWaitDialog = NULL;
    }

}


void CDecoderModel::StartDecodeL(const TFileName &aFileName)
{
   

    if (!iImageHandler)
    {

	iImageBitmap = new (ELeave) CFbsBitmap;	
	iImageHandler = NOCRUtils::CImageHandler::NewL(iImageBitmap, *this);
    }

    RFs& rfs = CCoeEnv::Static()->FsSession();
    TRAPD(err, iImageHandler->LoadFileAndScaleL(rfs, aFileName, iViewContainer->Size() ));
    
    if (err != KErrNone)
    {
	ShowErrorL(err);
    }
    else{
 
	iState = ELoadingImage;
	iWaitDialog = new (ELeave) CAknWaitDialog(NULL, ETrue);
        
	if (iWaitDialog->ExecuteLD(R_DECODER_WAITDIALOG) == 0)
	{
            DoCancel();
        }
	else DecodeFinishedL(&aFileName);
	iWaitDialog = NULL;
    }
}

void CDecoderModel::DecodeFinishedL(const TDesC *aFileName)
{
    if (iState == ECodeExtracted)
    {
	ShowDataMenuL();
	if (aFileName)
	{
	    CEikonEnv* env = CEikonEnv::Static();	
	    CHistoryStore* hs = STATIC_CAST(CSquirrelAppUi*, env->EikAppUi())->HistoryStore(); 
	    if (hs) hs->AddItemL(*aFileName);
	}

    }

    else if (iState == EError)
    {
	ShowError();
    }
	
    iStatus = KErrNone; // reset
    iState = EIdle;
}



void CDecoderModel::ShowError()
{
    TInt errCode = iStatus.Int(); 
    if (errCode == KErrNotFound)
    {
	ErrorNoteL(NULL, R_DECODER_MSG1); // qrcode not found
    }

    else if (errCode != KErrNone)
    {
	ShowErrorL(errCode); // error while loading image

    }
    // errno set by the decoder.
    else
    {
	const wchar_t* error = iDecoder.GetWError();
	if (error)
	{
	    TPtrC errorPtr((TUint16*)error);
	    ErrorNoteL(&errorPtr);
	}
	else {
	    _LIT(KUnknownErr, "Unknown error!");
	    ErrorNoteL(&KUnknownErr);
	}

    }
}

void CDecoderModel::SendPayloadL()
{

    RFs& rfs = CCoeEnv::Static()->FsSession();
    RFile f;  
   
    TFileName fp(_L("D:\\Squirrel"));
 
    switch (iDecoderResult.payload_type)
    {
	case PL_BINARY:
	    fp.Append(_L(".bin"));
	    break;

	case PL_CONTACT:
	    fp.Append(_L(".vcf"));

	case PL_CALENDAR:
	    fp.Append(_L(".vcs"));
	    break;

	case PL_CALENDARv2:
	    fp.Append(_L(".ics"));
	    break;

	default:
	    fp.Append(_L(".txt"));
	    break;
    }

    // write data to temp file
   
    TPtrC8 dataPtr((TUint8*) iDecoderResult.payload, iDecoderResult.payload_len);
 
    User::LeaveIfError(f.Replace(rfs, fp, EFileWrite/*|EFileShareAny*/));
    User::LeaveIfError(f.Write(dataPtr));
    f.Close();
    SendFileL(fp);
    BaflUtils::DeleteFile(rfs, fp); 
}

#if 0
void CDecoderModel::ViewPayloadL(TDesC &aPayload)
{

    TPtrC8 payload8((TUint8*)iDecoderResult.payload, iDecoderResult.payload_len);

    switch (iDecoderResult.payload_type)
    {
	/*case PL_TEXT:
	case PL_BINARY: // encoded as hex string
	    ViewTextWithNotePadL(aPayload);
	    break;
	case PL_URL:
	    break;
	*/
	/*case PL_CONTACT:
	case PL_CALENDAR:
	    break;
	*/
	default:
	    ViewTextWithNotePadL(aPayload);
	    break;
    }
}
#endif

TBool CDecoderModel::DataAvailable()
{
    return iDataAvailable;
}

void CDecoderModel::ShowDataMenuL(TBool aNewData)
{

    static TUint16* payload = NULL;  
    static TInt len = 0;
    if (aNewData)
    {
	if (iDecoderResult.has_qrcode)
	{
	    payload = (TUint16*) iDecoder.ParseAndEncodePayload(&iDecoderResult, len, "UTF-16LE");
	    
	}
	else
	{
	    payload = (TUint16*) iDecoder.EncodePayload(&iDecoderResult, len, "UTF-16LE");

	}
    }

    if (!payload)
    {
	if (aNewData)
	{
	    ShowError();
	}
	else
	{
	    // show some message ...
	}
	return;
    }

    TPtrC payloadPtr(payload, len); 
 
    TInt item(0);

    CAknListQueryDialog* d = new (ELeave)CAknListQueryDialog( &item );
    d->PrepareLC(R_QRCDATA_MENU_DIALOG);

    if (iDecoderResult.payload_type == PL_BINARY)
    {
	HBufC* headerText = CCoeEnv::Static()->AllocReadResourceLC(R_DATA_MENU_TITLE2);
	d->SetHeaderTextL(*headerText);
	CleanupStack::PopAndDestroy(headerText);
    }

    if ( d->RunLD() )

    {
        switch ( item )
        {
	    case 0: // View
		//ViewPayloadL(payloadPtr);
		ViewTextWithNotePadL(payloadPtr);

		break;
	    case 1: // Save
		ViewTextWithNotePadL(payloadPtr, ETrue);
		break;
	    case 2: // Copy 
		CopyTextToClipboardL(payloadPtr);
		break;
	    case 3:// send 	    	
		SendPayloadL();
		break;
	    default:
		break;
	}
    }
}

void CDecoderModel::StepCompleted(TState aState, TInt err)
{
    iState = aState; 
    SetActive();
    TRequestStatus* status = &iStatus;
    User::RequestComplete(status, err);
}

void CDecoderModel::RunL()
{
    // Handle cancel ??

    if (iState == EImageLoaded)
    {
	DecodeBitmap(iImageBitmap);
    }

    /*else if (iState == EImageDecoded)
    {
	iState = EExtractingCode;

	Mem::FillZ(&iDecoderResult, sizeof(DecoderResult));
	
	if (!iDecoder.ExtractCode(0, &iDecoderResult))
	{

	    iDataAvailable = EFalse;
	    StepCompleted(EError);
	}
 	else {

	    iDataAvailable = ETrue;
	    StepCompleted(ECodeExtracted);
	}
    }
    */

    else if (iState == ECodeExtracted || iState == EError)
    {
	DrawCodeImageL();
	if (iWaitDialog) iWaitDialog->ProcessFinishedL();
    }

}

void CDecoderModel::DoCancel()
{
    if (iImageHandler) iImageHandler->Cancel();
    //StepCompleted(KErrCanceled);
}

void CDecoderModel::ImageOperationCompleteL(TInt aError)
{

    if (aError != KErrNone)
    {
	StepCompleted(EError, aError);
    }
    else
    {
	iImageView->SetBitmap(iImageHandler->ScaledBitmap());
	if (iState == ERedrawImage)
	{
	    DrawCodeImageL();
	    iState = EIdle;
	}
	else StepCompleted(EImageLoaded);
    }
}

ScanHelper::ScanHelper()
{
    iBuffer = NULL;
    iBufferSize = 0;
}

ScanHelper::~ScanHelper()
{
    if (iBuffer) delete iBuffer;
}

TInt ScanHelper::AllocateBuffer(const TInt aLen)
{
    if (iBuffer) delete iBuffer;
    iBuffer = new TUint8[aLen];
    if (iBuffer) iBufferSize = aLen;
    return iBuffer ? KErrNone : KErrNoMemory;
}

TInt ScanHelper::FindCode(CFbsBitmap* aBitmap)
{

    TSize size = aBitmap->SizeInPixels();
    TInt w = size.iWidth, h = size.iHeight;
    TInt len = w*h;

    if (iBufferSize != len)
    {
	if (AllocateBuffer(len) != KErrNone) return -1;
	if (!iDecoder.Init(w, h)) return -1;
    
    }

    ScanHelper::BGRBitmapToGray(aBitmap, iBuffer);
 
    TInt ret = iDecoder.DecodeImageData(iBuffer, len); 

    /*if (ret > 0)
    {
	DecoderResult res;
	ret = iDecoder.ExtractCode(0, &res);
    }
    */
    return ret;
}


const TUint16* ScanHelper::GetError()
{
    return (const TUint16*) iDecoder.GetWError();
}


void ScanHelper::BGRBitmapToGray(CFbsBitmap* aBitmap, TUint8* &aBuffer)
{


    TSize size = aBitmap->SizeInPixels();
    TInt w = size.iWidth, h = size.iHeight;

    TBitmapUtil u(aBitmap);
    u.Begin(TPoint(0, 0));
    TUint8* p = aBuffer;
    TRgb pix;
    for(TInt y=0; y < h; y++)
    {
	for (TInt x=0; x < w; x++)
	{
	    //TRgb pix(u.GetPixel());	
	    pix.SetInternal(u.GetPixel());
	    TUint8 mono = (TUint8) TColorConvertor::RgbToMonochrome(pix);     
	    (*p++) = mono;
	    u.SetPos(TPoint(x, y));
	}
    }
    u.End();
}


