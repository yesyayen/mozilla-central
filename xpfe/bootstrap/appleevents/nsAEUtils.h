/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
 * The contents of this file are subject to the Netscape Public
 * License Version 1.1 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at http://www.mozilla.org/NPL/
 *
 * Software distributed under the License is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 *
 * The Original Code is Mozilla Communicator client code.
 *
 * The Initial Developer of the Original Code is Netscape Communications
 * Corporation.  Portions created by Netscape are
 * Copyright (C) 1998 Netscape Communications Corporation. All
 * Rights Reserved.
 *
 * Contributor(s): 
 *  Simon Fraser <sfraser@netscape.com>
 */



#ifndef __AEUTILS__
#define __AEUTILS__

#include <MacTypes.h>
#include <AppleEvents.h>
#include <AERegistry.h>
#include <ASRegistry.h>
#include <AEObjects.h>
#include <FinderRegistry.h>

#include <Files.h>
#include <Errors.h>
#include <Aliases.h>


#include "nsAEDefs.h"
#include "nsMacUtils.h"


#define ThrowIfNil(a)      		{ if (a == nil) { throw((OSErr)memFullErr); } }
#define ThrowErrIfNil(a, err)	{ if (a == nil) { OSErr theErr = (err); throw((OSErr)theErr); } }
#define ThrowIfOSErr(err)       { OSErr theErr = (err); if (theErr != noErr) { throw((OSErr)theErr); } }
#define ThrowOSErr(err)         { throw((OSErr)err); }
#define ThrowNoErr()		{ throw((OSErr)noErr); }


#define pObjectType				'pObT'

// Windows
#define pTitle					'pTit'
#define pIsModeless				'pNMo'
#define pIsMovableModal      		'pMMo'
#define pIsSuspended			'pSus'
#define pIsPalette				'pPal'
#define pIsDialog				'pDlg'
#define pLocation				'pLcn'   // the upper left corner of the object's bounding box

// Application 

#define pFreeMemory			'pMem'
#define pTicks					'pTic'



// data given to a spawned thread for suspending and resuming Apple Events.
// there should be one of these per event.
typedef struct TThreadAEInfo
{
	AppleEvent		mAppleEvent;
	AppleEvent		mReply;
	SInt32			mSuspendCount;			// how many suspend calls we have
	Boolean			mGotEventData;
} TThreadAEInfo, *TThreadAEInfoPtr;




#ifdef __cplusplus
extern "C" {
#endif

OSErr CreateAliasAEDesc(AliasHandle theAlias, AEDesc *ioDesc);
OSErr GetTextFromAEDesc(const AEDesc *inDesc, Handle *outTextHandle);

OSErr CreateThreadAEInfo(const AppleEvent *event, AppleEvent *reply, TThreadAEInfoPtr *outThreadAEInfo);
void DisposeThreadAEInfo(TThreadAEInfo *threadAEInfo);

OSErr SuspendThreadAE(TThreadAEInfo *threadAEInfo);
OSErr ResumeThreadAE(TThreadAEInfo *threadAEInfo, OSErr threadError);

#if !TARGET_CARBON

OSErr AEGetDescData(const AEDesc *theAEDesc,  DescType typeCode,  void * dataPtr, Size maximumSize);
Size AEGetDescDataSize(const AEDesc *theAEDesc);

#endif /* TARGET_CARBON */

#ifdef __cplusplus
}
#endif


// utility port setting class

class StPortSetter
{
public:
				StPortSetter(WindowPtr destWindowPort)
				{
					::GetPort(&mOldPort);
#if TARGET_CARBON
					::SetPortWindowPort(destWindowPort);
#else
					::SetPort(destWindowPort);
#endif
				}
				
				~StPortSetter()
				{
					::SetPort(mOldPort);
				}
				
protected:
	GrafPtr		mOldPort;
};


// a stack-based, exception safely class for an AEDesc

class StAEDesc: public AEDesc
{
public:
					StAEDesc()
					{
						descriptorType = typeNull;
						dataHandle = nil;
					}
					
					StAEDesc(const StAEDesc& rhs);				// copy constructor
					
					~StAEDesc()
					{
						::AEDisposeDesc(this);
					}

					void	Clear()
					{
						::AEDisposeDesc(this);
						descriptorType = typeNull;
						dataHandle = nil;
					}
					
					void CheckDataType(DescType expectedType)
					{
						if (descriptorType != expectedType)
							ThrowOSErr(errAEWrongDataType);
					}
					
	StAEDesc& 		operator= (const StAEDesc&rhs);		// throws OSErrs

	Size				GetDataSize() { return (dataHandle) ? GetHandleSize(dataHandle) : 0; }
		
	Boolean			GetBoolean();
	SInt16			GetShort();
	SInt32			GetLong();
	DescType			GetEnumType();
	
	void				GetRect(Rect& outRect);
	void				GetRGBColor(RGBColor& outColor);
	void				GetLongDateTime(LongDateTime& outDateTime);
	void				GetFileSpec(FSSpec &outFileSpec);
	void				GetCString(char *outString, short maxLen);
	void				GetPString(Str255 outString);
	
