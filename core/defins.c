   /*******************************************************/
   /*      "C" Language Integrated Production System      */
   /*                                                     */
   /*            CLIPS Version 6.40  08/25/16             */
   /*                                                     */
   /*                  DEFINSTANCES MODULE                */
   /*******************************************************/

/*************************************************************/
/* Purpose: Kernel definstances interface commands           */
/*              and routines                                 */
/*                                                           */
/* Principal Programmer(s):                                  */
/*      Brian L. Dantes                                      */
/*                                                           */
/* Contributing Programmer(s):                               */
/*                                                           */
/*                                                           */
/* Revision History:                                         */
/*                                                           */
/*      6.23: Correction for FalseSymbol/TrueSymbol. DR0859  */
/*                                                           */
/*            Corrected compilation errors for files         */
/*            generated by constructs-to-c. DR0861           */
/*                                                           */
/*            Changed name of variable exp to theExp         */
/*            because of Unix compiler warnings of shadowed  */
/*            definitions.                                   */
/*                                                           */
/*      6.24: Converted INSTANCE_PATTERN_MATCHING to         */
/*            DEFRULE_CONSTRUCT.                             */
/*                                                           */
/*            Renamed BOOLEAN macro type to intBool.         */
/*                                                           */
/*      6.30: Removed conditional code for unsupported       */
/*            compilers/operating systems (IBM_MCW,          */
/*            MAC_MCW, and IBM_TBC).                         */
/*                                                           */
/*            GetConstructNameAndComment API change.         */
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
/*      6.40: Pragma once and other inclusion changes.       */
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

/* =========================================
   *****************************************
               EXTERNAL DEFINITIONS
   =========================================
   ***************************************** */
#include "setup.h"

#if DEFINSTANCES_CONSTRUCT

#if BLOAD || BLOAD_ONLY || BLOAD_AND_BSAVE
#include "bload.h"
#include "dfinsbin.h"
#endif

#if CONSTRUCT_COMPILER && (! RUN_TIME)
#include "dfinscmp.h"
#endif

#include "argacces.h"
#include "classcom.h"
#include "classfun.h"
#include "cstrccom.h"
#include "cstrcpsr.h"
#include "constant.h"
#include "constrct.h"
#include "envrnmnt.h"
#include "evaluatn.h"
#include "extnfunc.h"
#include "insfun.h"
#include "inspsr.h"
#include "memalloc.h"
#include "modulpsr.h"
#include "router.h"
#include "scanner.h"
#include "symbol.h"
#include "utility.h"

#include "defins.h"

/* =========================================
   *****************************************
                   CONSTANTS
   =========================================
   ***************************************** */
#define ACTIVE_RLN "active"

/* =========================================
   *****************************************
      INTERNALLY VISIBLE FUNCTION HEADERS
   =========================================
   ***************************************** */

#if (! BLOAD_ONLY) && (! RUN_TIME)
   static bool                    ParseDefinstances(Environment *,const char *);
   static CLIPSLexeme            *ParseDefinstancesName(Environment *,const char *,bool *);
   static void                    RemoveDefinstances(Environment *,Definstances *);
   static void                    SaveDefinstances(Environment *,Defmodule *,const char *);
   static bool                    RemoveAllDefinstances(Environment *);
   static void                    DefinstancesDeleteError(Environment *,const char *);

#if DEFRULE_CONSTRUCT
   static void                    CreateInitialDefinstances(Environment *);
#endif
#endif

#if ! RUN_TIME
   static void                   *AllocateModule(Environment *);
   static void                    ReturnModule(Environment *,void *);
   static bool                    ClearDefinstancesReady(Environment *);
   static void                    CheckDefinstancesBusy(Environment *,struct constructHeader *,void *);
   static void                    DestroyDefinstancesAction(Environment *,struct constructHeader *,void *);
#endif

   static void                    ResetDefinstances(Environment *);
   static void                    ResetDefinstancesAction(Environment *,struct constructHeader *,void *);
   static void                    DeallocateDefinstancesData(Environment *);

/* =========================================
   *****************************************
          EXTERNALLY VISIBLE FUNCTIONS
   =========================================
   ***************************************** */

/***************************************************
  NAME         : SetupDefinstances
  DESCRIPTION  : Adds the definstance support routines
                   to the Kernel
  INPUTS       : None
  RETURNS      : Nothing useful
  SIDE EFFECTS : Appropriate function lists modified
  NOTES        : None
 ***************************************************/
