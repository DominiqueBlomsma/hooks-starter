#include "hookapi.h"
#include <stdint.h>

#define DEBUG 1

#define SVAR(x) &x, sizeof(x)

#define DONE(msg)\
    accept(msg, sizeof(msg),__LINE__)


uint8_t dest_key[3] = { 'H', 'D', 'E' }; //484445
uint8_t dtag_key[3] = { 'H', 'D', 'T' }; //484445
uint8_t amt_key[3]  = { 'H', 'A', 'M' }; //48414D

int64_t hook(uint32_t r)
{
    _g(1,1);

    // pass anything that isn't a ttINVOKE
    if (otxn_type() != 99)
        DONE("High value: Passing non-Invoke txn");

    uint8_t hook_acc[20];
    uint8_t req_acc[20];

    // get the account id
    otxn_field(req_acc, 20, sfAccount);
    hook_account(hook_acc, 20);

    // is the account is the sender?
    if (!BUFFER_EQUAL_20(hook_acc, req_acc))
        DONE("High value: Ignoring non self-Invoke");


    /*
     * Packed state data:
     *   0-19 : destination acc
     *  20-20 : has dtag
     *  21-24 : dtag if any
     *  25-73 : amount tl/drops
     */
    
    #define DEST (data + 0)
    #define DTAG (data + 21)
    #define AMT (data + 25)

    uint8_t data[73];

    int64_t has_dest = otxn_param(DEST, 20, SBUF(dest_key)) == 20;
    uint8_t has_dtag = otxn_param(DTAG, 4, SBUF(dtag_key)) == 4;
    int64_t amt_size = otxn_param(AMT, 48,  SBUF(amt_key ));
    int64_t has_amt = amt_size == 8 || amt_size == 48;

    if (has_dest && has_amt)
    {
        // pass
    }
    else if (has_dest)
        rollback(SBUF("High value: amount param missing (HAM)"), __LINE__);
    else
        rollback(SBUF("High value: dest param missing (HDE)"), __LINE__);


   *(data + 20) = has_dtag;
    
    uint8_t hash[32];
    util_sha512h(SBUF(hash), SBUF(data));

    int64_t lgr = ledger_seq();
    if (state_set(&lgr, sizeof(lgr), SBUF(hash)) == sizeof(lgr))
        DONE("High value: ready for txn");

    return rollback(SBUF("High value: could not set state (low reserve?)"), __LINE__);
}
