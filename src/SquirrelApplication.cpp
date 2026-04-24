/*
 ============================================================================
 Name		: SquirrelApplication.cpp
 Author	  : JigokuMaster
 Copyright   : Your copyright notice
 Description : Main application class
 ============================================================================
 */

// INCLUDE FILES
#include <eikstart.h>
#include <bautils.h>
#include <s32file.h>
#include <coemain.h>

#include "Squirrel.hrh"
#include "SquirrelDocument.h"
#include "SquirrelApplication.h"
#include "SquirrelModel.h"

_LIT(KLangSetFileName, "Language.dat");
_LIT(KResFileNameFormat0, "%S:\\Resource\\Apps\\Squirrel.r0%d");
_LIT(KResFileNameFormat, "%S:\\Resource\\Apps\\Squirrel.r%d");


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CSquirrelApplication::CreateDocumentL()
// Creates CApaDocument object
// -----------------------------------------------------------------------------
//
CApaDocument* CSquirrelApplication::CreateDocumentL()
	{
	// Create an Squirrel document, and return a pointer to it
	return CSquirrelDocument::NewL(*this);
	}

// -----------------------------------------------------------------------------
// CSquirrelApplication::AppDllUid()
// Returns application UID
// -----------------------------------------------------------------------------
//
TUid CSquirrelApplication::AppDllUid() const
	{
	// Return the UID for the Squirrel application
	return KUidSquirrelApp;
	}


TFileName CSquirrelApplication::ResourceFileName() const
{
    TFileName res;
    TLanguage systemLang = User::Language();
    iCurrentLanguage = systemLang;
    TBool ok = ReadLanguageL(res);
    if (ok && BaflUtils::FileExists(CCoeEnv::Static()->FsSession(), res)) return res;
    else iCurrentLanguage = User::Language();
    return CAknApplication::ResourceFileName(); 
}


TBool CSquirrelApplication::ReadLanguageL(TFileName &aFileName) const
{
    TFileName storePath;
    RFs& fs = iCoeEnv->FsSession();
    GetPrivateFilePathL(fs, storePath, KLangSetFileName);
    if (BaflUtils::FileExists(fs, storePath))
    {

	RDictionaryReadStream in;

        CDictionaryFileStore* settingsStore = CDictionaryFileStore::OpenLC(fs, storePath, KUidSquirrelApp);
        in.OpenLC(*settingsStore, TUid::Uid(0x1));
        
	TRAPD(err, iCurrentLanguage = (TLanguage)in.ReadUint32L());
        CleanupStack::PopAndDestroy(2); // in, settingsStore 
	if (err == KErrNone)
	{
	    TPtrC driveChar = AppFullName().Left(1);
	    if (iCurrentLanguage > 9) aFileName.Format(KResFileNameFormat, &driveChar, iCurrentLanguage);
	    else aFileName.Format(KResFileNameFormat0, &driveChar, iCurrentLanguage);
	    return ETrue;
	}
    }
    
    return EFalse;
}

TBool CSquirrelApplication::SaveLanguageL(const TLanguage aLanguage)
{
    if (iCurrentLanguage == aLanguage) return EFalse;

    RFs& fs = iCoeEnv->FsSession();
    TFileName res;

    TPtrC driveChar = AppFullName().Left(1);
    if (aLanguage > 9) res.Format(KResFileNameFormat, &driveChar, aLanguage);
    else res.Format(KResFileNameFormat0, &driveChar, aLanguage);

    if (!BaflUtils::FileExists(CCoeEnv::Static()->FsSession(), res) && aLanguage!= ELangEnglish) return EFalse;

    TFileName storePath;
    GetPrivateFilePathL(fs, storePath, KLangSetFileName);

    // delete existing store to make sure that it is clean and not eg corrupted
    if (BaflUtils::FileExists(fs, storePath))
    {
	fs.Delete(storePath);
    }

    RDictionaryWriteStream out;

    CDictionaryFileStore* settingsStore = CDictionaryFileStore::OpenLC(fs, storePath, KUidSquirrelApp);
    out.AssignLC(*settingsStore, TUid::Uid(0x1));
    out.WriteUint32L(aLanguage);
    out.CommitL(); 	
    settingsStore->CommitL();
    CleanupStack::PopAndDestroy(2); // out, settingsStore 
    iCurrentLanguage = aLanguage;
    return ETrue;
}


// End of File