void SetupDefinstances(
  Environment *theEnv)
  {
   AllocateEnvironmentData(theEnv,DEFINSTANCES_DATA,sizeof(struct definstancesData),DeallocateDefinstancesData);

   DefinstancesData(theEnv)->DefinstancesModuleIndex =
                RegisterModuleItem(theEnv,"definstances",
#if (! RUN_TIME)
                                    AllocateModule,
                                    ReturnModule,
#else
                                    NULL,NULL,
#endif
#if BLOAD_AND_BSAVE || BLOAD || BLOAD_ONLY
                                    BloadDefinstancesModuleRef,
#else
                                    NULL,
#endif
#if CONSTRUCT_COMPILER && (! RUN_TIME)
                                    DefinstancesCModuleReference,
#else
                                    NULL,
#endif
                                    (FindConstructFunction *) EnvFindDefinstancesInModule);

   DefinstancesData(theEnv)->DefinstancesConstruct =
      AddConstruct(theEnv,"definstances","definstances",
#if (! BLOAD_ONLY) && (! RUN_TIME)
                   ParseDefinstances,
#else
                   NULL,
#endif
                   (FindConstructFunction *) EnvFindDefinstances,
                   GetConstructNamePointer,GetConstructPPForm,
                   GetConstructModuleItem,
                   (GetNextConstructFunction *) EnvGetNextDefinstances,
                   SetNextConstruct,
                   (IsConstructDeletableFunction *) EnvIsDefinstancesDeletable,
                   (DeleteConstructFunction *) EnvUndefinstances,
#if (! BLOAD_ONLY) && (! RUN_TIME)
                   (FreeConstructFunction *) RemoveDefinstances
#else
                   NULL
#endif
                   );

#if ! RUN_TIME
   AddClearReadyFunction(theEnv,"definstances",ClearDefinstancesReady,0);

#if ! BLOAD_ONLY
   EnvAddUDF(theEnv,"undefinstances","v",1,1,"y",UndefinstancesCommand,"UndefinstancesCommand",NULL);
   AddSaveFunction(theEnv,"definstances",SaveDefinstances,0);

#if DEFRULE_CONSTRUCT
   EnvAddClearFunction(theEnv,"definstances",CreateInitialDefinstances,-1000);
#endif

#endif

#if DEBUGGING_FUNCTIONS
   EnvAddUDF(theEnv,"ppdefinstances","v",1,1,"y",PPDefinstancesCommand,"PPDefinstancesCommand",NULL);
   EnvAddUDF(theEnv,"list-definstances","v",0,1,"y",ListDefinstancesCommand,"ListDefinstancesCommand",NULL);
#endif

   EnvAddUDF(theEnv,"get-definstances-list","m",0,1,"y",GetDefinstancesListFunction,"GetDefinstancesListFunction",NULL);
   EnvAddUDF(theEnv,"definstances-module","y",1,1,"y",GetDefinstancesModuleCommand,"GetDefinstancesModuleCommand",NULL);

#endif
   EnvAddResetFunction(theEnv,"definstances",ResetDefinstances,0);

#if BLOAD || BLOAD_ONLY || BLOAD_AND_BSAVE
   SetupDefinstancesBload(theEnv);
#endif

#if CONSTRUCT_COMPILER && (! RUN_TIME)
   SetupDefinstancesCompiler(theEnv);
#endif
  }

/*******************************************************/
/* DeallocateDefinstancesData: Deallocates environment */
/*    data for the definstances construct.             */
/*******************************************************/
static void DeallocateDefinstancesData(
  Environment *theEnv)
  {
#if ! RUN_TIME
   struct definstancesModule *theModuleItem;
   Defmodule *theModule;

#if BLOAD || BLOAD_AND_BSAVE
   if (Bloaded(theEnv)) return;
#endif

   DoForAllConstructs(theEnv,DestroyDefinstancesAction,DefinstancesData(theEnv)->DefinstancesModuleIndex,false,NULL);

   for (theModule = EnvGetNextDefmodule(theEnv,NULL);
        theModule != NULL;
        theModule = EnvGetNextDefmodule(theEnv,theModule))
     {
      theModuleItem = (struct definstancesModule *)
                      GetModuleItem(theEnv,theModule,
                                    DefinstancesData(theEnv)->DefinstancesModuleIndex);
      rtn_struct(theEnv,definstancesModule,theModuleItem);
     }
#else
#if MAC_XCD
#pragma unused(theEnv)
#endif
#endif
  }

