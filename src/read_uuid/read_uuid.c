#include <stdio.h>

#ifdef PLATFORM_MIYOOMINI
#include "mi_common_datatype.h"
#include "mi_sys.h"
#endif

int main(void)
{
#ifdef PLATFORM_MIYOOMINI

    MI_U64 u64Uuid;
    MI_S32 s32Ret = MI_ERR_SYS_FAILED;

    if (!(s32Ret = MI_SYS_ReadUuid(&u64Uuid)))
        printf("%llx\n", u64Uuid);

#endif

    return 0;
}
