#include <uusb.h>

/* Imported from Petari (github.com/SMGCommunity/Petari, CC0), src/RVL_SDK/bte/uusb_ppc.c.
 *
 * Changed for this version (Wii Play, RHAE01): this game links a later uusb_ppc.c than
 * Petari's. It traces through OSReport when usb.trace_state is set (the "HCISU_LOG"/"HCISU_ERR"
 * messages), and it has the uusb_issue_bulk_read and uusb_get_devId functions. The functions
 * were rewritten from this game's code.
 */

/*******************************************************************************
 * headers
 */

#include <cstring>
#include <mem.h>

#include "decomp.h"
#include "macros.h"

#include <data_types.h>
#include <gki_target.h>

#include <bte.h>
#include <gki.h>
#include <hci.h>
#include <l2c_int.h>

#include "revolution/types.h"

#define IS_BTE
#include "context_rvl.h"

/*******************************************************************************
 * macros
 */

#undef NULL
#define NULL ((void*)(0))

#define UUSB_TRACE(...)                                                                                                                              \
    do {                                                                                                                                             \
        if (usb.trace_state)                                                                                                                         \
            OSReport(__VA_ARGS__);                                                                                                                   \
    } while (FALSE)

#define UUSB_PPC_CLOSED_ST     0
#define UUSB_PPC_OPENED_ST     2
#define UUSB_PPC_REGISTERED_ST 4
#define UUSB_PPC_INIT_ST       5

#define UUSB_VENDOR_ID  0x057E  // 1406 - Nintendo Co., Ltd.
#define UUSB_PRODUCT_ID 0x0305  //  773 - Broadcom BCM2045A Bluetooth Radio [Nintendo Wii/Wii U]

#define UUSB_MAX_WRITES_OUTSTANDING 5

/*******************************************************************************
 * types
 */

typedef struct {
    long fd;  // TOOD: IOSFd
    char pad0_[0x10 - (0x00 + 0x04)];
    unk1_t unsigned bulkEndpoint1;
    unk1_t unsigned bulkEndpoint2;
    unk1_t unsigned intrEndpoint1;
    unk1_t at_0x13;  // intrEndpoint2?
    unk4_t vid;
    unk4_t pid;
    UINT8 cmd_buffer_pool;
    UINT8 acl_buffer_pool;
    char pad2_[0x20 - (0x1d + 0x01)];
    tUUSB_CBACK* cback;
    tUUSB* p_uusb_cfg;
    UINT8 trace_state;
    UINT8 state;
    char pad4_[1];
    UINT8 reading_intr_data;
    BUFFER_Q bulk_buffer_q;
    UINT8 at_0x38;
    BUFFER_Q intr_buffer_q;
    UINT8 at_0x48;
    char pad8_[3];
} tUUSB_CB;

/*******************************************************************************
 * local function declarations
 */

static void uusb_ReleaseCmdBufferPool(void);
static void uusb_ReleaseAclBufferPool(void);
void uusb_CloseDeviceCB(IPCResult result, void* p_data);
void uusb_ReadIntrDataCB(IPCResult result, void* p_data);
void uusb_ReadBulkDataCB(IPCResult result, void* p_data);
void uusb_issue_bulk_read(void);
void uusb_issue_intr_read(void);
void uusb_WriteCtrlDataCB(IPCResult result, void* p_data);
void uusb_WriteBulkDataCB(IPCResult result, void* p_data);
long uusb_get_devId(int vid, int pid);

/*******************************************************************************
 * variables
 */

// .bss
static tUUSB_CB usb;
unsigned char __uusb_ppc_stack1[0x1000] ATTRIBUTE_ALIGN(32);
unsigned char __uusb_ppc_stack2[0x1000] ATTRIBUTE_ALIGN(32);

// .sdata
// TODO: IOSFd, IOS_INVALID_FD
UINT32 volatile wait4hci = TRUE;
static long __ntd_ios_file_descriptor = -1l;