#if ! RUN_TIME
/*****************************************************/
/* DestroyDefinstancesAction: Action used to remove  */
/*   definstances as a result of DestroyEnvironment. */
/*****************************************************/
static void DestroyDefinstancesAction(
  Environment *theEnv,
  struct constructHeader *theConstruct,
  void *buffer)
  {
#if MAC_XCD
#pragma unused(buffer)
#endif
#if (! BLOAD_ONLY) && (! RUN_TIME)
   struct definstances *theDefinstances = (struct definstances *) theConstruct;

   if (theDefinstances == NULL) return;

   ReturnPackedExpression(theEnv,theDefinstances->mkinstance);

   DestroyConstructHeader(theEnv,&theDefinstances->header);

   rtn_struct(theEnv,definstances,theDefinstances);
#else
#if MAC_XCD
#pragma unused(theConstruct,theEnv)
#endif
#endif
  }
#endif

/***********************************************************
  NAME         : EnvGetNextDefinstances
  DESCRIPTION  : Finds first or next definstances
  INPUTS       : The address of the current definstances
  RETURNS      : The address of the next definstances
                   (NULL if none)
  SIDE EFFECTS : None
  NOTES        : If ptr == NULL, the first definstances
                    is returned.
 ***********************************************************/
Definstances *EnvGetNextDefinstances(
  Environment *theEnv,
  Definstances *theDefinstances)
  {
   return (Definstances *) GetNextConstructItem(theEnv,(struct constructHeader *) theDefinstances,
                                                DefinstancesData(theEnv)->DefinstancesModuleIndex);
  }

/***************************************************
  NAME         : EnvFindDefinstances
  DESCRIPTION  : Looks up a definstance construct
                   by name-string
  INPUTS       : The symbolic name
  RETURNS      : The definstance address, or NULL
                    if not found
  SIDE EFFECTS : None
  NOTES        : None
 ***************************************************/
Definstances *EnvFindDefinstances(
  Environment *theEnv,
  const char *name)
  {
   return (Definstances *) FindNamedConstructInModuleOrImports(theEnv,name,DefinstancesData(theEnv)->DefinstancesConstruct);
  }

/***************************************************
  NAME         : EnvFindDefinstancesInModule
  DESCRIPTION  : Looks up a definstance construct
                   by name-string
  INPUTS       : The symbolic name
  RETURNS      : The definstance address, or NULL
                    if not found
  SIDE EFFECTS : None
  NOTES        : None
 ***************************************************/
Definstances *EnvFindDefinstancesInModule(
  Environment *theEnv,
  const char *name)
  {
   return (Definstances *) FindNamedConstructInModule(theEnv,name,DefinstancesData(theEnv)->DefinstancesConstruct);
  }

/***************************************************
  NAME         : EnvIsDefinstancesDeletable
  DESCRIPTION  : Determines if a definstances
                   can be deleted
  INPUTS       : Address of the definstances
  RETURNS      : True if deletable, false otherwise
  SIDE EFFECTS : None
  NOTES        : None
 ***************************************************/
bool EnvIsDefinstancesDeletable(
  Environment *theEnv,
  Definstances *theDefinstances)
  {
   if (! ConstructsDeletable(theEnv))
     { return false; }

   return (theDefinstances->busy == 0) ? true : false;
  }

/***********************************************************
  NAME         : UndefinstancesCommand
  DESCRIPTION  : Removes a definstance
  INPUTS       : None
  RETURNS      : Nothing useful
  SIDE EFFECTS : Definstance deallocated
  NOTES        : H/L Syntax : (undefinstances <name> | *)
 ***********************************************************/
void UndefinstancesCommand(
  Environment *theEnv,
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   UndefconstructCommand(context,"undefinstances",DefinstancesData(theEnv)->DefinstancesConstruct);
  }

/*****************************************************************
  NAME         : GetDefinstancesModuleCommand
  DESCRIPTION  : Determines to which module a definstances belongs
  INPUTS       : None
  RETURNS      : The symbolic name of the module
  SIDE EFFECTS : None
  NOTES        : H/L Syntax: (definstances-module <defins-name>)
 *****************************************************************/
void GetDefinstancesModuleCommand(
  Environment *theEnv,
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   returnValue->value = GetConstructModuleCommand(context,"definstances-module",DefinstancesData(theEnv)->DefinstancesConstruct);
  }

