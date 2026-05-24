// INCLUDE FILES
#include <eikbtgpc.h>
#include <avkon.hrh>
#include <aknpopupfieldtext.h>
#include <eikmfne.h>
#include <eikedwin.h>
#include <badesca.h>
#include <eikcapc.h>
#include <aknselectionlist.h> // CAknSelectionListDialog
#include <cpbkcontactengine.h>
#include <cpbkviewstate.h>
#include <cpbkcontacteditordlg.h>
#include <cpbkcontactitem.h>
#include <vcard.h>
#include <versit.h>
#include "SquirrelDlgs.h"
#include "Squirrel.rsg"
#include "Squirrel.hrh"
#include "SquirrelModel.h"

///
/// Implementation of CGeneratorSettingsDlg
///

CGeneratorSettingsDlg* CGeneratorSettingsDlg::NewL(CGeneratorSettings *aSettings)
{
    CGeneratorSettingsDlg* self = new(ELeave) CGeneratorSettingsDlg;
    CleanupStack::PushL(self);
    self->ConstructL(aSettings);
    CleanupStack::Pop(self);
    return self;    
}


CGeneratorSettingsDlg::CGeneratorSettingsDlg()
{
}


CGeneratorSettingsDlg::~CGeneratorSettingsDlg()
{
}

void CGeneratorSettingsDlg::ConstructL(CGeneratorSettings *aSettings)
{
    CAknForm::ConstructL();
    iSettings = aSettings;
}

void CGeneratorSettingsDlg::PreLayoutDynInitL()
{
    CAknForm::PreLayoutDynInitL();

    SetIntValue(EGeneratorSettingsItem1, iSettings->iEncoder->MinVersion());

    SetIntValue(EGeneratorSettingsItem2, iSettings->iEncoder->MaxVersion());


    CAknPopupFieldText* eclComboBox = static_cast<CAknPopupFieldText*>(Control(EGeneratorSettingsItem3));
    eclComboBox->SetCurrentValueIndex(iSettings->iEncoder->Ecl());

    SetIntValue(EGeneratorSettingsItem4, iSettings->iScale);

    SetIntValue(EGeneratorSettingsItem5, iSettings->iBorder);

    CAknPopupFieldText* formatComboBox = static_cast<CAknPopupFieldText*>(Control(EGeneratorSettingsItem6));
    formatComboBox->SetCurrentValueIndex(iSettings->iImageFormatIndex);

    SetColorValue(EGeneratorSettingsItem7, iSettings->iColors[0]);

    SetColorValue(EGeneratorSettingsItem8, iSettings->iColors[1]);

}

TBool CGeneratorSettingsDlg::OkToExitL(TInt aButtonId)
{
    if (aButtonId == EAknSoftkeyOk) UpdateSettingsL(); 
    return ETrue;
}       

TBool CGeneratorSettingsDlg::ShowLD()
{
    SetAppTitleL(NULL, R_SETTINGS_TITLE);
    return ExecuteLD(R_GENERATOR_SETTINGS_FORM_DIALOG);
}

void CGeneratorSettingsDlg::UpdateSettingsL()
{
  
    TInt minVersion = GetIntValue(EGeneratorSettingsItem1);

    TInt maxVersion = GetIntValue(EGeneratorSettingsItem2);


    CAknPopupFieldText* eclComboBox = static_cast<CAknPopupFieldText*>(Control(EGeneratorSettingsItem3));

    TInt ecl = eclComboBox->CurrentValueIndex();

    iSettings->iEncoder->SetParams(ecl, minVersion, maxVersion); 

    iSettings->iScale = GetIntValue(EGeneratorSettingsItem4);

    iSettings->iBorder = GetIntValue(EGeneratorSettingsItem5);

    CAknPopupFieldText* formatComboBox = static_cast<CAknPopupFieldText*>(Control(EGeneratorSettingsItem6));
    
    iSettings->iImageFormatIndex  = formatComboBox->CurrentValueIndex();

    GetColorValue(EGeneratorSettingsItem7, iSettings->iColors[0]);    
    GetColorValue(EGeneratorSettingsItem8, iSettings->iColors[1]);  

    iSettings->StoreL();
}

void CGeneratorSettingsDlg::SetIntValue(TInt aId, TInt aValue)
{
    CEikNumberEditor* ed = static_cast<CEikNumberEditor*>(Control(aId));
    ed->SetNumber(aValue);
}


TInt CGeneratorSettingsDlg::GetIntValue(TInt aId)
{
    CEikNumberEditor* ed = static_cast<CEikNumberEditor*>(Control(aId));
    return ed->Number();
}