// .sbss
static int __ntd_pid;
static int __ntd_vid;
static BOOLEAN __ntd_pid_vid_specified;
static UINT8 uusb_g_usb_devid_found = FALSE;
static UINT8 uusb_g_trace_state_initialized = FALSE;
UINT32 __ntd_ohci = 0;
UINT32 __ntd_ohci_init_flag = FALSE;

/*******************************************************************************
 * functions
 */

void HCI_TRACE(char const* p_msg) {
    /* ... */
}

void HCI_TRACE1(char const* p_msg, UINT32 arg1) {
    /* ... */
}

void __ntd_set_ohci(UINT8 ohci) {
    if (ohci == 0) {
        __ntd_ohci = 0;
        __ntd_ohci_init_flag = TRUE;
    } else if (ohci == 1) {
        __ntd_ohci = 1;
        __ntd_ohci_init_flag = TRUE;
    }
}

void __ntd_set_pid_vid(int pid, int vid) {
    __ntd_vid = vid;
    __ntd_pid = pid;
    __ntd_pid_vid_specified = TRUE;
}

int __ntd_get_usb_file_descriptor(void) {
    return __ntd_ios_file_descriptor;
}

UINT32 __ntd_get_allocated_mem_size(void) {
    UINT32 size = 0;

    // TODO: what are these the sizes of?
    size += 0x07620;
    size += 0x10000;
    size -= 0x02BA8;

    return size;
}

void uusb_set_trace_state(UINT8 state) {
    usb.trace_state = state;

    uusb_g_trace_state_initialized = TRUE;
}

UINT8 uusb_get_trace_state(void) {
    return usb.trace_state;
}

static void uusb_ReleaseCmdBufferPool(void) {
    GKI_delete_pool(usb.cmd_buffer_pool);

    usb.cmd_buffer_pool = GKI_INVALID_POOL;
}

static void uusb_ReleaseAclBufferPool(void) {
    GKI_delete_pool(usb.acl_buffer_pool);

    usb.acl_buffer_pool = GKI_INVALID_POOL;
}

void uusb_CloseDeviceCB(IPCResult result, void* p_data) {
    usb.fd = 0;

    usb.bulkEndpoint1 = 0;
    usb.bulkEndpoint2 = 0;
    usb.intrEndpoint1 = 0;
    usb.at_0x13 = 0;

    uusb_g_usb_devid_found = 0;

    UUSB_Unregister();

    if (usb.cback)
        (*usb.cback)(4, result);
}

void uusb_ReadIntrDataCB(IPCResult result, void* p_data) {
    UINT32 len;
    HC_BT_HDR* p_buffer;
    HC_BT_HDR* p_buf = p_data;
    UINT8* p;
    int i;

    UUSB_TRACE("HCISU_LOG: uusb_ReadIntrDataCB called\n");

    usb.reading_intr_data = FALSE;

    if (usb.state != UUSB_PPC_OPENED_ST) {
        UUSB_TRACE("HCISU_ERR: uusb_ReadIntrDataCB: usb_state != UUSB_PPC_OPENED_ST\n");
        UUSB_TRACE("HCISU_ERR: ************\n* uusb_ReadIntrDataCB(): usb_state != UUSB_PPC_OPENED_ST - stop reading\n************\n");

        GKI_freebuf(p_buf);

        uusb_ReleaseCmdBufferPool();

        return;
    }

    if (p_buf == NULL)
        goto end;

    if (result <= 0) {
        UUSB_TRACE("HCISU_ERR: uusb_ReadIntrDataCB(): Got Error code %d\n", result);

        GKI_freebuf(p_buf);
        goto end;
    }

    p_buffer = GKI_getpoolbuf(GKI_POOL_ID_2);
    if (!p_buffer) {
        GKI_freebuf(p_buf);
        goto end;
    }

    p_buf->event = 0x1000;
    p_buf->len = result;

    len = sizeof *p_buf + p_buf->len + p_buf->offset;
    len = (len >> 2 << 2) + 4;  // round up to 4
    memcpy(p_buffer, p_buf, len);

    UUSB_TRACE("HCISU_LOG: uusb_ReadIntrDataCB: len(%d) offset (%d) cp_len(%d) data = ", p_buffer->len, p_buffer->offset, len);

    p = (UINT8*)(p_buffer + 1) + p_buffer->offset;
    for (i = 0; i < result; i++)
        UUSB_TRACE("%02x ", p[i]);

    UUSB_TRACE("\n");

    OSSwitchFiberEx((unsigned long)p_buffer, 0, 0, 0, (void*)&bta_ci_hci_msg_handler, (void*)(__uusb_ppc_stack1 + 4096));

    GKI_freebuf(p_buf);

end:
    uusb_issue_intr_read();
}

