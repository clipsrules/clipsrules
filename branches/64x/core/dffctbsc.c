   /*******************************************************/
   /*      "C" Language Integrated Production System      */
   /*                                                     */
   /*            CLIPS Version 6.40  08/25/16             */
   /*                                                     */
   /*         DEFFACTS BASIC COMMANDS HEADER FILE         */
   /*******************************************************/

/*************************************************************/
/* Purpose: Implements core commands for the deffacts        */
/*   construct such as clear, reset, save, undeffacts,       */
/*   ppdeffacts, list-deffacts, and get-deffacts-list.       */
/*                                                           */
/* Principal Programmer(s):                                  */
/*      Gary D. Riley                                        */
/*                                                           */
/* Contributing Programmer(s):                               */
/*      Brian L. Dantes                                      */
/*                                                           */
/* Revision History:                                         */
/*                                                           */
/*      6.23: Corrected compilation errors for files         */
/*            generated by constructs-to-c. DR0861           */
/*                                                           */
/*      6.24: Renamed BOOLEAN macro type to intBool.         */
/*                                                           */
/*      6.30: Removed conditional code for unsupported       */
/*            compilers/operating systems (IBM_MCW,          */
/*            MAC_MCW, and IBM_TBC).                         */
/*                                                           */
/*            Added const qualifiers to remove C++           */
/*            deprecation warnings.                          */
/*                                                           */
/*            Converted API macros to function calls.        */
/*                                                           */
/*            Changed find construct functionality so that   */
/*            imported modules are search when locating a    */
/*            named construct.                               */
/*                                                           */
/*      6.40: Added Env prefix to GetEvaluationError and     */
/*            SetEvaluationError functions.                  */
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

#include "setup.h"

#if DEFFACTS_CONSTRUCT

#include <stdio.h>
#include <string.h>

#include "argacces.h"
#include "constrct.h"
#include "cstrccom.h"
#include "cstrcpsr.h"
#include "dffctpsr.h"
#include "envrnmnt.h"
#include "extnfunc.h"
#include "factrhs.h"
#include "memalloc.h"
#include "router.h"
#include "scanner.h"
#include "tmpltdef.h"

#if BLOAD || BLOAD_ONLY || BLOAD_AND_BSAVE
#include "dffctbin.h"
#endif
#if CONSTRUCT_COMPILER && (! RUN_TIME)
#include "dffctcmp.h"
#endif

#include "dffctbsc.h"

/***************************************/
/* LOCAL INTERNAL FUNCTION DEFINITIONS */
/***************************************/

   static void                    ResetDeffacts(Environment *);
   static void                    ClearDeffacts(Environment *);
   static void                    SaveDeffacts(Environment *,Defmodule *,const char *);
   static void                    ResetDeffactsAction(Environment *,ConstructHeader *,void *);

/***************************************************************/
/* DeffactsBasicCommands: Initializes basic deffacts commands. */
/***************************************************************/
void DeffactsBasicCommands(
  Environment *theEnv)
  {
   EnvAddResetFunction(theEnv,"deffacts",ResetDeffacts,0);
   EnvAddClearFunction(theEnv,"deffacts",ClearDeffacts,0);
   AddSaveFunction(theEnv,"deffacts",SaveDeffacts,10);

#if ! RUN_TIME
   EnvAddUDF(theEnv,"get-deffacts-list","m",0,1,"y",GetDeffactsListFunction,"GetDeffactsListFunction",NULL);
   EnvAddUDF(theEnv,"undeffacts","v",1,1,"y",UndeffactsCommand,"UndeffactsCommand",NULL);
   EnvAddUDF(theEnv,"deffacts-module","y",1,1,"y",DeffactsModuleFunction,"DeffactsModuleFunction",NULL);

#if DEBUGGING_FUNCTIONS
   EnvAddUDF(theEnv,"list-deffacts","v",0,1,"y",ListDeffactsCommand,"ListDeffactsCommand",NULL);
   EnvAddUDF(theEnv,"ppdeffacts","v",1,1,"y",PPDeffactsCommand,"PPDeffactsCommand",NULL);
#endif

#if (BLOAD || BLOAD_ONLY || BLOAD_AND_BSAVE)
   DeffactsBinarySetup(theEnv);
#endif

#if CONSTRUCT_COMPILER && (! RUN_TIME)
   DeffactsCompilerSetup(theEnv);
#endif

#endif
  }

/**********************************************************/
/* ResetDeffacts: Deffacts reset routine for use with the */
/*   reset command. Asserts all of the facts contained in */
/*   deffacts constructs.                                 */
/**********************************************************/
static void ResetDeffacts(
  Environment *theEnv)
  {
   DoForAllConstructs(theEnv,
                      ResetDeffactsAction,
                      DeffactsData(theEnv)->DeffactsModuleIndex,
                      true,NULL);
  }

/*****************************************************/
/* ResetDeffactsAction: Action to be applied to each */
/*   deffacts construct during a reset command.      */
/*****************************************************/
static void ResetDeffactsAction(
  Environment *theEnv,
  ConstructHeader *theConstruct,
  void *buffer)
  {
#if MAC_XCD
#pragma unused(buffer)
#endif
   UDFValue returnValue;
   Deffacts *theDeffacts = (Deffacts *) theConstruct;

   if (theDeffacts->assertList == NULL) return;

   EnvSetEvaluationError(theEnv,false);

   EvaluateExpression(theEnv,theDeffacts->assertList,&returnValue);
  }

