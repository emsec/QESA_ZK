#pragma once

// Should QESA/LMPA/ShuffleProof use small exponents when appropriate?
#define USE_SMALL_EXPONENTS false
// size of small exponents in bit
#define SMALL_EXP_SIZE 140

// Should there be an IPA used in Bulletproofs, if yes, which:
#define USE_BULLETPROOFS_IPA false
#define USE_QESA_IPA true

#if USE_QESA_IPA && USE_BULLETPROOFS_IPA
    #error Either USE_QESA_IPA or USE_BULLETPROOFS_IPA can be true, but not both
#endif