void uusb_ReadBulkDataCB(IPCResult result, void* p_data) {
    UINT32 len;
    BT_HDR* p_buffer;
    HC_BT_HDR* p_buf = p_data;
    UINT8* p;
    int i;

    if (usb.state != UUSB_PPC_OPENED_ST) {
        UUSB_TRACE("HCISU_ERR: uusb_readBulkDataCB: usb_state != UUSB_PPC_OPENED_ST\n");
        UUSB_TRACE("HCISU_ERR: ****\n* uusb_readBulkDataCB(): usb_state != UUSB_PPC_OPENED_ST - stop reading\n****\n");

        GKI_freebuf(p_buf);

        uusb_ReleaseAclBufferPool();

        return;
    }

    if (result <= 0) {
        UUSB_TRACE("HCISU_ERR: uusb_ReadBulkDataCB(): Got Error code %d\n", result);

        GKI_freebuf(p_buf);
        goto end;
    }

    p_buf->len = result;

    p_buffer = GKI_getpoolbuf(GKI_POOL_ID_3);
    if (!p_buffer) {
        GKI_freebuf(p_buf);
        goto end;
    }

    len = sizeof *p_buf + p_buf->len + p_buf->offset;
    len = (len >> 2 << 2) + 4;
    memcpy(p_buffer, p_buf, len);

    UUSB_TRACE("HCISU_LOG: uusb_ReadBulkDataCB: len(%d) offset(%d) cp_len(%d) data = ", p_buffer->len, p_buffer->offset, len);

    p = (UINT8*)(p_buffer + 1) + p_buffer->offset;
    for (i = 0; i < p_buffer->len; i++)
        UUSB_TRACE("%02x ", p[i]);

    UUSB_TRACE("\n");

    p_buffer = l2cap_link_chk_pkt_start(p_buffer);

    if (p_buffer != NULL && l2cap_link_chk_pkt_end())
        OSSwitchFiberEx((unsigned long)p_buffer, 0, 0, 0, (void*)&bta_ci_hci_msg_handler, (void*)(__uusb_ppc_stack2 + 4096));

    GKI_freebuf(p_buf);

end:
    uusb_issue_bulk_read();
}

void uusb_issue_bulk_read(void) {
    HC_BT_HDR* p_buf;
    void* p_buffer;
    IPCResult ret;

    while (TRUE) {
        p_buf = GKI_getpoolbuf(usb.acl_buffer_pool);
        if (p_buf != NULL)
            break;

        UUSB_TRACE("HCISU_ERR: ****\n* uusb_issue_bulk_read(): unable to get buffer - try again\n****\n");
    }

    p_buf->event = 0x1100;
    p_buf->len = 0;

    p_buffer = ROUND_UP_PTR((UINT8*)(p_buf + 1), 32);

    p_buf->offset = (UINT8*)p_buffer - (UINT8*)(p_buf + 1);

    // TODO: what is 0x28?
    ret = IUSB_ReadBlkMsgAsync(usb.fd, usb.bulkEndpoint2, GKI_get_buf_size(p_buf) - 0x28 - p_buf->offset, p_buffer, uusb_ReadBulkDataCB, p_buf);

    if (ret != 0) {
        UUSB_TRACE("HCISU_ERR: uusb_issue_bulk_read: IUSB_ReadBlkMsgAsync failed with err = %d\n", ret);
        UUSB_TRACE("HCISU_ERR: ****\n* uusb_issue_bulk_read: IUSB_ReadBlkMsgAsync failed with err = %d\n****\n", ret);

        GKI_freebuf(p_buf);
    }
}