/***********************************************************
  NAME         : EnvUndefinstances
  DESCRIPTION  : Removes a definstance
  INPUTS       : Address of definstances to remove
  RETURNS      : True if successful,
                 false otherwise
  SIDE EFFECTS : Definstance deallocated
  NOTES        : None
 ***********************************************************/
bool EnvUndefinstances(
  Environment *theEnv,
  Definstances *theDefinstances)
  {
#if RUN_TIME || BLOAD_ONLY
#if MAC_XCD
#pragma unused(theEnv,vptr)
#endif
   return false;
#else

#if BLOAD || BLOAD_AND_BSAVE
   if (Bloaded(theEnv))
     return false;
#endif
   if (theDefinstances == NULL)
     { return RemoveAllDefinstances(theEnv); }

   if (EnvIsDefinstancesDeletable(theEnv,theDefinstances) == false)
     { return false; }

   RemoveConstructFromModule(theEnv,(struct constructHeader *) theDefinstances);
   RemoveDefinstances(theEnv,theDefinstances);

   return true;
#endif
  }

#if DEBUGGING_FUNCTIONS

/***************************************************************
  NAME         : PPDefinstancesCommand
  DESCRIPTION  : Prints out the pretty-print form of a definstance
  INPUTS       : None
  RETURNS      : Nothing useful
  SIDE EFFECTS : None
  NOTES        : H/L Syntax : (ppdefinstances <name>)
 ***************************************************************/
void PPDefinstancesCommand(
  Environment *theEnv,
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   PPConstructCommand(context,"ppdefinstances",DefinstancesData(theEnv)->DefinstancesConstruct);
  }

/***************************************************
  NAME         : ListDefinstancesCommand
  DESCRIPTION  : Displays all definstances names
  INPUTS       : None
  RETURNS      : Nothing useful
  SIDE EFFECTS : Definstances name sprinted
  NOTES        : H/L Interface
 ***************************************************/
void ListDefinstancesCommand(
  Environment *theEnv,
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   ListConstructCommand(context,DefinstancesData(theEnv)->DefinstancesConstruct);
  }

/***************************************************
  NAME         : EnvListDefinstances
  DESCRIPTION  : Displays all definstances names
  INPUTS       : 1) The logical name of the output
                 2) The module
  RETURNS      : Nothing useful
  SIDE EFFECTS : Definstances names printed
  NOTES        : C Interface
 ***************************************************/
void EnvListDefinstances(
  Environment *theEnv,
  const char *logicalName,
  Defmodule *theModule)
  {
   ListConstruct(theEnv,DefinstancesData(theEnv)->DefinstancesConstruct,logicalName,theModule);
  }

#endif

/****************************************************************
  NAME         : GetDefinstancesListFunction
  DESCRIPTION  : Groups all definstances names into
                 a multifield list
  INPUTS       : A data object buffer to hold
                 the multifield result
  RETURNS      : Nothing useful
  SIDE EFFECTS : Multifield allocated and filled
  NOTES        : H/L Syntax: (get-definstances-list [<module>])
 ****************************************************************/
void GetDefinstancesListFunction(
  Environment *theEnv,
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   GetConstructListFunction(context,returnValue,DefinstancesData(theEnv)->DefinstancesConstruct);
  }

/***************************************************************
  NAME         : EnvGetDefinstancesList
  DESCRIPTION  : Groups all definstances names into
                 a multifield list
  INPUTS       : 1) A data object buffer to hold
                    the multifield result
                 2) The module from which to obtain definstances
  RETURNS      : Nothing useful
  SIDE EFFECTS : Multifield allocated and filled
  NOTES        : External C access
 ***************************************************************/
void EnvGetDefinstancesList(
  Environment *theEnv,
  CLIPSValue *returnValue,
  Defmodule *theModule)
  {
   GetConstructList(theEnv,returnValue,DefinstancesData(theEnv)->DefinstancesConstruct,theModule);
  }

/* =========================================
   *****************************************
          INTERNALLY VISIBLE FUNCTIONS
   =========================================
   ***************************************** */

#if (! BLOAD_ONLY) && (! RUN_TIME)

