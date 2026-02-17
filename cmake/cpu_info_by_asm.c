/*  Copyright (c) 2026, Alliance for Open Media
    All rights reserved. */
/*
    Redistribution and use in source and binary forms, with or without
    modification, are permitted (subject to the limitations in the
    disclaimer below) provided that the following conditions are met:

    - Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

    - Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

    - Neither the name of the Alliance for Open Media nor the names of
    its contributors may be used to endorse or promote products derived
    from this software without specific prior written permission.

    NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
    GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
    HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
    WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
    OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
    BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
    OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
    OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
    BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
    LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
    NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <stdio.h>
int main() {
    unsigned int CPUInfo0;
    unsigned int CPUInfo1;
    unsigned int CPUInfo2;
    unsigned int CPUInfo3;
    unsigned int InfoType;
#if defined(__i386__) && defined(__PIC__)
/* %ebx is PIC register in 32-bit, so mustn't clobber it. */
    __asm__ __volatile__ (
        "xchg %%ebx, %1\n"
        "cpuid\n"
        "xchg %%ebx, %1\n" :
        "=a" (CPUInfo0),
        "=r" (CPUInfo1),
        "=c" (CPUInfo2),
        "=d" (CPUInfo3) :
        "0" (InfoType), "2" (0)
        );
#else
    __asm__ __volatile__ (
        "cpuid" :
        "=a" (CPUInfo0),
        "=b" (CPUInfo1),
        "=c" (CPUInfo2),
        "=d" (CPUInfo3) :
        "0" (InfoType), "2" (0)
        );
#endif
    return 0;
}
