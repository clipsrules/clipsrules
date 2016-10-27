   /*******************************************************/
   /*      "C" Language Integrated Production System      */
   /*                                                     */
   /*             CLIPS Version 6.40  10/01/16            */
   /*                                                     */
   /*                MULTIFIELD_TYPE HEADER FILE               */
   /*******************************************************/

/*************************************************************/
/* Purpose: Routines for creating and manipulating           */
/*   multifield values.                                      */
/*                                                           */
/* Principal Programmer(s):                                  */
/*      Gary D. Riley                                        */
/*                                                           */
/* Contributing Programmer(s):                               */
/*                                                           */
/* Revision History:                                         */
/*                                                           */
/*      6.24: Renamed BOOLEAN macro type to intBool.         */
/*                                                           */
/*            Corrected code to remove compiler warnings.    */
/*                                                           */
/*            Moved ImplodeMultifield from multifun.c.       */
/*                                                           */
/*      6.30: Changed integer type/precision.                */
/*                                                           */
/*            Changed garbage collection algorithm.          */
/*                                                           */
/*            Used DataObjectToString instead of             */
/*            ValueToString in implode$ to handle            */
/*            print representation of external addresses.    */
/*                                                           */
/*            Added const qualifiers to remove C++           */
/*            deprecation warnings.                          */
/*                                                           */
/*            Converted API macros to function calls.        */
/*                                                           */
/*            Fixed issue with StoreInMultifield when        */
/*            asserting void values in implied deftemplate   */
/*            facts.                                         */
/*                                                           */
/*      6.40: Refactored code to reduce header dependencies  */
/*            in sysdep.c.                                   */
/*                                                           */
/*            Removed LOCALE definition.                     */
/*                                                           */
/*            Pragma once and other inclusion changes.       */
/*                                                           */
/*            Added support for booleans with <stdbool.h>.   */
/*                                                           */
/*            Removed use of void pointers for specific      */
/*            data structures.                               */
/*                                                           */
/*            ALLOW_ENVIRONMENT_GLOBALS no longer supported. */
/*                                                           */
/*            UDF redesign.                                  */
/*                                                           */
/*************************************************************/

#ifndef _H_multifld

#pragma once

#define _H_multifld

struct field;
typedef struct multifield Multifield;

#include "evaluatn.h"
#include "object.h"

struct field
  {
   union
     {
      void *value;
      TypeHeader *header;
      CLIPSLexeme *lexemeValue;
      CLIPSFloat *floatValue;
      CLIPSInteger *integerValue;
      CLIPSVoid *voidValue;
      Fact *factValue;
      Instance *instanceValue;
      Multifield *multifieldValue;
      CLIPSExternalAddress *externalAddressValue;
     };
  };

struct multifield
  {
   TypeHeader th;
   unsigned busyCount;
   long length;
   struct multifield *next;
   struct field theFields[1];
  };

typedef struct field FIELD;
typedef struct field * FIELD_PTR;

   Multifield                    *CreateUnmanagedMultifield(Environment *,long);
   void                           ReturnMultifield(Environment *,struct multifield *);
   void                           MultifieldInstall(Environment *,struct multifield *);
   void                           MultifieldDeinstall(Environment *,struct multifield *);
   void                           CVMultifieldInstall(Environment *,struct multifield *);
   void                           CVMultifieldDeinstall(Environment *,struct multifield *);
   Multifield                    *StringToMultifield(Environment *,const char *);
   Multifield                    *EnvCreateMultifield(Environment *,long);
   void                           AddToMultifieldList(Environment *,struct multifield *);
   void                           FlushMultifields(Environment *);
   void                           DuplicateMultifield(Environment *,UDFValue *,UDFValue *);
   void                           PrintMultifield(Environment *,const char *,Multifield *,long,long,bool);
   bool                           MultifieldDOsEqual(UDFValue *,UDFValue *);
   void                           StoreInMultifield(Environment *,UDFValue *,EXPRESSION *,bool);
   Multifield                    *CopyMultifield(Environment *,struct multifield *);
   bool                           MultifieldsEqual(struct multifield *,struct multifield *);
   Multifield                    *DOToMultifield(Environment *,UDFValue *);
   unsigned long                  HashMultifield(struct multifield *,unsigned long);
   Multifield                    *GetMultifieldList(Environment *);
   CLIPSLexeme                   *ImplodeMultifield(Environment *,UDFValue *);
   void                           EphemerateMultifield(Environment *,struct multifield *);
   Multifield                    *EnvArrayToMultifield(Environment *,CLIPSValue *,long size);
   void                           NormalizeMultifield(Environment *,UDFValue *);
   void                           CLIPSToUDFValue(CLIPSValue *,UDFValue *);

#endif /* _H_multifld */