/*********************************************************************
  NAME         : ParseDefinstances
  DESCRIPTION  : Parses and allocates a definstances construct
  INPUTS       : The logical name of the input source
  RETURNS      : False if no errors, true otherwise
  SIDE EFFECTS : Definstances parsed and created
  NOTES        : H/L Syntax :

                 (definstances  <name> [active] [<comment>]
                    <instance-definition>+)

                 <instance-definition> ::=
                    (<instance-name> of <class-name> <slot-override>*)

                 <slot-override> ::= (<slot-name> <value-expression>*)
 *********************************************************************/
static bool ParseDefinstances(
  Environment *theEnv,
  const char *readSource)
  {
   CLIPSLexeme *dname;
   struct FunctionDefinition *mkinsfcall;
   EXPRESSION *mkinstance,*mkbot = NULL;
   Definstances *dobj;
   bool active;

   SetPPBufferStatus(theEnv,true);
   FlushPPBuffer(theEnv);
   SetIndentDepth(theEnv,3);
   SavePPBuffer(theEnv,"(definstances ");

#if BLOAD || BLOAD_AND_BSAVE
   if ((Bloaded(theEnv)) && (! ConstructData(theEnv)->CheckSyntaxMode))
     {
      CannotLoadWithBloadMessage(theEnv,"definstances");
      return true;
     }
#endif
   dname = ParseDefinstancesName(theEnv,readSource,&active);
   if (dname == NULL)
     return true;

   dobj = get_struct(theEnv,definstances);
   InitializeConstructHeader(theEnv,"definstances",DEFINSTANCES,(struct constructHeader *) dobj,dname);
   dobj->busy = 0;
   dobj->mkinstance = NULL;
#if DEFRULE_CONSTRUCT
   if (active)
     mkinsfcall = FindFunction(theEnv,"active-make-instance");
   else
     mkinsfcall = FindFunction(theEnv,"make-instance");
#else
   mkinsfcall = FindFunction(theEnv,"make-instance");
#endif
   while (DefclassData(theEnv)->ObjectParseToken.tknType == LEFT_PARENTHESIS_TOKEN)
     {
      mkinstance = GenConstant(theEnv,UNKNOWN_VALUE,mkinsfcall);
      mkinstance = ParseInitializeInstance(theEnv,mkinstance,readSource);
      if (mkinstance == NULL)
        {
         ReturnExpression(theEnv,dobj->mkinstance);
         rtn_struct(theEnv,definstances,dobj);
         return true;
        }
      if (ExpressionContainsVariables(mkinstance,false) == true)
        {
         LocalVariableErrorMessage(theEnv,"definstances");
         ReturnExpression(theEnv,mkinstance);
         ReturnExpression(theEnv,dobj->mkinstance);
         rtn_struct(theEnv,definstances,dobj);
         return true;
        }
      if (mkbot == NULL)
        dobj->mkinstance = mkinstance;
      else
        GetNextArgument(mkbot) = mkinstance;
      mkbot = mkinstance;
      GetToken(theEnv,readSource,&DefclassData(theEnv)->ObjectParseToken);
      PPBackup(theEnv);
      PPCRAndIndent(theEnv);
      SavePPBuffer(theEnv,DefclassData(theEnv)->ObjectParseToken.printForm);
     }

   if (DefclassData(theEnv)->ObjectParseToken.tknType != RIGHT_PARENTHESIS_TOKEN)
     {
      ReturnExpression(theEnv,dobj->mkinstance);
      rtn_struct(theEnv,definstances,dobj);
      SyntaxErrorMessage(theEnv,"definstances");
      return true;
     }
   else
     {
      if (ConstructData(theEnv)->CheckSyntaxMode)
        {
         ReturnExpression(theEnv,dobj->mkinstance);
         rtn_struct(theEnv,definstances,dobj);
         return false;
        }
#if DEBUGGING_FUNCTIONS
      if (EnvGetConserveMemory(theEnv) == false)
        {
         if (dobj->mkinstance != NULL)
           PPBackup(theEnv);
         PPBackup(theEnv);
         SavePPBuffer(theEnv,")\n");
         EnvSetDefinstancesPPForm(theEnv,dobj,CopyPPBuffer(theEnv));
        }
#endif
      mkinstance = dobj->mkinstance;
      dobj->mkinstance = PackExpression(theEnv,mkinstance);
      ReturnExpression(theEnv,mkinstance);
      IncrementSymbolCount(EnvGetDefinstancesNamePointer(theEnv,dobj));
      ExpressionInstall(theEnv,dobj->mkinstance);
     }

   AddConstructToModule((struct constructHeader *) dobj);
   return false;
  }