void CGeneratorSettingsDlg::GetColorValue(TInt aId, TUint32 &aValue)
{
    CEikEdwin* ed = static_cast<CEikEdwin*>(Control(aId));
    TInt len = ed->TextLength();

    if (len)
    {

	TPtrC hexStr = ed->Text()->Read(0, len);  
	TLex lex( (hexStr.Left(2) == _L("0x")) ? hexStr.Mid(2) : hexStr);
	lex.Val(aValue, EHex);
	//if (lex.Val(aValue, EHex) != KErrNone) aValue = 9;

    }
}

void CGeneratorSettingsDlg::SetColorValue(TInt aId, TUint32 &aValue)
{
    // Uint32 to hex string.
    TBuf<8> hexValue;
    hexValue.Format(_L("0x%X"), aValue);
    static_cast<CEikEdwin*>(Control(aId))->SetTextL(&hexValue);

}

////
/// Implementation of CContactSelectionDlg
///

CContactSelectionDlg* CContactSelectionDlg::NewL()
{
    CContactSelectionDlg* self = new (ELeave) CContactSelectionDlg;
    CleanupStack::PushL(self);
    return self;    
}

CContactSelectionDlg::CContactSelectionDlg()
{
    iContactDb = NULL;
    iContactArray = NULL;
    iVCardsBuf = NULL;
}

CContactSelectionDlg::~CContactSelectionDlg()
{
    if ( iContactDb )
    {
        iContactDb->CloseTables();
        delete iContactDb;
    }

    if ( iContactArray ) delete iContactArray;
    if (iVCardsBuf) delete iVCardsBuf;
}

void CContactSelectionDlg::OpenContactDatabaseL()
{
    TFileName fp;
    CContactDatabase::GetDefaultNameL(fp);
    iContactDb = CContactDatabase::OpenL();
}

// Copied from s60cppexamples/Contacts

void CContactSelectionDlg::ReadContactsL()
{
    iContactDb->SetDbViewContactType( KUidContactCard );
    TFieldType aFieldType1(KUidContactFieldFamilyName);
    TFieldType aFieldType2(KUidContactFieldGivenName);
    CContactDatabase::TSortPref sortPref1(aFieldType1);
    CContactDatabase::TSortPref sortPref2(aFieldType2);

    // Sort contacts by Family and Given Name
    CArrayFixFlat<CContactDatabase::TSortPref>* aSortOrder = new (ELeave) CArrayFixFlat<CContactDatabase::TSortPref>(2);

    CleanupStack::PushL(aSortOrder);
    aSortOrder->AppendL(sortPref1);
    aSortOrder->AppendL(sortPref2);

    // The database takes ownership of the sort order array passed in
    iContactDb->SortL(aSortOrder);

    // The caller does not take ownership of this object.
    // so do not push it onto the CleanupStack
    const CContactIdArray* contacts = iContactDb->SortedItemsL();

    // Go through each contact item and
    // make items for listbox

    const TInt KFormattersSpace = 10;
    _LIT(KListItemFormatter,"\t%S %S\t\t");
    _LIT(KListOItemFormatter,"\t%S\t\t");

    const TInt nc(contacts->Count());
    
    for ( TInt i( nc-1 ); i >= 0; i-- ) //For each ContactId
    {
        CContactItem* contact = NULL;
        // The caller takes ownership of the returned object.
        // So push it onto the CleanupStack
        contact = iContactDb->OpenContactL( (*contacts)[i]);

        CleanupStack::PushL(contact);

        CContactItemFieldSet& fieldSet = contact->CardFields();

        HBufC* firstNameBuf = NULL;
        HBufC* lastNameBuf = NULL;

        // Get first name
        TInt findpos(fieldSet.Find(KUidContactFieldGivenName));

        // Check that the first name field is actually there.
        if ( (findpos > -1) || (findpos >= fieldSet.Count()) )
	{
            CContactItemField& firstNameField = fieldSet[findpos];
            CContactTextField* firstName = firstNameField.TextStorage();
            firstNameBuf = firstName->Text().AllocLC();
        }

	    // Get last name
        findpos = fieldSet.Find( KUidContactFieldFamilyName );

        // Check that the last name field is actually there.
        if ( (findpos > -1) || (findpos >= fieldSet.Count()) )
        {
            CContactItemField& lastNameField = fieldSet[ findpos ];
            CContactTextField* lastName = lastNameField.TextStorage();
            lastNameBuf = lastName->Text().AllocLC();
        }

        TInt len(0);

        if (firstNameBuf) len += firstNameBuf->Length();
        if (lastNameBuf) len += lastNameBuf->Length();

	// Create new buffer and add space for formatters
	HBufC* combinedDes = HBufC::NewLC(len + KFormattersSpace);

        if ( ( firstNameBuf ) && ( lastNameBuf ) )

	{
	    combinedDes->Des().Format( KListItemFormatter, lastNameBuf, firstNameBuf );
        }
        else
	{
            if ( firstNameBuf )
            {
                combinedDes->Des().Format( KListOItemFormatter, firstNameBuf );
            }
            else if ( lastNameBuf )
            {
                combinedDes->Des().Format( KListOItemFormatter, lastNameBuf );
            }
        }

        // Insert a contacts name into the array at the specified position.
        // If the specified position is the same as the current number of
        // descriptor elements in the array, this has the effect of
        // appending the element.
	if (len == 0)  iContactArray->InsertL( 0, _L("\t(unnamed)\t\t"));

	else iContactArray->InsertL( 0, *combinedDes );

        CleanupStack::PopAndDestroy(combinedDes);

        if (lastNameBuf) {CleanupStack::PopAndDestroy(lastNameBuf); }


        if (firstNameBuf){ CleanupStack::PopAndDestroy(firstNameBuf); }

        iContactDb->CloseContactL(contact->Id());
        CleanupStack::PopAndDestroy(contact);

    }

    CleanupStack::Pop(aSortOrder);
}

