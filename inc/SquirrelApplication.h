/*
 ============================================================================
 Name		: SquirrelApplication.h
 Author	  : JigokuMaster
 Copyright   : Your copyright notice
 Description : Declares main application class.
 ============================================================================
 */

#ifndef __SQUIRRELAPPLICATION_H__
#define __SQUIRRELAPPLICATION_H__

// INCLUDES
#include <aknapp.h>
#include "Squirrel.hrh"

// UID for the application;
// this should correspond to the uid defined in the mmp file
const TUid KUidSquirrelApp =
	{
	_UID3
	};

// CLASS DECLARATION

/**
 * CSquirrelApplication application class.
 * Provides factory to create concrete document object.
 * An instance of CSquirrelApplication is the application part of the
 * AVKON application framework for the Squirrel example application.
 */
class CSquirrelApplication : public CAknApplication
	{
public:
	// Functions from base classes

	TFileName ResourceFileName() const;
	
	/**
	 * From CApaApplication, AppDllUid.
	 * @return Application's UID (KUidSquirrelApp).
	 */
	TUid AppDllUid() const;

	TBool ReadLanguageL(TFileName &aFileName) const;

	TBool SaveLanguageL(const TLanguage aLanguage);

protected:
	// Functions from base classes

	/**
	 * From CApaApplication, CreateDocumentL.
	 * Creates CSquirrelDocument document object. The returned
	 * pointer in not owned by the CSquirrelApplication object.
	 * @return A pointer to the created document object.
	 */
	CApaDocument* CreateDocumentL();

public:
	// Data
	mutable TUint32 iCurrentLanguage;

	};

#endif // __SQUIRRELAPPLICATION_H__
// End of File
