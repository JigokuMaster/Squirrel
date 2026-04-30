/*
 ============================================================================
 Name		: SquirrelGeneratorView.cpp
 Author	  : JigokuMaster
 Copyright   : Your copyright notice
 Description : Declares view class for application.
 ============================================================================
 */

#ifndef __SQUIRRELGENERATORVIEW_h__
#define __SQUIRRELGENERATORVIEW_h__

// INCLUDES
#include <aknview.h>
#include <eikimage.h>
#include <imageconversion.h>
#include <etel3rdparty.h>
#include <eikrted.h>

#include "uicommons.h"
#include "SquirrelModel.h"

// CLASS DECLARATION

class CTextEditWrapper: public CCoeControl
{
    
public:
    // constructors and destructor

    void ConstructL(const TRect& aRect, const CCoeControl* aParent);

    ~CTextEditWrapper();

public:
    // new functions	
    void SetSkinnedTextColorL();

    void SetTextL(const TDesC *aDes);

    HBufC*  GetTextL();

    TBool IsEmpty();

private:
    // functions from CCoeControl

    void SizeChanged();

    TInt CountComponentControls() const;

    CCoeControl* ComponentControl(TInt aIndex) const;

    //void Draw(const TRect& aRect) const;

    TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType);

private:
    CEikRichTextEditor* iRtEd; // Rich Text Editor
};


class CSquirrelGeneratorView : public CAknView
{
public:
	// New methods
	static CSquirrelGeneratorView* NewL();
	static CSquirrelGeneratorView* NewLC();

	/**
	 * ~CSquirrelGeneratorView
	 * Virtual Destructor.
	 */
	virtual ~CSquirrelGeneratorView();

public:
    
        /**
        * From CAknView.
        * Returns views id.
        *
        * @return Id for this view.
        */
        TUid Id() const;
    
        /**
        * From CAknView.
        * Handles commands
        *
        * @param aCommand A command to be handled.
        */
        void HandleCommandL( TInt aCommand );
 
	void HandleViewRectChange();

        /**
        * From CAknView.
        * Activates view
        *
        * @param aPrevViewId Specifies the view previously active.
        * @param aCustomMessageId Specifies the message type.
        * @param aCustomMessage The activation message.
        */
        void DoActivateL( const TVwsViewId& aPrevViewId,
                          TUid aCustomMessageId,
                          const TDesC8& aCustomMessage );
    
        /**
        * From CAknView.
        * Deactivates view.
        */
        void DoDeactivate();

	void DynInitMenuPaneL(TInt aResourceId, CEikMenuPane *aMenuPane);
	
	TRect ClientRect() const;

	void ToggleFullScreenMode();
private:
	// Constructors
	void ConstructL();

	/**
	 * CSquirrelGeneratorView.
	 * C++ default constructor.
	 */
	CSquirrelGeneratorView();

	void SetupControlsL();
	void LayoutControls();

	void DoGenerateL();
	void DoGenerateFromPhoneIMEIL(const TBool aRetry);
	void DoGenerateFromIPAddressL();
	void DoGenerateFromTemplateL(const TInt aCmd);
	void DoSaveImageL();

private:
	CViewContainer *iContainer;
	CEikImage *iImageView;
	CQRCEncoderModel *iEncoderModel;
	//CEikRichTextEditor *iTextEdit;
	CTextEditWrapper *iTextEdit;
	CTelephony* iTelephony;
	TBool iFullScreenMode;

};

#endif // __SQUIRRELGENERATORVIEW_h__
// End of File