void CContactSelectionDlg::DiscardPhotoFieldL(CBufFlat* aVCardBuf)
{

    CBufFlat* buf = CBufFlat::NewL(4);
    CleanupStack::PushL(buf);
    buf->InsertL(0, aVCardBuf->Ptr(0));
   
    RBufReadStream stream(*buf);
    CleanupClosePushL(stream);	
    CParserVCard* parser = CParserVCard::NewL();
    CleanupStack::PushL(parser);
    parser->InternalizeL(stream);
    CArrayPtr<CParserProperty>* props = parser->ArrayOfProperties(EFalse);

    for (TInt i=0; i < props->Count(); i++)
    {
	CParserProperty* p = (*props)[i];
	if (p && (p->Name() == KVersitTokenPHOTO))
	{
	    props->Delete(i);
	    aVCardBuf->Reset();
	    RBufWriteStream stream2(*aVCardBuf);
	    CleanupClosePushL(stream2);	
	    parser->ExternalizeL(stream2);
	    CleanupStack::PopAndDestroy(&stream2);
	    break;
	}
    }

    CleanupStack::PopAndDestroy(buf);
    CleanupStack::PopAndDestroy(&stream);
    CleanupStack::PopAndDestroy(parser);
    
}

TPtr8 CContactSelectionDlg::GetVCardL(const TInt aIndex)
{

    const CContactIdArray* contacts = iContactDb->SortedItemsL();

    CContactIdArray* idArray = CContactIdArray::NewL();
    CleanupStack::PushL(idArray);
  
    // find photo field
    CContactItem* contact = iContactDb->OpenContactL((*contacts)[aIndex]);
    CleanupStack::PushL(contact);
    CContactItemFieldSet& fieldSet = contact->CardFields();
    TInt findpos(fieldSet.Find(KUidContactFieldPicture));
    TBool hasPhoto = (findpos > -1) || (findpos >= fieldSet.Count());
    CleanupStack::PopAndDestroy(contact);


    idArray->AddL((*contacts)[aIndex]);
	
    if (!iVCardsBuf) iVCardsBuf = CBufFlat::NewL(4);
    
    else iVCardsBuf->Reset();
    

    RBufWriteStream outputStream(*iVCardsBuf);
    CleanupClosePushL(outputStream);
  

    TInt opts = /*CContactDatabase::EIncludeX |*/CContactDatabase::ETTFormat;
    TUid formatUid = TUid::Uid(KVersitEntityUidVCard);  
    iContactDb->ExportSelectedContactsL(formatUid, *idArray, outputStream, opts);  
    
    outputStream.CommitL();        
    
    CleanupStack::PopAndDestroy(&outputStream); 
    
    CleanupStack::PopAndDestroy(idArray);

    // discard photo field
    if ( hasPhoto) DiscardPhotoFieldL(iVCardsBuf);

    // cleanup extra junks and make sure the data is NULL-terminated.
    _LIT8(KVCEnd, "END:VCARD");
    TPtr8 p = iVCardsBuf->Ptr(0);
    TInt len = p.Length();
    TInt pos = p.FindF(KVCEnd);
    TInt endPos = pos + 10;
    if (pos != KErrNotFound && len >= endPos) endPos--;
    else endPos = --len;
    p[endPos] = '\0';
    return p;
}


TInt CContactSelectionDlg::ShowL()
{

    TInt selectedItem = 0;
    CAknSelectionListDialog *dlg =  NULL;

    iContactArray = new (ELeave) CDesCArrayFlat(4);
 
    if (CContactDatabase::DefaultContactDatabaseExistsL())
    {
	OpenContactDatabaseL();
	ReadContactsL();
    }
   
    dlg = CAknSelectionListDialog::NewL(selectedItem, iContactArray, R_AVKON_DIALOG_EMPTY_MENUBAR);

    TInt ok = dlg->ExecuteLD(R_CONTACT_SELECTION_DIALOG);
    
    return (ok ? selectedItem : -1);

}

// End of File