/*************************************************************
  NAME         : ParseDefinstancesName
  DESCRIPTION  : Parses definstance name and optional comment
                 and optional "active" keyword
  INPUTS       : 1) The logical name of the input source
                 2) Buffer to hold flag indicating if
                    definstances should cause pattern-matching
                    to occur during slot-overrides
  RETURNS      : Address of name symbol, or
                   NULL if there was an error
  SIDE EFFECTS : Token after name or comment is scanned
  NOTES        : Assumes "(definstances" has already
                   been scanned.
 *************************************************************/
static CLIPSLexeme *ParseDefinstancesName(
  Environment *theEnv,
  const char *readSource,
  bool *active)
  {
   CLIPSLexeme *dname;

   *active = false;
   dname = GetConstructNameAndComment(theEnv,readSource,&DefclassData(theEnv)->ObjectParseToken,"definstances",
                                      (FindConstructFunction *) EnvFindDefinstancesInModule,
                                      (DeleteConstructFunction *) EnvUndefinstances,"@",
                                      true,false,true,false);
   if (dname == NULL)
     return NULL;

#if DEFRULE_CONSTRUCT
   if ((DefclassData(theEnv)->ObjectParseToken.tknType != SYMBOL_TYPE) ? false :
       (strcmp(DefclassData(theEnv)->ObjectParseToken.lexemeValue->contents,ACTIVE_RLN) == 0))
     {
      PPBackup(theEnv);
      PPBackup(theEnv);
      SavePPBuffer(theEnv," ");
      SavePPBuffer(theEnv,DefclassData(theEnv)->ObjectParseToken.printForm);
      PPCRAndIndent(theEnv);
      GetToken(theEnv,readSource,&DefclassData(theEnv)->ObjectParseToken);
      *active = true;
     }
#endif
   if (DefclassData(theEnv)->ObjectParseToken.tknType == STRING_TOKEN)
     {
      PPBackup(theEnv);
      PPBackup(theEnv);
      SavePPBuffer(theEnv," ");
      SavePPBuffer(theEnv,DefclassData(theEnv)->ObjectParseToken.printForm);
      PPCRAndIndent(theEnv);
      GetToken(theEnv,readSource,&DefclassData(theEnv)->ObjectParseToken);
     }
   return(dname);
  }

/**************************************************************
  NAME         : RemoveDefinstances
  DESCRIPTION  : Deallocates and removes a definstance construct
  INPUTS       : The definstance address
  RETURNS      : Nothing useful
  SIDE EFFECTS : Existing definstance construct deleted
  NOTES        : Assumes busy count of definstance is 0
 **************************************************************/
static void RemoveDefinstances(
  Environment *theEnv,
  Definstances *theDefinstances)
  {
   DecrementSymbolCount(theEnv,EnvGetDefinstancesNamePointer(theEnv,theDefinstances));
   ExpressionDeinstall(theEnv,theDefinstances->mkinstance);
   ReturnPackedExpression(theEnv,theDefinstances->mkinstance);
   EnvSetDefinstancesPPForm(theEnv,theDefinstances,NULL);
   ClearUserDataList(theEnv,theDefinstances->header.usrData);
   rtn_struct(theEnv,definstances,theDefinstances);
  }

/***************************************************
  NAME         : SaveDefinstances
  DESCRIPTION  : Prints pretty print form of
                   definstances to specified output
  INPUTS       : The logical name of the output
  RETURNS      : Nothing useful
  SIDE EFFECTS : None
  NOTES        : None
 ***************************************************/
static void SaveDefinstances(
  Environment *theEnv,
  Defmodule *theModule,
  const char *logName)
  {
   SaveConstruct(theEnv,theModule,logName,DefinstancesData(theEnv)->DefinstancesConstruct);
  }

/***************************************************
  NAME         : RemoveAllDefinstances
  DESCRIPTION  : Removes all definstances constructs
  INPUTS       : None
  RETURNS      : True if successful,
                 false otherwise
  SIDE EFFECTS : All definstances deallocated
  NOTES        : None
 ***************************************************/
