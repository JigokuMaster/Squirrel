/*
 ============================================================================
 Name		: SquirrelAppUi.h
 Author	  : JigokuMaster
 Copyright   : Your copyright notice
 Description : Declares UI class for application.
 ============================================================================
 */

#ifndef __SQUIRRELAPPUI_h__
#define __SQUIRRELAPPUI_h__

// INCLUDES
#include <aknviewappui.h>

// FORWARD DECLARATIONS
class CSquirrelAppView;
class CSquirrelDecoderView;
class CHistoryStore;
class CSquirrelHistoryView;
class CSquirrelScannerView;
class CSquirrelGeneratorView;
class CSquirrelAboutView;
class CSquirrelDummyView;

// CLASS DECLARATION
/**
 * CSquirrelAppUi application UI class.
 * Interacts with the user through the UI and request message processing
 * from the handler class
 */
class CSquirrelAppUi : public CAknViewAppUi
{
public:
	// Constructors and destructor

	/**
	 * ConstructL.
	 * 2nd phase constructor.
	 */
	void ConstructL();

	/**
	 * CSquirrelAppUi.
	 * C++ default constructor. This needs to be public due to
	 * the way the framework constructs the AppUi
	 */
	CSquirrelAppUi();

	/**
	 * ~CSquirrelAppUi.
	 * Virtual Destructor.
	 */
	virtual ~CSquirrelAppUi();

public:
	void ActivateDecoderViewL(CFbsBitmap* aBitmap);
	CHistoryStore* HistoryStore();


private:
	// Functions from base classes

	/**
	 * From CEikAppUi, HandleCommandL.
	 * Takes care of command handling.
	 * @param aCommand Command to be handled.
	 */
	void HandleCommandL(TInt aCommand);

	void HandleWsEventL(const TWsEvent& aEvent, CCoeControl* aDestination);

	void HandleResourceChangeL(TInt aType);

	/**
	 *  HandleStatusPaneSizeChange.
	 *  Called by the framework when the application status pane
	 *  size is changed.
	 */
	//void HandleStatusPaneSizeChange();

	/**
	 *  From CCoeAppUi, HelpContextL.
	 *  Provides help context for the application.
	 *  size is changed.
	 */
	//CArrayFix<TCoeHelpContext>* HelpContextL() const;

private:
	// New functions

	TBool IsViewActivated(CAknView* aView);

private:
	// Data

	CSquirrelAppView	*iMainView;
	CSquirrelDecoderView	*iDecoderView;
	CSquirrelHistoryView	*iHistoryView;
	CSquirrelScannerView	*iScannerView;
	CSquirrelGeneratorView	*iGeneratorView;
	CSquirrelAboutView	*iAboutView;
	CSquirrelDummyView	*iDummyView;

};

#endif // __SQUIRRELAPPUI_h__
// End of File