void uusb_issue_intr_read(void) {
    HC_BT_HDR* p_buf;
    void* p_buffer;
    IPCResult ret;

    while (TRUE) {
        p_buf = GKI_getpoolbuf(usb.cmd_buffer_pool);
        if (p_buf != NULL)
            break;

        UUSB_TRACE("HCISU_ERR: ****\n* uusb_issue_intr_read(): unable to get buffer - try again\n****\n");
    }

    p_buf->event = 0x1000;
    p_buf->len = 0;

    p_buffer = ROUND_UP_PTR((UINT8*)(p_buf + 1), 32);

    p_buf->offset = (UINT8*)p_buffer - (UINT8*)(p_buf + 1);

    ret = IUSB_ReadIntrMsgAsync(usb.fd, usb.intrEndpoint1, GKI_get_buf_size(p_buf) - 0x28 - p_buf->offset, p_buffer, uusb_ReadIntrDataCB, p_buf);

    if (ret != 0) {
        UUSB_TRACE("HCISU_ERR: IUSB_ReadIntrMsgAsync failed with err = %d\n", ret);
        UUSB_TRACE("HCISU_ERR: ****\n* uusb_issue_intr_read(): IUSB_ReadIntrMsgAsync failed with err = %d\n****\n", ret);

        GKI_freebuf(p_buf);
    }

    usb.reading_intr_data = TRUE;
}

void uusb_WriteCtrlDataCB(IPCResult result, void* p_data) {
    HC_BT_HDR* p_buf;
    void* p_buffer;
    IPCResult ret;

    UUSB_TRACE("HCISU_LOG: uusb_WriteCtrlDataCB called with err = %d\n", result);

    if (result < 0)
        UUSB_TRACE("HCISU_ERR: uusb_WriteCtrlDataCB(): Got error (%d)\n", result);

    if (p_data != NULL) {
        GKI_freebuf(p_data);

        GKI_disable();

        --usb.at_0x48;

        GKI_enable();
    }

    if (usb.at_0x48 >= UUSB_MAX_WRITES_OUTSTANDING)
        return;

    if (usb.intr_buffer_q.count == 0)
        return;

    p_buf = GKI_dequeue(&usb.intr_buffer_q);

    p_buffer = (UINT8*)(p_buf + 1) + p_buf->offset;

    ret = IUSB_WriteCtrlMsgAsync(usb.fd, 0x20, 0, 0, 0, p_buf->len, p_buffer, uusb_WriteCtrlDataCB, p_buf);

    if (ret < 0) {
        UUSB_TRACE("HCISU_ERR: uusb_WriteCtrlDataCB(): error - unable to write data packet\n");

        GKI_enqueue_head(&usb.intr_buffer_q, p_buf);
    } else {
        GKI_disable();

        ++usb.at_0x48;

        GKI_enable();
    }
}

void uusb_WriteBulkDataCB(IPCResult result, void* p_data) {
    HC_BT_HDR* p_buf;
    void* p_buffer;
    IPCResult ret;

    UUSB_TRACE("HCISU_LOG: uusb_WriteBulkDataCB called with err = %d\n", result);

    if (result < 0)
        UUSB_TRACE("HCISU_ERR: uusb_WriteBulkDataCB(): Got error (%d)\n", result);

    if (p_data != NULL) {
        GKI_freebuf(p_data);

        GKI_disable();

        --usb.at_0x38;

        GKI_enable();
    }

    if (usb.at_0x38 >= UUSB_MAX_WRITES_OUTSTANDING)
        return;

    if (usb.bulk_buffer_q.count == 0)
        return;

    p_buf = GKI_dequeue(&usb.bulk_buffer_q);

    p_buffer = (UINT8*)(p_buf + 1) + p_buf->offset;
    ret = IUSB_WriteBlkMsgAsync(usb.fd, usb.bulkEndpoint1, p_buf->len, p_buffer, uusb_WriteBulkDataCB, p_buf);

    if (ret < 0) {
        UUSB_TRACE("HCISU_ERR: uusb_WriteBulkDataCB(): error - unable to write data packet\n");

        GKI_enqueue_head(&usb.bulk_buffer_q, p_buf);
    } else {
        GKI_disable();

        ++usb.at_0x38;

        GKI_enable();
    }
}

