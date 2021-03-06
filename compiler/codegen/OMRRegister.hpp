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

#ifndef OMR_REGISTER_INCL
#define OMR_REGISTER_INCL

/*
 * The following #define and typedef must appear before any #includes in this file
 */
#ifndef OMR_REGISTER_CONNECTOR
#define OMR_REGISTER_CONNECTOR
namespace OMR { class Register; }
namespace OMR { typedef OMR::Register RegisterConnector; }
#endif

#include <stddef.h>                         // for NULL
#include <stdint.h>                         // for uint32_t, uint16_t
#include "codegen/RegisterConstants.hpp"    // for TR_RegisterKinds, etc
#include "env/TRMemory.hpp"                 // for TR_Memory, etc
#include "il/Node.hpp"                      // for ncount_t, MAX_NODE_COUNT
#include "infra/Assert.hpp"                 // for TR_ASSERT
#include "infra/Flags.hpp"                  // for flags32_t
#include "infra/Annotations.hpp"            // for OMR_EXTENSIBLE

class TR_BackingStore;
namespace TR { class AutomaticSymbol; }
namespace TR { class Compilation; }
namespace TR { class Instruction; }
namespace TR { class RealRegister; }
namespace TR { class Register; }
namespace TR { class RegisterPair; }

#define TR_SSR_ASSERT() TR_ASSERT(self()->getKind() != TR_SSR,"only non-TR_SSR registers use _totalUseCount : an aggregate or BCD type node has been evaluated as a non-aggregate or BCD node (missing o2x cast?)\n")
#define TR_MAX_USECOUNT_ASSERT(a) TR_ASSERT(_totalUseCount <= MAX_NODE_COUNT-a, "TR::Register::_totalUseCount == MAX_NODE_COUNT")

namespace OMR
{

class OMR_EXTENSIBLE Register
   {
   protected:
   Register(uint32_t f=0);
   Register(TR_RegisterKinds rk);
   Register(TR_RegisterKinds rk, uint16_t ar);

   public:

   TR::Register* self();

   TR_ALLOC(TR_Memory::Register)

   /*
    * Getter/Setters for private fields
    */
   TR_BackingStore *getBackingStorage()                     { return _backingStorage; }
   TR_BackingStore *setBackingStorage(TR_BackingStore *bs)  { return (_backingStorage = bs); }

   TR::AutomaticSymbol *getPinningArrayPointer()                      {return _pinningArrayPointer;}
   TR::AutomaticSymbol *setPinningArrayPointer(TR::AutomaticSymbol *s) {return (_pinningArrayPointer = s);}

   TR::Register *getAssignedRegister()               {return _assignedRegister;}
   TR::Register *setAssignedRegister(TR::Register *r) {return (_assignedRegister = r);}

   TR::Register *getColouredRegister()               {return _colouredRegister;}
   TR::Register *setColouredRegister(TR::Register *r) {return (_colouredRegister = r);}

   TR::Register *getSiblingRegister()               {return _siblingRegister;}
   TR::Register *setSiblingRegister(TR::Register *r) {return (_siblingRegister = r);}

   TR::Instruction *getStartOfRange()                  {return _startOfRange;}
   TR::Instruction *setStartOfRange(TR::Instruction *i) {return _startOfRange = i;}
   TR::Instruction *getEndOfRange()                    {return _endOfRange;}
   TR::Instruction *setEndOfRange(TR::Instruction *i)   {return _endOfRange = i;}
   TR::Node *getStartOfRangeNode()                  {return _startOfRangeNode;}
   TR::Node *setStartOfRangeNode(TR::Node *n) { /* _startOfRange=NULL; */ return _startOfRangeNode = n;}

   ncount_t getTotalUseCount();
   ncount_t setTotalUseCount(ncount_t tuc);
   ncount_t incTotalUseCount(ncount_t tuc=1);
   ncount_t decTotalUseCount(ncount_t tuc=1);

   ncount_t getFutureUseCount() {return _futureUseCount;}
   ncount_t setFutureUseCount(ncount_t fuc) {return (_futureUseCount = fuc);}
   ncount_t incFutureUseCount(ncount_t fuc=1) {TR_ASSERT(_futureUseCount <= MAX_NODE_COUNT-fuc, "TR::Register::_futureUseCount == MAX_NODE_COUNT"); return (_futureUseCount += fuc);}
   ncount_t decFutureUseCount(ncount_t fuc=1) { return (_futureUseCount -= fuc);}

   ncount_t getOutOfLineUseCount() { return _outOfLineUseCount; }
   ncount_t setOutOfLineUseCount(ncount_t uc) { return (_outOfLineUseCount = uc); }
   ncount_t incOutOfLineUseCount(ncount_t uc=1) { return (_outOfLineUseCount +=uc); }
   ncount_t decOutOfLineUseCount(ncount_t uc=1) { return (_outOfLineUseCount -= uc); }

