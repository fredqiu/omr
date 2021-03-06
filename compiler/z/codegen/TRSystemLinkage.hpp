/*******************************************************************************
 *
 * (c) Copyright IBM Corp. 2000, 2016
 *
 *  This program and the accompanying materials are made available
 *  under the terms of the Eclipse Public License v1.0 and
 *  Apache License v2.0 which accompanies this distribution.
 *
 *      The Eclipse Public License is available at
 *      http://www.eclipse.org/legal/epl-v10.html
 *
 *      The Apache License v2.0 is available at
 *      http://www.opensource.org/licenses/apache2.0.php
 *
 * Contributors:
 *    Multiple authors (IBM Corp.) - initial implementation and documentation
 *******************************************************************************/

#ifndef S390LINKAGE_INCL
#define S390LINKAGE_INCL

#include "codegen/TRzOSSystemLinkageBase.hpp"

#include <stdint.h>                               // for uint32_t, int32_t, etc
#include "codegen/Linkage.hpp"
#include "codegen/LinkageConventionsEnum.hpp"
#include "codegen/RealRegister.hpp"
#include "codegen/Register.hpp"                   // for Register
#include "codegen/SystemLinkage.hpp"              // for SystemLinkage
#include "cs2/arrayof.h"                          // for ArrayOf
#include "env/TRMemory.hpp"                       // for GlobalAllocator
#include "env/jittypes.h"                         // for intptrj_t
#include "il/DataTypes.hpp"                       // for DataTypes
#include "il/SymbolReference.hpp"                 // for SymbolReference

class TR_EntryPoint;
class TR_S390ConstantDataSnippet;
class TR_S390JNICallDataSnippet;
namespace TR { class AutomaticSymbol; }
namespace TR { class CodeGenerator; }
namespace TR { class Instruction; }
namespace TR { class LabelSymbol; }
namespace TR { class Node; }
namespace TR { class ParameterSymbol; }
namespace TR { class RegisterDependencyConditions; }
namespace TR { class ResolvedMethodSymbol; }
namespace TR { class Symbol; }
template <class T> class List;

////////////////////////////////////////////////////////////////////////////////
//  TR_S390zOSSystemLinkage Definition
////////////////////////////////////////////////////////////////////////////////

enum TR_XPLinkFrameType
   {
   TR_XPLinkUnknownFrame,
   TR_XPLinkStackLeafFrame,
   TR_XPLinkNoStackLeafFrame,
   TR_XPLinkSmallFrame,
   TR_XPLinkIntermediateFrame,
   TR_XPLinkStackCheckFrame
   };

/**
 * XPLink call types whose value is encoded in NOP following call.
 * These values are encoded - so don't change
 */
enum TR_XPLinkCallTypes {
   TR_XPLinkCallType_BASR      =0,     ///< generic BASR
   TR_XPLinkCallType_BRAS7     =1,     ///< BRAS  r7,ep
   TR_XPLinkCallType_RESVD_2   =2,     //
   TR_XPLinkCallType_BRASL7    =3,     ///< BRASL r7,ep
   TR_XPLinkCallType_RESVD_4   =4,     //
   TR_XPLinkCallType_RESVD_5   =5,     //
   TR_XPLinkCallType_BALR1415  =6,     ///< BALR  r14,r15
   TR_XPLinkCallType_BASR33    =7      ///< BASR  r3,r3
   };

