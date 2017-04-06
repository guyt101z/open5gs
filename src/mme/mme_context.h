#ifndef __MME_CONTEXT__
#define __MME_CONTEXT__

#include "core_list.h"
#include "core_errno.h"
#include "core_net.h"
#include "core_sha2.h"
#include "core_hash.h"

#include "3gpp_common.h"
#include "nas_types.h"
#include "gtp_xact.h"

#include "mme_sm.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define MAX_PLMN_ID                 6
#define GRP_PER_MME                 256    /* According to spec it is 65535 */
#define CODE_PER_MME                256    /* According to spec it is 256 */

typedef struct _served_gummei {
    c_uint32_t      num_of_plmn_id;
    plmn_id_t       plmn_id[MAX_PLMN_ID];

    c_uint32_t      num_of_mme_gid;
    c_uint16_t      mme_gid[GRP_PER_MME];
    c_uint32_t      num_of_mme_code;
    c_uint8_t       mme_code[CODE_PER_MME];
} srvd_gummei_t;

typedef struct _mme_context_t {
    c_uint32_t      s1ap_addr;  /* MME S1AP local address */
    c_uint16_t      s1ap_port;  /* MME S1AP local port */
    net_sock_t      *s1ap_sock; /* MME S1AP local listen socket */

    c_uint32_t      s11_addr;   /* MME S11 local address */
    c_uint16_t      s11_port;   /* MME S11 local port */
    net_sock_t      *s11_sock;  /* MME S11 local listen socket */

    msgq_id         queue_id;       /* Queue for processing MME control plane */
    tm_service_t    tm_service;     /* Timer Service */
    gtp_xact_ctx_t  gtp_xact_ctx;   /* GTP Transaction Context for MME */

    c_uint32_t      mme_ue_s1ap_id; /** mme_ue_s1ap_id generator */
    plmn_id_t       plmn_id;

    /* defined in 'nas_ies.h'
     * #define NAS_SECURITY_ALGORITHMS_EIA0        0
     * #define NAS_SECURITY_ALGORITHMS_128_EEA1    1
     * #define NAS_SECURITY_ALGORITHMS_128_EEA2    2
     * #define NAS_SECURITY_ALGORITHMS_128_EEA3    3 */
    c_uint8_t       selected_enc_algorithm;
    /* defined in 'nas_ies.h'
     * #define NAS_SECURITY_ALGORITHMS_EIA0        0
     * #define NAS_SECURITY_ALGORITHMS_128_EIA1    1
     * #define NAS_SECURITY_ALGORITHMS_128_EIA1    2
     * #define NAS_SECURITY_ALGORITHMS_128_EIA3    3 */
    c_uint8_t       selected_int_algorithm;

    /* S1SetupRequest */
    c_uint16_t      tracking_area_code;
    c_uint16_t      default_paging_drx;

    /* S1SetupResponse */
    srvd_gummei_t   srvd_gummei;
    c_uint8_t       relative_capacity;

    list_t          sgw_list;
    list_t          enb_list;

    hash_t          *mme_ue_s1ap_id_hash; /* hash table for MME-UE-S1AP-ID */
} mme_context_t;

typedef struct _mme_sgw_t {
    gtp_node_t      gnode; /* SGW S11 remote GTPv2-C node */

} mme_sgw_t;

typedef struct _mme_enb_t {
    lnode_t         node; /**< A node of list_t */

    c_uint32_t      enb_id; /** eNB_ID received from eNB */

    s1ap_sm_t       s1ap_sm;
    net_sock_t      *s1ap_sock;

    list_t          ue_list;

} mme_enb_t;

typedef struct _mme_ue_t {
    lnode_t         node; /**< A node of list_t */

    /* State Machine */
    emm_sm_t        emm_sm;
    esm_sm_t        esm_sm;

    /* UE identity */
    c_uint32_t      enb_ue_s1ap_id; /** eNB-UE-S1AP-ID received from eNB */
    c_uint32_t      mme_ue_s1ap_id; /** MME-UE-S1AP-ID received from MME */
    c_uint8_t       imsi[MAX_IMSI_LEN+1];
    c_uint8_t       imsi_len;

    /* Security Context */
    int             security_context_available;
    nas_ue_network_capability_t ue_network_capability;
    nas_ms_network_capability_t ms_network_capability;
    c_uint8_t       xres[MAX_RES_LEN];
    c_uint8_t       xres_len;
    c_uint8_t       kasme[SHA256_DIGEST_SIZE];
    c_uint8_t       knas_int[SHA256_DIGEST_SIZE/2]; 
    c_uint8_t       knas_enc[SHA256_DIGEST_SIZE/2];
    c_uint32_t      dl_count;
    union {
        struct {
        ED3(c_uint8_t spare;,
            c_uint16_t overflow;,
            c_uint8_t sqn;)
        } __attribute__ ((packed));
        c_uint32_t i32;
    } ul_count;

    mme_enb_t       *enb;
} mme_ue_t;

CORE_DECLARE(status_t)      mme_context_init(void);
CORE_DECLARE(status_t)      mme_context_final(void);
CORE_DECLARE(mme_context_t*) mme_self(void);

CORE_DECLARE(mme_sgw_t*)    mme_sgw_add(void);
CORE_DECLARE(status_t)      mme_sgw_remove(mme_sgw_t *sgw);
CORE_DECLARE(status_t)      mme_sgw_remove_all(void);
CORE_DECLARE(mme_sgw_t*)    mme_sgw_find_by_node(gtp_node_t *gnode);
CORE_DECLARE(mme_sgw_t*)    mme_sgw_first(void);
CORE_DECLARE(mme_sgw_t*)    mme_sgw_next(mme_sgw_t *sgw);

CORE_DECLARE(mme_enb_t*)    mme_enb_add(void);
CORE_DECLARE(status_t)      mme_enb_remove(mme_enb_t *enb);
CORE_DECLARE(status_t)      mme_enb_remove_all(void);
CORE_DECLARE(mme_enb_t*)    mme_enb_find_by_sock(net_sock_t *sock);
CORE_DECLARE(mme_enb_t*)    mme_enb_find_by_enb_id(c_uint32_t enb_id);
CORE_DECLARE(mme_enb_t*)    mme_enb_first(void);
CORE_DECLARE(mme_enb_t*)    mme_enb_next(mme_enb_t *enb);

CORE_DECLARE(mme_ue_t*)     mme_ue_add(mme_enb_t *enb);
CORE_DECLARE(status_t)      mme_ue_remove(mme_ue_t *ue);
CORE_DECLARE(status_t)      mme_ue_remove_all();
CORE_DECLARE(mme_ue_t*)     mme_ue_find_by_mme_ue_s1ap_id(
                                c_uint32_t mme_ue_s1ap_id);
CORE_DECLARE(hash_index_t *) mme_ue_first();
CORE_DECLARE(hash_index_t *) mme_ue_next(hash_index_t *hi);
CORE_DECLARE(mme_ue_t *)    mme_ue_this(hash_index_t *hi);
CORE_DECLARE(unsigned int)  mme_ue_count();
CORE_DECLARE(status_t)      mme_ue_remove_in_enb(mme_enb_t *enb);
CORE_DECLARE(mme_ue_t*)     mme_ue_find_by_enb_ue_s1ap_id(
                                mme_enb_t *enb, c_uint32_t enb_ue_s1ap_id);
CORE_DECLARE(mme_ue_t*)     mme_ue_first_in_enb(mme_enb_t *enb);
CORE_DECLARE(mme_ue_t*)     mme_ue_next_in_enb(mme_ue_t *ue);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __MME_CONTEXT__ */