   uint32_t getAssociation()           {return _association;}
   uint32_t setAssociation(uint32_t a) {return (_association = a);}

   TR_RegisterKinds getKind()                    {return _kind;}
   TR_RegisterKinds setKind(TR_RegisterKinds rk) {return (_kind = rk);}
   TR_RegisterKinds getKindAsMask()              {return TR_RegisterKinds(1<<_kind);}

   uint32_t getIndex()           { return _index; }
   void     setIndex(uint32_t i) { _index=i; }

   TR::Register *getCCRegister() { return _cc; }
   void setCCRegister(TR::Register *s) { _cc = s; }


   /*
    * Get/Set Flag Value
    */
   uint32_t getFlags()          {return _flags.getValue();}
   void    setFlags(uint32_t f) {_flags.setValue(0xffffffff,f);}


   /*
    * Methods for getting/setting flag masks
    */
   bool isPlaceholderReg()  {return _flags.testAny(PlaceholderReg);}
   void setPlaceholderReg()  {  _flags.set(PlaceholderReg); }
   void resetPlaceholderReg() {_flags.reset(PlaceholderReg);}

   bool containsCollectedReference()      { return _flags.testAny(ContainsCollectedReference); }
   void setContainsCollectedReference()  {  _flags.set(ContainsCollectedReference); }

   bool isLive()      {return _flags.testAny(IsLive);}
   void setIsLive()   {_flags.set(IsLive);}
   void resetIsLive() {_flags.reset(IsLive); }

   bool containsInternalPointer()    {return _flags.testAny(ContainsInternalPointer);}
   void setContainsInternalPointer();

   bool isSinglePrecision()                 {return _flags.testAny(IsSinglePrecision);}
   void setIsSinglePrecision(bool b = true) {_flags.set(IsSinglePrecision, b);}

   bool isUpperHalfDead()                   {return _flags.testAny(UpperHalfIsDead);}
   void setIsUpperHalfDead(bool b = true)   {_flags.set(UpperHalfIsDead, b);}

   /*
    * Methods for getting real reg, reg pairs etc in subclasses.
    * return NULL in base class and return THIS ptr in subclasses
    * but in some cases, base can also return THIS
    */

   TR::RealRegister *getAssignedRealRegister();
   virtual TR::Register    *getRegister();

   virtual TR::Register    *getLowOrder()   {return NULL;}
   virtual TR::Register    *getHighOrder()  {return NULL;}

   virtual TR::RegisterPair    *getRegisterPair()   {return NULL;}
   virtual TR::RealRegister    *getRealRegister()   {return NULL;}

   virtual const char         *getRegisterName(TR::Compilation *comp, TR_RegisterSizes size = TR_WordReg);
   static const char          *getRegisterKindName(TR::Compilation *comp, TR_RegisterKinds rk);


   virtual void block();
   virtual void unblock();


#if defined(DEBUG)
   virtual void print(TR::Compilation *comp, TR::FILE *pOutFile, TR_RegisterSizes size = TR_WordReg);
#endif


   protected:

   flags32_t _flags;

   enum // _flags masks
      {
      PlaceholderReg                = 0x0001,
      ContainsCollectedReference    = 0x0008, // GPR contains a collected reference
      IsLive                        = 0x0010, // Register is currently live
      ContainsInternalPointer       = 0x0080,
      IsSinglePrecision             = 0x0400,
      UpperHalfIsDead               = 0x0400, // GPRS ONLY -- no need to save/restore the high-order bits when spilled; aliased with IsSinglePrecision
      };


   private:

   TR_BackingStore          *_backingStorage;        // location where register is spilled if spilled
   TR::AutomaticSymbol       *_pinningArrayPointer;   // pinning array object if containing internal ptr

   TR::Register              *_assignedRegister;      // register to which this register is assigned
   TR::Register              *_colouredRegister;      // register to which this register has been coloured to
   TR::Register              *_siblingRegister;       // Sibling to a register pair

   TR::Instruction   *_startOfRange;    // start of live range
   TR::Instruction   *_endOfRange;      // end of live range
   TR::Node           *_startOfRangeNode;// Node of _startOfRange. Needed if start of range is Register load

   ncount_t          _totalUseCount;   // holds the number of references to the register
   ncount_t          _futureUseCount;  // used by register assigner to keep track of how many uses are left to assign
   ncount_t          _outOfLineUseCount;   // holds the number of references to the register in out of line code sections

   // used by register assigner to predispose virtuals to a particular real register so that they tend
   // to end up being in the right register for instructions that care (like div or call)
   uint16_t         _association;

   TR_RegisterKinds _kind;
   uint32_t         _index;             // index into register table

   // Holds the condition codes produced by the operation that produced the container register.
   TR::Register     *_cc;
   };

}

#endif /* OMR_REGISTER_INCL */
