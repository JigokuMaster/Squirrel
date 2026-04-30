/*
 ============================================================================
 Name		: SquirrelAppView.h
 Author	  : JigokuMaster
 Copyright   : Your copyright notice
 Description : Declares view class for application.
 ============================================================================
 */

#ifndef __SQUIRRELABOUTVIEW_h__
#define __SQUIRRELABOUTVIEW_h__

// INCLUDES
#include <aknview.h>
#include <eiklbo.h>
#include <brctlinterface.h>
#include "uicommons.h"


// CLASS DECLARATION

/**
 *  CWebView
 * 
 */
class CWebView : public CCoeControl
	{
public:
	// Constructors and destructor

	/**
	 * Destructor.
	 */
	~CWebView();

	/**
	 * Two-phased constructor.
	 */
	static CWebView* NewL(const TRect& aRect);

	/**
	 * Two-phased constructor.
	 */
	static CWebView* NewLC(const TRect& aRect);
	
private:

	/**
	 * Constructor for performing 1st stage construction
	 */
	CWebView();

	/**
	 * EPOC default constructor for performing 2nd stage construction
	 */
	void ConstructL(const TRect& aRect);

	virtual void SizeChanged();

        virtual TInt CountComponentControls() const; 

        virtual CCoeControl* ComponentControl(TInt aIndex) const; 


	void HandlePointerEventL(const TPointerEvent& aPointerEvent);

public:
        TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType);

	void LoadPageL();

public: //data 
        // Pointer to the browser control interface
        CBrCtlInterface* iBrCtlInterface;
};

class CSquirrelAboutView : public CAknView
{
public:
	// New methods
	static CSquirrelAboutView* NewL();
	static CSquirrelAboutView* NewLC();

	/**
	 * ~CSquirrelAboutView
	 * Virtual Destructor.
	 */
	virtual ~CSquirrelAboutView();

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

private:
	// Constructors
	void ConstructL();

	/**
	 * CSquirrelAboutView.
	 * C++ default constructor.
	 */
	CSquirrelAboutView();


private:
	CWebView* iContainer;
};

#endif // __SQUIRRELABOUTVIEW_h__
// End of File
