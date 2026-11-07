/** 
 * to be included in exception.c
 */


/************************************************************************/
p4_Except *catchframe = NULL;  /* links to chain of CATCHed words */
        /* and no exceptions to be caught */
/************************************************************************/

/**
 * the CATCH impl
 */
int p4_catch (p4xt xt)
{
    register int returnvalue;
    auto p4_Except frame;

    frame.magic = P4_EXCEPTION_MAGIC;
    frame.ipp = IP;
    frame.spp = SP;
#if defined PF_WITH_FLOATING
    frame.fpp = FP;
#endif
    frame.rpp = RP;
    frame.prev = catchframe;  catchframe = &frame;
    returnvalue = setjmp (frame.jmp);
    if (! returnvalue) {
        pf_call (xt);
    }
    catchframe = frame.prev;
    RP = frame.rpp;
    return returnvalue;
}

/** CATCH ( catch-xt* -- 0 | throw#! ) [ANS]
 * execute the given execution-token and catch
 * any exception that can be caught therein.
 * software can arbitrarily raise an exception
 * using => THROW - the value 0 means there
 * was no exception, other denote implementation
 * dependent exception-codes.
 */
FCode (p4_catch)
{
    p4cell catch_code = p4_catch ((p4xt) *SP++);
    *--SP = catch_code;
}

