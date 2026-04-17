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
#include <browserlauncher.h>
#include "uicommons.h"


// CLASS DECLARATION
class CSquirrelAboutView : public CAknView, MEikListBoxObserver
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

	virtual void HandleListBoxEventL(CEikListBox* aListBox, TListBoxEvent aEventType);
	
	void OpenLinkL();

private:
	// Constructors
	void ConstructL();

	/**
	 * CSquirrelAboutView.
	 * C++ default constructor.
	 */
	CSquirrelAboutView();

	static TInt OpenLink(TAny* aArg);

	void ShowOSNoticeL();

private:
	TListBox<DOUBLE_STYLE> *iListbox;
	CBrowserLauncher* iBrowserLauncher;
	TBuf<512> iLinkBuf;
	HBufC* iOSNoticeContent;
};

#endif // __SQUIRRELABOUTVIEW_h__
// End of File
