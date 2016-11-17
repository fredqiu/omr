/*******************************************************************************
 *
 * (c) Copyright IBM Corp. 2016, 2016
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
 ******************************************************************************/


#include <iostream>
#include <stdlib.h>
#include <stdint.h>
#include <dlfcn.h>
#include <errno.h>

#include "Jit.hpp"
#include "ilgen/TypeDictionary.hpp"
#include "ilgen/MethodBuilder.hpp"
#include "Simple.hpp"

using std::cout;
using std::cerr;

static int32_t
doublesum(int32_t first, int32_t second)
   {   
   #define DOUBLESUM_LINE LINETOSTR(__LINE__)
   int32_t result = 2 * (first + second);
   fprintf(stderr,"doublesum(%d, %d) == %d\n", first, second, result);
   return result;
   }   



int
main(int argc, char *argv[])
   {
   cout << "Step 1: initialize JIT\n";
   bool initialized = initializeJit();
   if (!initialized)
      {
      cerr << "FAIL: could not initialize JIT\n";
      exit(-1);
      }

   cout << "Step 2: define type dictionary\n";
   TR::TypeDictionary types;

   cout << "Step 3: compile method builder\n";
   SimpleMethod method(&types);
   uint8_t *entry = 0;
   int32_t rc = compileMethodBuilder(&method, &entry);
   if (rc != 0)
      {
      cerr << "FAIL: compilation error " << rc << "\n";
      exit(-2);
      }

   cout << "Step 4: invoke compiled code and print results\n";
   typedef int32_t (SimpleMethodFunction)(int32_t*);
   SimpleMethodFunction *increment = (SimpleMethodFunction *) entry;

   int32_t v;
   v=0;   cout << "increment(" << v; cout << ") == " << increment(&v) << "\n";
   v=1;   cout << "increment(" << v; cout << ") == " << increment(&v) << "\n";
   v=10;  cout << "increment(" << v; cout << ") == " << increment(&v) << "\n";
   v=-15; cout << "increment(" << v; cout << ") == " << increment(&v) << "\n";

   cout << "Step 5: shutdown JIT\n";
   shutdownJit();
   }



SimpleMethod::SimpleMethod(TR::TypeDictionary *d)
   : MethodBuilder(d)
   {
   DefineLine(LINETOSTR(__LINE__));
   DefineFile(__FILE__);

   DefineName("increment");
   pInt32=d->PointerTo(Int32);
   DefineParameter("addressOfValue", pInt32);
   DefineReturnType(Int32);
   
   DefineFunction((char *)"doublesum",
              (char *)__FILE__,
              (char *)DOUBLESUM_LINE,
              (void *)&doublesum,
              Int32,
              2,
              Int32,
              Int32);

	}

bool
SimpleMethod::buildIL()
   {
   cout << "SimpleMethod::buildIL() running!\n";

   TR::IlBuilder *persistent=NULL;
   TR::IlBuilder *transient=NULL;
   TR::IlBuilder *fallthrough=NULL; 
   TransactionBegin(&persistent, &transient, &fallthrough);
  
   persistent->AtomicIntegerAdd(
           persistent->IndexAt(pInt32, persistent->Load("addressOfValue"), persistent->ConstInt32(0)),
           persistent->ConstInt32(1));
   transient->Goto(&persistent);
   fallthrough->StoreAt(fallthrough->IndexAt(pInt32,fallthrough->Load("addressOfValue"), fallthrough->ConstInt32(0)),
                        fallthrough->Add(fallthrough->LoadAt(pInt32, fallthrough->IndexAt(pInt32, fallthrough->Load("addressOfValue"), fallthrough->ConstInt32(0))),
                                         fallthrough->ConstInt32(1)));

   fallthrough->TransactionEnd();
   Return(LoadAt(pInt32, Load("addressOfValue")));

   return true;
   }

