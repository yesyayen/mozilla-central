/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
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
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is Netscape
 * Communications Corporation.  Portions created by Netscape are
 * Copyright (C) 1998 Netscape Communications Corporation. All
 * Rights Reserved.
 *
 * Contributor(s): 
 */

#ifndef _MIMETPFL_H_
#define _MIMETPFL_H_

#include "mimetext.h"

/* The MimeInlineTextPlainFlowed class implements the
   text/plain MIME content type for the special case of a supplied
   format=flowed. See
   ftp://ftp.ietf.org/internet-drafts/draft-gellens-format-06.txt for
   more information. 
 */

typedef struct MimeInlineTextPlainFlowedClass MimeInlineTextPlainFlowedClass;
typedef struct MimeInlineTextPlainFlowed      MimeInlineTextPlainFlowed;

struct MimeInlineTextPlainFlowedClass {
  MimeInlineTextClass text;
};

extern MimeInlineTextPlainFlowedClass mimeInlineTextPlainFlowedClass;

struct MimeInlineTextPlainFlowed {
  MimeInlineText  text;
  PRInt32         mQuotedSizeSetting;   // mail.quoted_size
  PRInt32         mQuotedStyleSetting;  // mail.quoted_style
  char            *mCitationColor;      // mail.citation_color
};

void Update_in_tag_info(PRBool *a_in_tag, /* IN/OUT */
                   PRBool *a_in_quote_in_tag, /* IN/OUT */
                   char *a_quote_char, /* IN/OUT (pointer to single char) */
                   char a_current_char); /* IN */


/*
 * Made to contain information to be kept during the whole message parsing.
 */
struct MimeInlineTextPlainFlowedExData {
  struct MimeObject *ownerobj; /* The owner of this struct */
  PRBool inflow; /* If we currently are in flow */
  PRBool fixedwidthfont; /* If we output text for fixed width font */
  PRUint32 quotelevel; /* How deep is your love, uhr, quotelevel I meen. */
  PRBool isSig;  // we're currently in a signature
  struct MimeInlineTextPlainFlowedExData *next;
};

#endif /* _MIMETPFL_H_ */
