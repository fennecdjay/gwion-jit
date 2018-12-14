#include <string.h>
#include <jit/jit.h>
#include "gwjit_common.h"
#include "jitter.h"
#include "cc.h"
#include "sig.h"
#include "code.h"
#include "code/func.h"

static inline int trig_ex(const UGen u, const VM_Shred shred) {
  if(!u->module.gen.trig) {
    release_connect(shred);
    exception(shred, "NonTriggerException");
    return 1;
  }
  return 0;
}

#define JIT_TRIG_EX                      \
  CJval trig_arg[] = { ru, cc->shred };  \
  CALL_NATIVE2(trig_ex, "ipp", trig_arg); \

#define jit_describe_connect_instr(name, func, opt) \
JIT_CODE(name##func) { \
  push_reg(cc, -SZ_INT*2);\
  CJval lhs = JLOADR(cc->reg, 0, void_ptr);\
  CJval rhs = JLOADR(cc->reg, SZ_INT, void_ptr);\
  INIT_LABEL(end);\
  JINSN(branch_if_not, lhs, &end);\
  CJval ldata = JLOADR(lhs, JOFF(M_Object, data), void_ptr); \
  CJval lu = JLOADR(ldata, 0, void_ptr); \
  CJval rdata = JLOADR(rhs, JOFF(M_Object, data), void_ptr);\
  CJval ru = JLOADR(rdata, 0, void_ptr); \
  opt \
  CJval f = JCONST(void_ptr, func);\
  CJval arg3[] = { f, lu, ru }; \
  CALL_NATIVE2(_do_, "vppp", arg3); \
  JINSN(label, &end);\
  JSTORER(cc->reg, 0, rhs);\
  cc_release2(cc, lhs); \
  cc_release2(cc, rhs);\
  push_reg(cc, SZ_INT);\
}
jit_describe_connect_instr(Ugen, Connect,)
jit_describe_connect_instr(Ugen, Disconnect,)
jit_describe_connect_instr(Trig, Connect, JIT_TRIG_EX)
jit_describe_connect_instr(Trig, Disconnect, JIT_TRIG_EX)

#define JIT_IMPORT(a) jit_code_import(j, a, jitcode_##a);
ANN void jit_code_import_ugen(struct Jit* j) {
  JIT_IMPORT(UgenConnect);
  JIT_IMPORT(UgenDisconnect);
  JIT_IMPORT(TrigConnect);
  JIT_IMPORT(TrigDisconnect);
}
