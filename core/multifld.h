   /*******************************************************/
   /*      "C" Language Integrated Production System      */
   /*                                                     */
   /*             CLIPS Version 6.40  08/25/16            */
   /*                                                     */
   /*                MULTIFIELD HEADER FILE               */
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
     };
  };

struct multifield
  {
   TypeHeader th;
   unsigned busyCount;
   long multifieldLength;
   struct multifield *next;
   struct field theFields[1];
  };

typedef struct field FIELD;
typedef struct field * FIELD_PTR;

#define GetMFLength(target)     (((Multifield *) (target))->multifieldLength)
#define SetMFValue(target,index,val)  (((struct field *) ((Multifield *) (target))->theFields)[index].value = (void *) (val))

#define GetMFType(target,index)  ((((struct field *) ((Multifield *) (target))->theFields)[index].header)->type)
#define GetMFValue(target,index)  (((struct field *) ((Multifield *) (target))->theFields)[index].value)

   Multifield                    *CreateMultifield2(Environment *,long);
   void                           ReturnMultifield(Environment *,struct multifield *);
   void                           MultifieldInstall(Environment *,struct multifield *);
   void                           MultifieldDeinstall(Environment *,struct multifield *);
   void                           CVMultifieldInstall(Environment *,struct multifield *);
   void                           CVMultifieldDeinstall(Environment *,struct multifield *);
   Multifield                    *StringToMultifield(Environment *,const char *);
   Multifield                    *EnvCreateMultifield(Environment *,long);
   void                           AddToMultifieldList(Environment *,struct multifield *);
   void                           FlushMultifields(Environment *);
   void                           DuplicateMultifield(Environment *,CLIPSValue *,CLIPSValue *);
   void                           PrintMultifield(Environment *,const char *,Multifield *,long,long,bool);
   bool                           MultifieldDOsEqual(CLIPSValue *,CLIPSValue *);
   void                           StoreInMultifield(Environment *,CLIPSValue *,EXPRESSION *,bool);
   Multifield                    *CopyMultifield(Environment *,struct multifield *);
   bool                           MultifieldsEqual(struct multifield *,struct multifield *);
   Multifield                    *DOToMultifield(Environment *,CLIPSValue *);
   unsigned long                  HashMultifield(struct multifield *,unsigned long);
   Multifield                    *GetMultifieldList(Environment *);
   CLIPSLexeme                   *ImplodeMultifield(Environment *,CLIPSValue *);
   void                           EphemerateMultifield(Environment *,struct multifield *);

#endif /* _H_multifld */