long uusb_get_devId(int vid, int pid) {
    IPCResult ret;

    if (__ntd_ohci_init_flag == TRUE) {
        if (__ntd_ohci == 0)
            ret = IUSB_OpenDeviceIds("oh0", vid, pid, (IPCResult*)&usb.fd);
        else if (__ntd_ohci == 1)
            ret = IUSB_OpenDeviceIds("oh1", vid, pid, (IPCResult*)&usb.fd);
    } else {
        ret = IUSB_OpenDeviceIds("oh1", vid, pid, (IPCResult*)&usb.fd);
    }

    if (ret < 0) {
        UUSB_TRACE("HCISU_ERR: ERROR from IUSB_OpenDeviceIds (%d) : Device (vendor 0x%x product 0x%x) not found\n", ret, UUSB_VENDOR_ID,
                   UUSB_PRODUCT_ID);
        UUSB_TRACE("HCISU_ERR: No suitable Bluetooth Device was found. Exiting...\n");
    } else {
        __ntd_ios_file_descriptor = ret;
    }

    return ret;
}

void UUSB_Register(tUUSB* uusb) {
    memset(&usb, 0, sizeof usb);

    GKI_disable();

    usb.state = UUSB_PPC_INIT_ST;

    GKI_enable();

    if (!uusb_g_trace_state_initialized)
        uusb_set_trace_state(0);

    if (IPCCltInit() != 0) {
        UUSB_TRACE("HCISU_ERR: UUSB_Register: ERROR - failed IPCCltInit\n");
        return;
    }

    if (IUSB_OpenLib() != 0) {
        UUSB_TRACE("HCISU_ERR: UUSB_Register: ERROR - failed IUSB_OpenLib\n");
        return;
    }

    usb.p_uusb_cfg = uusb;

    if (__ntd_pid_vid_specified == TRUE) {
        usb.vid = __ntd_vid;
        usb.pid = __ntd_pid;
    } else {
        usb.vid = UUSB_VENDOR_ID;
        usb.pid = UUSB_PRODUCT_ID;
    }

    usb.bulkEndpoint1 = 0;
    usb.bulkEndpoint2 = 0;
    usb.intrEndpoint1 = 0;
    usb.at_0x13 = 0;

    if (uusb_get_devId(usb.vid, usb.pid) < 0) {
        UUSB_TRACE("HCISU_ERR: UUSB_Register: Error returned from get_devId()\n");
        return;
    }

    UUSB_TRACE("HCISU_LOG: UUSB_Register: uusb_get_dev_id returned with usb.devId = %d\n", usb.fd);

    usb.bulkEndpoint1 = 2;
    usb.bulkEndpoint2 = 130;
    usb.intrEndpoint1 = 129;
    usb.at_0x13 = 0;

    GKI_init_q(&usb.bulk_buffer_q);
    usb.at_0x38 = 0;

    GKI_init_q(&usb.intr_buffer_q);
    usb.at_0x48 = 0;

    usb.cmd_buffer_pool = GKI_create_pool(660, 45, 1, NULL);
    usb.acl_buffer_pool = GKI_create_pool(1800, 30, 1, NULL);

    if (usb.cmd_buffer_pool == GKI_INVALID_POOL || usb.acl_buffer_pool == GKI_INVALID_POOL) {
        UUSB_TRACE("HCISU_LOG: UUSB_Register: Unable to create memory pools\n");
        return;
    }

    UUSB_TRACE("HCISU_LOG: UUSB_Register: hci_cmd_usb_pool_id (%d) hci_acl_usb_pool_id(%d)\n", usb.cmd_buffer_pool, usb.acl_buffer_pool);

    GKI_disable();

    usb.state = UUSB_PPC_REGISTERED_ST;

    GKI_enable();

    wait4hci = TRUE;
}

