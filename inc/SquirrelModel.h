#ifndef __SQUIRRELMODEL_h__
#define __SQUIRRELMODEL_h__

#include <e32base.h>
#include <fbs.h>
#include <s32file.h>
#include <imageconversion.h>
#include <aknwaitdialog.h>
#include "ImageHandler.h"
#include "SquirrelEngine.h"
#include "uicommons.h"

// utility functions 

void GetPrivateFilePathL(RFs& aRfs, TFileName &aFilePath, const TDesC &aFileName);

void ShowErrorL(const TInt aErrorCode);


class CGeneratorSettings
{

public:

    static CGeneratorSettings* NewL(QRCEncoder* aEncoder);
    virtual ~CGeneratorSettings();
    void StoreL(TBool aCreate=EFalse);
    TUid ImageFormat(TFileName &aFileName) const; 

private: 
    CGeneratorSettings(QRCEncoder* aEncoder);
    void ConstructL(); 
    void CreateStoreL();
    TBool RestoreL();

public:
    QRCEncoder* iEncoder;
    TInt iScale;
    TInt iBorder;
    TInt iImageFormatIndex;
    TUint32 iColors[2];
    CFileStore *iFileStore;
};



class CQRCEncoderModel : public CActive
{
    enum TState 
    {
	EIdle = 0, 
	EEncodingText,
	EDrawingImage,
	EEncodingImage,
	EError
    };

public:
	CQRCEncoderModel(TSize aSize);
	~CQRCEncoderModel();
	void RedrawQRCImageL(TSize aSize);
	CFbsBitmap* Bitmap();
	CFbsBitmap* FullBitmap();
	void EncodeL(const TDesC8 &aText);
	void EncodeL(const TDesC16 &aText);
	void SaveImageL(TFileName &aFilePath);
	void SendImageL();
	TBool ImageReady();
	TBool Finished();
	void CreateSettingsL();
	CGeneratorSettings* Settings();

private:

	void DrawQRCImageL(const TInt &aSize);

	void ShowInfoMessageL(TInt aResourceId=0, TInt aAlignment=-1, TInt aXpos=0, TInt aYPos=0, TInt aShowTime=0);

	// from CActive
	void RunL();
	void DoCancel();

private:
	CBitmapDrawer *iBitmapDrawer;
	CBitmapDrawer *iCachedBitmapDrawer;
	CFbsBitmap *iQRCBitmap;
	CImageEncoder *iImageEncoder;
	QRCEncoder iEncoder;
	HBufC8 *iUtf8Text;
	CGeneratorSettings *iSettings;
	CAknWaitDialog *iWaitDialog;
	CAknInfoPopupNoteController *iInfoPopup;
	TState iState;

};



class CDecoderModel: public CActive, NOCRUtils::MImageHandlerCallback
{
    enum TState 
    {
	EIdle = 0, 
	ELoadingImage,
	EImageLoaded,
	ERedrawImage,
	EDecodingImage,
	EImageDecoded,
	EExtractingCode,
	ECodeExtracted,
	EError
    };

public:
	CDecoderModel();

	~CDecoderModel();

	CViewContainer* SetupViewL(const TRect &aRect);
	
	CViewContainer* GetView();

	void RedrawCodeImageL();

	TBool DecodeBitmap(CFbsBitmap* aBitmap);

	TBool DecodeBitmapData(CFbsBitmap* aBitmap);

	void SetBitmap(CFbsBitmap* aBitmap);
	
	void StartDecodeL(CFbsBitmap* aBitmap=NULL);
	
	void StartDecodeL(const TFileName &aFileName);

	void ShowError();

	TBool DataAvailable();

	void ShowDataMenuL(TBool aNewData=ETrue);
	
	void SendPayloadL();

private:

	void DrawCodeImageL();

	void DecodeFinishedL(const TDesC *aFileName=NULL);

	void ViewPayloadL(TDesC &aPayload);

	void DisplayQRInfoL();

	void StepCompleted(TState aState, TInt err=KErrNone);


	// from CActive
	void RunL();

	void DoCancel();

private: // callbacks

	/* from MImageHandlerCallback */

        void ImageOperationCompleteL(TInt aError);
private:
	QRBARDecoder iDecoder;
	DecoderResult iDecoderResult;
	CViewContainer *iViewContainer;
	CEikImage *iImageView;
	//CTextEdit /*CEikLabel*/ *iInfoView;
	NOCRUtils::CImageHandler *iImageHandler;
	CFbsBitmap *iImageBitmap; 
	CAknWaitDialog* iWaitDialog;
	TState iState;
	TBool iDataAvailable;
};


class ScanHelper
{

public:
    ScanHelper();
    ~ScanHelper();
    TInt AllocateBuffer(const TInt aLen);
    TInt FindCode(CFbsBitmap* aBitmap);
    const TUint16* GetError();
//private:

    //void DrawQRC(CFbsBitmap* aBitmap, QRC* aQRC);

public:
    static void BGRBitmapToGray(CFbsBitmap* aBitmap, TUint8* &aBuffer);

public:
    CFbsBitGc* iGc;

private:
    QRBARDecoder iDecoder;
    TUint8* iBuffer; // gray data
    TInt iBufferSize;
};

#endif // __SQUIRRELMODEL_h__