static bool RemoveAllDefinstances(
  Environment *theEnv)
  {
   Definstances *dptr,*dhead;
   bool success = true;

#if BLOAD || BLOAD_AND_BSAVE

   if (Bloaded(theEnv))
     return false;
#endif
  dhead = EnvGetNextDefinstances(theEnv,NULL);
  while (dhead != NULL)
    {
     dptr = dhead;
     dhead = EnvGetNextDefinstances(theEnv,dhead);
     if (EnvIsDefinstancesDeletable(theEnv,dptr))
       {
        RemoveConstructFromModule(theEnv,(struct constructHeader *) dptr);
        RemoveDefinstances(theEnv,dptr);
       }
     else
       {
        DefinstancesDeleteError(theEnv,EnvGetDefinstancesName(theEnv,dptr));
        success = false;
       }
    }
   return(success);
  }

/***************************************************
  NAME         : DefinstancesDeleteError
  DESCRIPTION  : Prints an error message for
                 unsuccessful definstances
                 deletion attempts
  INPUTS       : The name of the definstances
  RETURNS      : Nothing useful
  SIDE EFFECTS : Error message printed
  NOTES        : None
 ***************************************************/
static void DefinstancesDeleteError(
  Environment *theEnv,
  const char *dname)
  {
   CantDeleteItemErrorMessage(theEnv,"definstances",dname);
  }

#if DEFRULE_CONSTRUCT

/********************************************************
  NAME         : CreateInitialDefinstances
  DESCRIPTION  : Makes the initial-object definstances
                 structure for creating an initial-object
                 which will match default object patterns
                 in defrules
  INPUTS       : None
  RETURNS      : Nothing useful
  SIDE EFFECTS : initial-object definstances created
  NOTES        : None
 ********************************************************/
static void CreateInitialDefinstances(
  Environment *theEnv)
  {
   EXPRESSION *tmp;
   Definstances *theDefinstances;

   theDefinstances = get_struct(theEnv,definstances);
   InitializeConstructHeader(theEnv,"definstances",DEFINSTANCES,(struct constructHeader *) theDefinstances,
                             DefclassData(theEnv)->INITIAL_OBJECT_SYMBOL);
   theDefinstances->busy = 0;
   tmp = GenConstant(theEnv,FCALL,FindFunction(theEnv,"make-instance"));
   tmp->argList = GenConstant(theEnv,INSTANCE_NAME,DefclassData(theEnv)->INITIAL_OBJECT_SYMBOL);
   tmp->argList->nextArg =
       GenConstant(theEnv,DEFCLASS_PTR,LookupDefclassInScope(theEnv,INITIAL_OBJECT_CLASS_NAME));
   theDefinstances->mkinstance = PackExpression(theEnv,tmp);
   ReturnExpression(theEnv,tmp);
   IncrementSymbolCount(EnvGetDefinstancesNamePointer(theEnv,theDefinstances));
   ExpressionInstall(theEnv,theDefinstances->mkinstance);
   AddConstructToModule((struct constructHeader *) theDefinstances);
  }

#endif

#endif

#if ! RUN_TIME

/*****************************************************
  NAME         : AllocateModule
  DESCRIPTION  : Creates and initializes a
                 list of definstances for a new module
  INPUTS       : None
  RETURNS      : The new definstances module
  SIDE EFFECTS : Definstances module created
  NOTES        : None
 *****************************************************/
static void *AllocateModule(
  Environment *theEnv)
  {
   return (void *) get_struct(theEnv,definstancesModule);
  }

/***************************************************
  NAME         : ReturnModule
  DESCRIPTION  : Removes a definstances module and
                 all associated definstances
  INPUTS       : The definstances module
  RETURNS      : Nothing useful
  SIDE EFFECTS : Module and definstances deleted
  NOTES        : None
 ***************************************************/
static void ReturnModule(
  Environment *theEnv,
  void *theItem)
  {
#if (! BLOAD_ONLY)
   FreeConstructHeaderModule(theEnv,(struct defmoduleItemHeader *) theItem,DefinstancesData(theEnv)->DefinstancesConstruct);
#endif
   rtn_struct(theEnv,definstancesModule,theItem);
  }

/***************************************************
  NAME         : ClearDefinstancesReady
  DESCRIPTION  : Determines if it is safe to
                 remove all definstances
                 Assumes *all* constructs will be
                 deleted
  INPUTS       : None
  RETURNS      : True if all definstances can
                 be deleted, false otherwise
  SIDE EFFECTS : None
  NOTES        : Used by (clear) and (bload)
 ***************************************************/