void UUSB_Open(tUUSB* uusb, tUUSB_CBACK* cback) {
    if (usb.state != UUSB_PPC_REGISTERED_ST) {
        UUSB_TRACE("HCISU_LOG: UUSB_Open: USB is not in registered state, unable to open\n");
        return;
    }

    if (usb.cmd_buffer_pool == GKI_INVALID_POOL || usb.acl_buffer_pool == GKI_INVALID_POOL) {
        UUSB_TRACE("HCISU_LOG: UUSB_Open: USB Buffer Pool's Invalid\n");
        UUSB_TRACE("HCISU_LOG: UUSB_Open: hci_cmd_usb_pool_id = %d\n", usb.cmd_buffer_pool);
        UUSB_TRACE("HCISU_LOG: UUSB_Open: hci_acl_usb_pool_id = %d\n", usb.acl_buffer_pool);
        return;
    }

    GKI_disable();

    usb.cback = cback;
    usb.state = UUSB_PPC_OPENED_ST;

    GKI_enable();

    uusb_issue_intr_read();
    uusb_issue_bulk_read();

    wait4hci = FALSE;

    (void)uusb;
}

UINT16 UUSB_Read(UINT8 param_1, void* param_2, UINT16 param_3) {
    return 0;
}

void UUSB_ReadBuf(/* ... */) {
    /* ... */
}

unk_t UUSB_WriteBuf(/* ... */) {
    return 0;
}