class TR_S390zOSSystemLinkage : public TR::ZOSBaseSystemLinkageConnector
   {
   TR::RealRegister::RegNum _environmentPointerRegister;

public:

   TR_S390zOSSystemLinkage(TR::CodeGenerator * cg) ;

   virtual void generateInstructionsForCall(TR::Node * callNode, TR::RegisterDependencyConditions * dependencies,
         intptrj_t targetAddress, TR::Register * methodAddressReg, TR::Register * javaLitOffsetReg, TR::LabelSymbol * returnFromJNICallLabel,
         TR_S390JNICallDataSnippet * jniCallDataSnippet, bool isJNIGCPoint = true);

   virtual TR::Register * callNativeFunction(TR::Node * callNode, TR::RegisterDependencyConditions * dependencies,
      intptrj_t targetAddress, TR::Register * methodAddressReg, TR::Register * javaLitOffsetReg, TR::LabelSymbol * returnFromJNICallLabel, TR_S390JNICallDataSnippet * jniCallDataSnippet, bool isJNIGCPoint = true);

   virtual TR::RealRegister::RegNum setEnvironmentPointerRegister (TR::RealRegister::RegNum r) { return _environmentPointerRegister = r; }
   virtual TR::RealRegister::RegNum getEnvironmentPointerRegister() { return _environmentPointerRegister; }
   virtual TR::RealRegister *getEnvironmentPointerRealRegister() {return getS390RealRegister(_environmentPointerRegister);}

   virtual int32_t getRegisterSaveOffset(TR::RealRegister::RegNum);

public:
   TR_XPLinkFrameType getFrameType() { return _frameType; }
   void setFrameType(enum TR_XPLinkFrameType type) { _frameType = type; }
   virtual bool getIsLeafRoutine() { return (getFrameType() == TR_XPLinkStackLeafFrame) || (getFrameType() == TR_XPLinkNoStackLeafFrame); }
   virtual int32_t getOutgoingParameterBlockSize();

   virtual TR::Register *loadEnvironmentIntoRegister(TR::Node * callNode, TR::RegisterDependencyConditions * deps, bool loadIntoEnvironmentReg);
   bool isAggregateReturnedInIntRegisters(int32_t aggregateLenth);

   uint32_t calculateInterfaceMappingFlags(TR::ResolvedMethodSymbol *method);

   static uint32_t shiftFloatParmDescriptorFlag(uint32_t fieldVal, int32_t floatParmNum)  { return (fieldVal) << (6*(3-floatParmNum)); } // accepts floatParmNum values 0,1,2,3
   bool updateFloatParmDescriptorFlags(uint32_t *parmDescriptorFields, TR::Symbol *funcSymbol, int32_t parmCount, int32_t argSize, TR::DataType dataType, int32_t *floatParmNum, uint32_t *lastFloatParmAreaOffset, uint32_t *parmAreaOffset);

   static uint32_t getFloatParmDescriptorFlag(uint32_t descriptorFields, int32_t floatParmNum)  { return  (descriptorFields >> (6*(3-floatParmNum))) & 0x3F; }
   uint32_t calculateReturnValueAdjustFlag(TR::DataType dataType, int32_t aggregateLength);
   static uint32_t isFloatDescriptorFlagUnprototyped(uint32_t flag)  { return flag == 0; }

   virtual bool isEnvironmentSpecialArgumentRegister(int8_t linkageRegisterIndex)
     {
     bool result = isSpecialArgumentRegisters() &&
                   (linkageRegisterIndex >= TR_FirstSpecialLinkageIndex) &&
                   (linkageRegisterIndex - TR_FirstSpecialLinkageIndex) == 0 ;
     return result;
     }

   virtual bool isSpecialNonVolatileArgumentRegister(int8_t linkageRegisterIndex)
      {
      return isCAASpecialArgumentRegister(linkageRegisterIndex);
      }

   virtual bool isCAASpecialArgumentRegister(int8_t linkageRegisterIndex)
     {
     bool result = isSpecialArgumentRegisters() &&
                   (linkageRegisterIndex >= TR_FirstSpecialLinkageIndex) &&
                   (linkageRegisterIndex - TR_FirstSpecialLinkageIndex) == 1 ;
     return result;
     }

   virtual bool isParentDSASpecialArgumentRegister(int8_t linkageRegisterIndex)
     {
     return false;
     }

   uint32_t calculateCallDescriptorFlags(TR::Node *callNode);

private:
   enum TR_XPLinkFrameType _frameType;

   };


////////////////////////////////////////////////////////////////////////////////
//  TR_S390zLinuxSystemLinkage Definition
////////////////////////////////////////////////////////////////////////////////
class TR_S390zLinuxSystemLinkage : public TR::SystemLinkage
   {
   TR::RealRegister::RegNum _GOTPointerRegister;
public:

   TR_S390zLinuxSystemLinkage(TR::CodeGenerator * cg) ;

   virtual void generateInstructionsForCall(TR::Node * callNode, TR::RegisterDependencyConditions * deps, intptrj_t targetAddress,
         TR::Register * methodAddressReg, TR::Register * javaLitOffsetReg, TR::LabelSymbol * returnFromJNICallLabel,
         TR_S390JNICallDataSnippet * jniCallDataSnippet, bool isJNIGCPoint);
   virtual TR::Register * callNativeFunction(TR::Node * callNode, TR::RegisterDependencyConditions * dependencies,
      intptrj_t targetAddress, TR::Register * methodAddressReg, TR::Register * javaLitOffsetReg, TR::LabelSymbol * returnFromJNICallLabel, TR_S390JNICallDataSnippet * jniCallDataSnippet, bool isJNIGCPoint = true);

   virtual void setGOTPointerRegister (TR::RealRegister::RegNum r)         { _GOTPointerRegister = r; }
   virtual TR::RealRegister::RegNum getGOTPointerRegister()         { return _GOTPointerRegister; }
   virtual TR::RealRegister *getGOTPointerRealRegister() {return getS390RealRegister(_GOTPointerRegister);}
   virtual int32_t getRegisterSaveOffset(TR::RealRegister::RegNum);
   virtual void initParamOffset(TR::ResolvedMethodSymbol * method, int32_t stackIndex, List<TR::ParameterSymbol> *parameterList=0);

   virtual FrameType checkLeafRoutine(int32_t stackFrameSize, TR::Instruction **callInstruction = 0);

   virtual bool canDataTypeBePassedByReference(TR::DataType type);
   virtual bool isSymbolPassedByReference(TR::Symbol *sym);
   };


////////////////////////////////////////////////////////////////////////////////
//  TR_S39PLXSystemLinkage Definition
////////////////////////////////////////////////////////////////////////////////

#endif