	Handle			GetTextHandle();
	
};


class StEventSuspender
{
public:

					// use deleteData = false when calling threads, which take ownership of the TThreadAEInfoPtr
					StEventSuspender(const AppleEvent *appleEvent, AppleEvent *reply, Boolean deleteData = true);
					~StEventSuspender();

	void				SetDeleteData(Boolean deleteData)	{ mDeleteData = deleteData; }
	void				SuspendEvent();
	void				ResumeEvent();
	
	TThreadAEInfoPtr	GetThreadAEInfo()	{ return mThreadAEInfo; }
	
protected:

	TThreadAEInfoPtr	mThreadAEInfo;
	Boolean			mDeleteData;
};

class StHandleHolder
{
public:
					StHandleHolder(Handle inHandle = nil);		// takes ownership of the handle
					~StHandleHolder();
					
	StHandleHolder&	operator=(Handle rhs);
					
	void				Lock();
	void				Unlock();
	
	Handle			GetHandle()		{ return mHandle; }
	Size				GetHandleSize()		{ return (mHandle) ? ::GetHandleSize(mHandle) : 0; } 
	Ptr				GetPtr()			{ return (mHandle) ? *mHandle : nil; }
	
	class getter
	{
	public:
					getter(StHandleHolder& handleHolder)
					:	mHandleHolder(handleHolder)
					,	mHandle(nil)
					{
					}
					
					~getter()
					{
						mHandleHolder = mHandle;
					}
					
					operator Handle*()
					{
						return &mHandle;
					}
	private:
		StHandleHolder&	mHandleHolder;
		Handle			mHandle;
	};

	// it is risky to define operator&, but, hey, we know what we're doing.
	getter			operator &()
					{
						return getter(*this);
					}
	// Does the same as operator&, but more explicitly.
	getter			AssignHandle()
					{
						return getter(*this);
					}
	
protected:
	
	Handle			mHandle;
	SInt32			mLockCount;
	UInt8			mOldHandleState;
};


class StHandleLocker
{
public:
					StHandleLocker(Handle inHandle)
					:	mHandle(inHandle)
					{
						AE_ASSERT(mHandle, "No handle");
						mOldHandleState = HGetState(mHandle);
						HLock(mHandle);
					}
					
					~StHandleLocker()
					{
						HSetState(mHandle, mOldHandleState);
					}
protected:
	Handle			mHandle;
	UInt8			mOldHandleState;
};


class AEListUtils
{
public:

	static Boolean	TokenContainsTokenList(const AEDesc *token) { return (token->descriptorType == typeAEList); }
	static OSErr	GetFirstNonListToken(const AEDesc *token, AEDesc *result);
	static OSErr 	FlattenAEList(AEDescList *deepList, AEDescList *flatList);
};


// simple iterator for AEDescs which may be lists. Is NOT safe to the list changing under it.
// also, it does not handle nested lists.
class AEListIterator
{
public:
				AEListIterator(AEDesc *token);
				~AEListIterator() {}
				
	Boolean		Next(AEDesc* outItemData);			// data should be deleted by the caller.
	SInt32		GetNumItems()  { return mNumItems; }
	
protected:
	AEDesc		mListToken;		// we don't own this.
	SInt32		mNumItems;
	SInt32		mCurItem;
	Boolean		mIsListDesc;

};


OSErr	DescToPString(const AEDesc* desc, Str255 aPString, short maxLength);
OSErr 	DescToCString(const AEDesc* desc, CStr255 aPString, short maxLength);
OSErr	DescToDescType(const AEDesc *desc, DescType *descType);
OSErr	DescToBoolean(const AEDesc* desc, Boolean* aBoolean);
OSErr	DescToFixed(const  AEDesc* desc, Fixed* aFixed);
OSErr 	DescToFloat(const  AEDesc* desc, float* aFloat);
OSErr 	DescToLong(const  AEDesc* desc, long* aLong);
OSErr 	DescToRGBColor(const  AEDesc* desc, RGBColor* aRGBColor);
OSErr 	DescToShort(const  AEDesc* desc, short* aShort);
OSErr 	DescToTextHandle(const AEDesc* desc, Handle *text);
OSErr 	DescToRect(const  AEDesc* desc, Rect* aRect);
OSErr 	DescToPoint(const  AEDesc* desc, Point* aPoint);

OSErr	CheckForUnusedParameters(const AppleEvent* appleEvent);
OSErr	PutReplyErrorNumber(AppleEvent* reply, long errorNumber);
OSErr	PutReplyErrorMessage(AppleEvent* reply, char *message);
OSErr	GetObjectClassFromAppleEvent(const AppleEvent *appleEvent, DescType *objectClass);
OSErr	NormalizeAbsoluteIndex(const AEDesc *keyData, long *index, long maxIndex, Boolean *isAllItems);
OSErr 	ProcessFormRange(AEDesc *keyData, AEDesc *start, AEDesc *stop);


#endif /* __AEUTILS__ */