/**********************************************************/
/* ClearDeffacts: Deffacts clear routine for use with the */
/*   clear command. Creates the initial-facts deffacts.   */
/**********************************************************/
static void ClearDeffacts(
  Environment *theEnv)
  {
#if (! RUN_TIME) && (! BLOAD_ONLY)
   struct expr *stub;
   Deffacts *newDeffacts;

   /*=====================================*/
   /* Create the data structures for the  */
   /* expression (assert (initial-fact)). */
   /*=====================================*/

   stub = GenConstant(theEnv,FCALL,FindFunction(theEnv,"assert"));
   stub->argList = GenConstant(theEnv,DEFTEMPLATE_PTR,EnvFindDeftemplateInModule(theEnv,"initial-fact"));
   ExpressionInstall(theEnv,stub);

   /*=============================================*/
   /* Create a deffacts data structure to contain */
   /* the expression and initialize it.           */
   /*=============================================*/

   newDeffacts = get_struct(theEnv,deffacts);
   newDeffacts->header.whichModule =
      (struct defmoduleItemHeader *) GetDeffactsModuleItem(theEnv,NULL);
   newDeffacts->header.name = EnvCreateSymbol(theEnv,"initial-fact");
   IncrementSymbolCount(newDeffacts->header.name);
   newDeffacts->assertList = PackExpression(theEnv,stub);
   newDeffacts->header.next = NULL;
   newDeffacts->header.ppForm = NULL;
   newDeffacts->header.usrData = NULL;
   newDeffacts->header.constructType = DEFFACTS;
   newDeffacts->header.env = theEnv;
   ReturnExpression(theEnv,stub);

   /*===========================================*/
   /* Store the deffacts in the current module. */
   /*===========================================*/

   AddConstructToModule(&newDeffacts->header);
#else
#if MAC_XCD
#pragma unused(theEnv)
#endif
#endif
  }

/***************************************/
/* SaveDeffacts: Deffacts save routine */
/*   for use with the save command.    */
/***************************************/
static void SaveDeffacts(
  Environment *theEnv,
  Defmodule *theModule,
  const char *logicalName)
  {
   SaveConstruct(theEnv,theModule,logicalName,DeffactsData(theEnv)->DeffactsConstruct);
  }

/*******************************************/
/* UndeffactsCommand: H/L access routine   */
/*   for the undeffacts command.           */
/*******************************************/
void UndeffactsCommand(
  Environment *theEnv,
  UDFContext *context,
  UDFValue *returnValue)
  {
   UndefconstructCommand(context,"undeffacts",DeffactsData(theEnv)->DeffactsConstruct);
  }

/*********************************/
/* Undeffacts: C access routine  */
/*   for the undeffacts command. */
/*********************************/
bool Undeffacts(
  Deffacts *theDeffacts,
  Environment *allEnv)
  {
   Environment *theEnv;
   
   if (theDeffacts == NULL)
     { theEnv = allEnv; }
   else
     { theEnv = theDeffacts->header.env; }
   
   return Undefconstruct(theEnv,&theDeffacts->header,DeffactsData(theEnv)->DeffactsConstruct);
  }

/*************************************************/
/* GetDeffactsListFunction: H/L access routine   */
/*   for the get-deffacts-list function.         */
/*************************************************/
void GetDeffactsListFunction(
  Environment *theEnv,
  UDFContext *context,
  UDFValue *returnValue)
  {
   GetConstructListFunction(context,returnValue,DeffactsData(theEnv)->DeffactsConstruct);
  }

/*****************************************/
/* EnvGetDeffactsList: C access routine  */
/*   for the get-deffacts-list function. */
/*****************************************/
void EnvGetDeffactsList(
  Environment *theEnv,
  CLIPSValue *returnValue,
  Defmodule *theModule)
  {
   UDFValue result;
   
   GetConstructList(theEnv,&result,DeffactsData(theEnv)->DeffactsConstruct,theModule);
   NormalizeMultifield(theEnv,&result);
   returnValue->value = result.value;
  }

/************************************************/
/* DeffactsModuleFunction: H/L access routine   */
/*   for the deffacts-module function.          */
/************************************************/
void DeffactsModuleFunction(
  Environment *theEnv,
  UDFContext *context,
  UDFValue *returnValue)
  {
   returnValue->value = GetConstructModuleCommand(context,"deffacts-module",DeffactsData(theEnv)->DeffactsConstruct);
  }

#if DEBUGGING_FUNCTIONS

/*******************************************/
/* PPDeffactsCommand: H/L access routine   */
/*   for the ppdeffacts command.           */
/*******************************************/
void PPDeffactsCommand(
  Environment *theEnv,
  UDFContext *context,
  UDFValue *returnValue)
  {
   PPConstructCommand(context,"ppdeffacts",DeffactsData(theEnv)->DeffactsConstruct);
  }

/************************************/
/* PPDeffacts: C access routine for */
/*   the ppdeffacts command.        */
/************************************/
bool PPDeffacts(
  Environment *theEnv,
  const char *deffactsName,
  const char *logicalName)
  {
   return(PPConstruct(theEnv,deffactsName,logicalName,DeffactsData(theEnv)->DeffactsConstruct));
  }

/*********************************************/
/* ListDeffactsCommand: H/L access routine   */
/*   for the list-deffacts command.          */
/*********************************************/
void ListDeffactsCommand(
  Environment *theEnv,
  UDFContext *context,
  UDFValue *returnValue)
  {
   ListConstructCommand(context,DeffactsData(theEnv)->DeffactsConstruct);
  }

/*************************************/
/* EnvListDeffacts: C access routine */
/*   for the list-deffacts command.  */
/*************************************/
void EnvListDeffacts(
  Environment *theEnv,
  const char *logicalName,
  Defmodule *theModule)
  {
   ListConstruct(theEnv,DeffactsData(theEnv)->DeffactsConstruct,logicalName,theModule);
  }

#endif /* DEBUGGING_FUNCTIONS */

#endif /* DEFFACTS_CONSTRUCT */