static bool ClearDefinstancesReady(
  Environment *theEnv)
  {
   bool flagBuffer = true;

   DoForAllConstructs(theEnv,CheckDefinstancesBusy,DefinstancesData(theEnv)->DefinstancesModuleIndex,
                      false,&flagBuffer);
   return(flagBuffer);
  }

/***************************************************
  NAME         : CheckDefinstancesBusy
  DESCRIPTION  : Determines if a definstances is
                 in use or not
  INPUTS       : 1) The definstances
                 2) A buffer to set to 0 if the
                    the definstances is busy
  RETURNS      : Nothing useful
  SIDE EFFECTS : Buffer set to 0 if definstances
                 busy
  NOTES        : The flag buffer is not modified
                 if definstances is not busy
                 (assumed to be initialized to 1)
 ***************************************************/
static void CheckDefinstancesBusy(
  Environment *theEnv,
  struct constructHeader *theDefinstances,
  void *userBuffer)
  {
#if MAC_XCD
#pragma unused(theEnv)
#endif

   if (((Definstances *) theDefinstances)->busy > 0)
     { *((bool *) userBuffer) = false; }
  }

#endif

/***************************************************
  NAME         : ResetDefinstances
  DESCRIPTION  : Calls EvaluateExpression for each of
                   the make-instance calls in all
                   of the definstances constructs
  INPUTS       : None
  RETURNS      : Nothing useful
  SIDE EFFECTS : All instances in the definstances
                   are evaluated (and created if
                   there are no errors)
                 Any previously existing instances
                 are deleted first.
  NOTES        : None
 ***************************************************/
static void ResetDefinstances(
  Environment *theEnv)
  {
   DoForAllConstructs(theEnv,ResetDefinstancesAction,DefinstancesData(theEnv)->DefinstancesModuleIndex,true,NULL);
  }

/***************************************************
  NAME         : ResetDefinstancesAction
  DESCRIPTION  : Performs all the make-instance
                 calls in a definstances
  INPUTS       : 1) The definstances
                 2) User data buffer (ignored)
  RETURNS      : Nothing useful
  SIDE EFFECTS : Instances created
  NOTES        : None
 ***************************************************/
static void ResetDefinstancesAction(
  Environment *theEnv,
  struct constructHeader *vDefinstances,
  void *userBuffer)
  {
#if MAC_XCD
#pragma unused(userBuffer)
#endif
   Definstances *theDefinstances = (Definstances *) vDefinstances;
   EXPRESSION *theExp;
   CLIPSValue temp;

   SaveCurrentModule(theEnv);
   EnvSetCurrentModule(theEnv,vDefinstances->whichModule->theModule);
   theDefinstances->busy++;
   for (theExp = theDefinstances->mkinstance ;
        theExp != NULL ;
        theExp = GetNextArgument(theExp))
     {
      EvaluateExpression(theEnv,theExp,&temp);
      if (EvaluationData(theEnv)->HaltExecution ||
          (temp.value == theEnv->FalseSymbol))
        {
         RestoreCurrentModule(theEnv);
         theDefinstances->busy--;
         return;
        }
     }
   theDefinstances->busy--;
   RestoreCurrentModule(theEnv);
  }

/*##################################*/
/* Additional Environment Functions */
/*##################################*/

const char *EnvGetDefinstancesName(
  Environment *theEnv,
  Definstances *theDefinstances)
  {
   return GetConstructNameString((struct constructHeader *) theDefinstances);
  }

const char *EnvGetDefinstancesPPForm(
  Environment *theEnv,
  Definstances *theDefinstances)
  {
   return GetConstructPPForm(theEnv,(struct constructHeader *) theDefinstances);
  }

void EnvSetDefinstancesPPForm(
  Environment *theEnv,
  Definstances *theDefinstances,
  const char *thePPForm)
  {
   SetConstructPPForm(theEnv,(struct constructHeader *) theDefinstances,thePPForm);
  }

const char *EnvDefinstancesModule(
  Environment *theEnv,
  Definstances *theDefinstances)
  {
   return GetConstructModuleName((struct constructHeader *) theDefinstances);
  }

CLIPSLexeme *EnvGetDefinstancesNamePointer(
  Environment *theEnv,
  Definstances *theDefinstances)
  {
   return GetConstructNamePointer((struct constructHeader *) theDefinstances);
  }

const char *EnvDefinstancesModuleName(
  Environment *theEnv,
  Definstances *theDefinstances)
  {
   return GetConstructModuleName((struct constructHeader *) theDefinstances);
  }

#endif


