#include <stdio.h>

#include <libdnas2.h>

int sceDNAS2InitNoHDD(sceDNAS2TitleAuthData_t *pAuthData,
                      int priority,
                      int debugFlag,
                      int sslFlag,
                      sceDNAS2TimeoutInfo_t *pTimeout) {
    (void)pAuthData;
    (void)priority;
    (void)debugFlag;
    (void)sslFlag;
    (void)pTimeout;
    printf("fixme:dnas2:sceDNAS2InitNoHDD stub\n");
    return sceDNAS2_ERR_NOT_INITIALIZED;
}

int sceDNAS2AuthNetStart(void) {
    printf("fixme:dnas2:sceDNAS2AuthNetStart stub\n");
    return sceDNAS2_ERR_NOT_INITIALIZED;
}

int sceDNAS2Abort(void) {
    printf("fixme:dnas2:sceDNAS2Abort stub\n");
    return sceDNAS2_ERR_NOT_INITIALIZED;
}

int sceDNAS2GetStatus(sceDNAS2Status_t *pStatus) {
    (void)pStatus;
    printf("fixme:dnas2:sceDNAS2GetStatus stub\n");
    return sceDNAS2_ERR_NOT_INITIALIZED;
}