UINT16 UUSB_Write(UINT8 ep, void* p_data, UINT16 len, void* p_msg) {
    IPCResult ret = 0;
    void* p_buffer;
    HC_BT_HDR* p_buf;
    UINT16 i;

    if (usb.state != UUSB_PPC_OPENED_ST) {
        UUSB_TRACE("HCISU_ERR: UUSB_Write: usb_state != UUSB_PPC_OPENED_ST\n");
        return 0;
    }

    UUSB_TRACE("HCISU_LOG: buffer size = %d\n", GKI_get_buf_size(p_msg) - sizeof(HC_BT_HDR));
    UUSB_TRACE("HCISU_LOG: UUSB_Write: ep(%d) len(%d) data = ", ep, len);

    for (i = 0; i < len; i++)
        UUSB_TRACE("%02x ", ((UINT8*)p_data)[i]);

    UUSB_TRACE("\n");

    switch (ep) {
    case 0:
        p_buf = GKI_getpoolbuf(usb.cmd_buffer_pool);

        if (p_buf == NULL) {
            UUSB_TRACE("HCISU_ERR: UUSB_Write: Unable to get buffer from hci_cmd_usb_pool\n");
            return 0;
        }

        p_buf->len = len;

        p_buffer = ROUND_UP_PTR((UINT8*)(p_buf + 1), 32);

        p_buf->offset = (UINT8*)p_buffer - (UINT8*)(p_buf + 1);

        memcpy(p_buffer, p_data, len);

        if (usb.at_0x48 < UUSB_MAX_WRITES_OUTSTANDING && usb.intr_buffer_q.count == 0) {
            ret = IUSB_WriteCtrlMsgAsync(usb.fd, 0x20, 0, 0, 0, len, p_buffer, uusb_WriteCtrlDataCB, p_buf);
        } else {
            GKI_enqueue(&usb.intr_buffer_q, p_buf);

            UUSB_TRACE("HCISU_ERR: UUSB_WRITE(): Ctrl Queue count =  %d\n", usb.intr_buffer_q.count);

            return 0;
        }

        if (ret != 0) {
            UUSB_TRACE("HCISU_ERR: *************************************************************************************\n");
            UUSB_TRACE("HCISU_ERR: *************************************************************************************\n");
            UUSB_TRACE("HCISU_ERR: * UUSB_Write(): IUSB_WriteCtrlMsgAsync error...  (%d), writes outstanding (%d)\n", ret, usb.at_0x48);

            if (ret != -22)
                UUSB_TRACE("HCISU_ERR: * UUSB_Write(): IUSB_WriteCtrlMsgAsync error... IOS_ERROR_FAIL_ALLOC  (%d), writes outstanding (%d)\n", ret,
                           usb.at_0x38);

            if (ret == -8)
                UUSB_TRACE("HCISU_ERR: * UUSB_Write(): IUSB_WriteCtrlMsgAsync error... IOS_ERROR_QFULL  (%d), writes outstanding (%d)\n", ret,
                           usb.at_0x38);

            UUSB_TRACE("HCISU_ERR: *************************************************************************************\n");
            UUSB_TRACE("HCISU_ERR: *************************************************************************************\n");

            GKI_freebuf(p_buf);
        } else {
            GKI_disable();

            ++usb.at_0x48;

            GKI_enable();
        }

        break;

    case 2:
        p_buf = GKI_getpoolbuf(usb.acl_buffer_pool);

        if (p_buf == NULL) {
            UUSB_TRACE("HCISU_ERR: UUSB_Write: Unable to get buffer from hci_acl_usb_pool\n");
            return 0;
        }

        p_buf->len = len;

        p_buffer = ROUND_UP_PTR((UINT8*)(p_buf + 1), 32);

        p_buf->offset = (UINT8*)p_buffer - (UINT8*)(p_buf + 1);

        memcpy(p_buffer, p_data, len);

        if (len > 190)
            UUSB_TRACE("HCISU_ERR: UUSB_Write - writing mem data\n");

        if (usb.at_0x38 < UUSB_MAX_WRITES_OUTSTANDING && usb.bulk_buffer_q.count == 0) {
            ret = IUSB_WriteBlkMsgAsync(usb.fd, usb.bulkEndpoint1, len, p_buffer, uusb_WriteBulkDataCB, p_buf);
        } else {
            GKI_enqueue(&usb.bulk_buffer_q, p_buf);

            UUSB_TRACE("HCISU_ERR: UUSB_WRITE(): Bulk Queue count =  %d\n", usb.bulk_buffer_q.count);

            return 0;
        }

        if (ret != 0) {
            UUSB_TRACE("HCISU_ERR: *************************************************************************************\n");
            UUSB_TRACE("HCISU_ERR: *************************************************************************************\n");
            UUSB_TRACE("HCISU_ERR: * UUSB_Write(): IUSB_WriteBlkMsgAsync error...  (%d), writes outstanding (%d)\n", ret, usb.at_0x38);

            if (ret != -22)
                UUSB_TRACE("HCISU_ERR: * UUSB_Write(): IUSB_WriteBlkMsgAsync error... IOS_ERROR_FAIL_ALLOC  (%d), writes outstanding (%d)\n", ret,
                           usb.at_0x38);

            if (ret == -8)
                UUSB_TRACE("HCISU_ERR: * UUSB_Write(): IUSB_WriteBlkMsgAsync error... IOS_ERROR_QFULL  (%d), writes outstanding (%d)\n", ret,
                           usb.at_0x38);

            UUSB_TRACE("HCISU_ERR: *************************************************************************************\n");
            UUSB_TRACE("HCISU_ERR: *************************************************************************************\n");

            GKI_freebuf(p_buf);
        } else {
            GKI_disable();

            ++usb.at_0x38;

            GKI_enable();
        }

        break;

    case 3:
        break;
    }

    return ret;
}

void UUSB_Ioctl(tUUSB_IOCTL_OP op, void* p_data) {
    /* ... */
}

void UUSB_Close(void) {
    GKI_disable();

    usb.state = UUSB_PPC_CLOSED_ST;

    GKI_enable();

    if (usb.bulk_buffer_q.count) {
        while (usb.bulk_buffer_q.count)
            GKI_freebuf(GKI_dequeue(&usb.bulk_buffer_q));
    }

    if (usb.intr_buffer_q.count) {
        while (usb.intr_buffer_q.count)
            GKI_freebuf(GKI_dequeue(&usb.intr_buffer_q));
    }

    IUSB_CloseDeviceAsync(usb.fd, uusb_CloseDeviceCB, NULL);
}

void UUSB_Unregister(void) {
    IUSB_CloseLib();

    GKI_disable();

    usb.state = UUSB_PPC_INIT_ST;
    usb.trace_state = 0;

    uusb_g_trace_state_initialized = FALSE;

    GKI_enable();
}

unk_t UUSB_Feature(void) {
    return 0;
}
