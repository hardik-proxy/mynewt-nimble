/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "os/os.h"
#include "console/console.h"
#include "nimble/hci_common.h"
#include "nimble/ble_hci_trans.h"
#include "ble_hs_priv.h"

static void
ble_hs_dbg_le_event_disp(uint8_t subev, uint8_t len, uint8_t *evdata)
{
    int8_t rssi;
    uint8_t advlen;
    uint8_t status;
    int i;
    int imax;
    uint8_t *dptr;
    char *adv_ptr;
    char adv_data_buf[32];

    switch (subev) {
    case BLE_HCI_LE_SUBEV_ENH_CONN_COMPLETE:
    case BLE_HCI_LE_SUBEV_CONN_COMPLETE:
        status = evdata[0];
        if (status == BLE_ERR_SUCCESS) {
            BLE_HS_LOG(DEBUG, "LE connection complete. handle=%u role=%u "
                              "paddrtype=%u addr=%x.%x.%x.%x.%x.%x ",
                       le16toh(evdata + 1), evdata[3], evdata[4],
                       evdata[10], evdata[9], evdata[8], evdata[7],
                       evdata[6], evdata[5]);

            evdata += 11;
            if (subev == BLE_HCI_LE_SUBEV_ENH_CONN_COMPLETE) {
                BLE_HS_LOG(DEBUG, "local_rpa=%x.%x.%x.%x.%x.%x "
                                   "peer_rpa=%x.%x.%x.%x.%x.%x ",
                           evdata[5], evdata[4], evdata[3], evdata[2],
                           evdata[1], evdata[0],
                           evdata[11], evdata[10], evdata[9], evdata[8],
                           evdata[7], evdata[6]);

                evdata += 12;
            }
            BLE_HS_LOG(DEBUG, "itvl=%u latency=%u spvn_tmo=%u mca=%u\n",
                       le16toh(evdata), le16toh(evdata + 2),
                       le16toh(evdata + 4), evdata[6]);
        } else {
            BLE_HS_LOG(DEBUG, "LE connection complete. FAIL (status=%u)\n",
                       status);
        }
        break;
    case BLE_HCI_LE_SUBEV_ADV_RPT:
        advlen = evdata[9];
        rssi = evdata[10 + advlen];
        BLE_HS_LOG(DEBUG, "LE advertising report. len=%u num=%u evtype=%u "
                          "addrtype=%u addr=%x.%x.%x.%x.%x.%x advlen=%u "
                          "rssi=%d\n", len, evdata[0], evdata[1], evdata[2],
                   evdata[8], evdata[7], evdata[6], evdata[5],
                   evdata[4], evdata[3], advlen, rssi);
        if (advlen) {
            dptr = &evdata[10];
            while (advlen > 0) {
                memset(adv_data_buf, 0, 32);
                imax = advlen;
                if (imax > 8) {
                    imax = 8;
                }
                adv_ptr = &adv_data_buf[0];
                for (i = 0; i < imax; ++i) {
                    snprintf(adv_ptr, 4, "%02x ", *dptr);
                    adv_ptr += 3;
                    ++dptr;
                }
                advlen -= imax;
                BLE_HS_LOG(DEBUG, "%s\n", adv_data_buf);
            }
        }
        break;
    case BLE_HCI_LE_SUBEV_CONN_UPD_COMPLETE:
        status = evdata[0];
        if (status == BLE_ERR_SUCCESS) {
            BLE_HS_LOG(DEBUG, "LE Connection Update Complete. handle=%u "
                              "itvl=%u latency=%u timeout=%u\n",
                       le16toh(evdata + 1), le16toh(evdata + 3),
                       le16toh(evdata + 5), le16toh(evdata + 7));
        } else {
            BLE_HS_LOG(DEBUG, "LE Connection Update Complete. FAIL "
                              "(status=%u)\n", status);
        }
        break;

    case BLE_HCI_LE_SUBEV_DATA_LEN_CHG:
        BLE_HS_LOG(DEBUG, "LE Data Length Change. handle=%u max_tx_bytes=%u "
                          "max_tx_time=%u max_rx_bytes=%u max_rx_time=%u\n",
                   le16toh(evdata), le16toh(evdata + 2),
                   le16toh(evdata + 4), le16toh(evdata + 6),
                   le16toh(evdata + 8));
        break;
    case BLE_HCI_LE_SUBEV_REM_CONN_PARM_REQ:
        BLE_HS_LOG(DEBUG, "LE Remote Connection Parameter Request. handle=%u "
                          "min_itvl=%u max_itvl=%u latency=%u timeout=%u\n",
                   le16toh(evdata), le16toh(evdata + 2),
                   le16toh(evdata + 4), le16toh(evdata + 6),
                   le16toh(evdata + 8));
        break;

    case BLE_HCI_LE_SUBEV_RD_REM_USED_FEAT:
        status = evdata[0];
        if (status == BLE_ERR_SUCCESS) {
            BLE_HS_LOG(DEBUG, "LE Remote Used Features. handle=%u feat=",
                       le16toh(evdata + 1));
            for (i = 0; i < BLE_HCI_RD_LOC_SUPP_FEAT_RSPLEN; ++i) {
                BLE_HS_LOG(DEBUG, "%02x ", evdata[3 + i]);
            }
            BLE_HS_LOG(DEBUG, "\n");
        } else {
            BLE_HS_LOG(DEBUG, "LE Remote Used Features. FAIL (status=%u)\n",
                       status);
        }
        break;

    case BLE_HCI_LE_SUBEV_LT_KEY_REQ:
            BLE_HS_LOG(DEBUG, "LE LTK Req. handle=%u rand=%lx%lx encdiv=%u\n",
                       le16toh(evdata), le32toh(evdata + 6),
                       le32toh(evdata + 2), le16toh(evdata + 10));
        break;

    default:
        BLE_HS_LOG(DEBUG, "\tUnknown LE event\n");
        break;
    }
}

/**
 * Display a disconnection complete command.
 *
 *
 * @param evdata
 * @param len
 */
static void
ble_hs_dbg_disconn_comp_disp(uint8_t *evdata, uint8_t len)
{
    uint8_t status;
    uint8_t reason;
    uint16_t handle;

    status = evdata[0];
    handle = le16toh(evdata + 1);
    /* Ignore reason if status is not success */
    if (status != BLE_ERR_SUCCESS) {
        reason = 0;
    } else {
        reason = evdata[3];
    }
    BLE_HS_LOG(DEBUG, "Disconnection Complete: status=%u handle=%u "
                      "reason=%u\n", status, handle, reason);
}

/**
 * Display an encryption change event or encryption key refresh event
 *
 * @param evdata
 * @param len
 */
static void
ble_hs_dbg_encrypt_chg_disp(uint8_t *evdata, uint8_t len)
{
    uint8_t status;
    uint8_t enabled;
    uint16_t handle;

    status = evdata[0];
    handle = le16toh(evdata + 1);

    /* Ignore reason if status is not success */
    if (status != BLE_ERR_SUCCESS) {
        enabled = 0;
    } else {
        enabled = evdata[3];
    }
    BLE_HS_LOG(DEBUG, "Encrypt change: status=%u handle=%u state=%u\n",
               status, handle, enabled);
}

/**
 * Display an encryption encryption key refresh event
 *
 * @param evdata
 * @param len
 */
static void
ble_hs_dbg_encrypt_refresh_disp(uint8_t *evdata, uint8_t len)
{
    uint8_t status;
    uint16_t handle;

    status = evdata[0];
    handle = le16toh(evdata + 1);

    BLE_HS_LOG(DEBUG, "Encrypt key refresh: status=%u handle=%u\n",
               status, handle);
}

/**
 * Display a version information event
 *
 * @param evdata
 * @param len
 */
static void
ble_hs_dbg_rd_rem_ver_disp(uint8_t *evdata, uint8_t len)
{
    BLE_HS_LOG(DEBUG, "Remote Version Info: status=%u handle=%u vers_nr=%u "
                      "compid=%u subver=%u\n",
               evdata[0], le16toh(evdata + 1), evdata[3],
               le16toh(evdata + 4), le16toh(evdata + 6));
}

/**
 * Display the number of completed packets event
 *
 * @param evdata
 * @param len
 */
static void
ble_hs_dbg_num_comp_pkts_disp(uint8_t *evdata, uint8_t len)
{
    uint8_t handles;
    uint8_t *handle_ptr;
    uint8_t *pkt_ptr;
    uint16_t handle;
    uint16_t pkts;

    handles = evdata[0];
    if (len != ((handles * 4) + 1)) {
        BLE_HS_LOG(DEBUG, "ERR: Number of Completed Packets bad length: "
                          "num_handles=%u len=%u\n", handles, len);
        return;

    }

    BLE_HS_LOG(DEBUG, "Number of Completed Packets: num_handles=%u\n",
               handles);
    if (handles) {
        handle_ptr = evdata + 1;
        pkt_ptr = handle_ptr + (2 * handles);
        while (handles) {
            handle = le16toh(handle_ptr);
            handle_ptr += 2;
            pkts = le16toh(pkt_ptr);
            pkt_ptr += 2;
            BLE_HS_LOG(DEBUG, "handle:%u pkts:%u\n", handle, pkts);
            --handles;
        }
    }
}

/**
 * Display the authenticated payload timeout event
 *
 * @param evdata
 * @param len
 */
static void
ble_hs_dbg_auth_pyld_tmo_disp(uint8_t *evdata, uint8_t len)
{
    uint16_t handle;

    if (len != sizeof(uint16_t)) {
        BLE_HS_LOG(DEBUG, "ERR: AuthPyldTmoEvent bad length %u\n", len);
        return;

    }

    handle = le16toh(evdata);
    BLE_HS_LOG(DEBUG, "AuthPyldTmo: handle=%u\n", handle);
}


static void
ble_hs_dbg_cmd_comp_info_params(uint8_t status, uint8_t ocf, uint8_t *evdata)
{
    int i;
    uint8_t *dptr;

    if (status != BLE_ERR_SUCCESS) {
        return;
    }

    switch (ocf) {
    case BLE_HCI_OCF_IP_RD_LOCAL_VER:
        BLE_HS_LOG(DEBUG, "hci_ver=%u hci_rev=%u lmp_ver=%u mfrg=%u "
                          "lmp_subver=%u",
                   evdata[0], le16toh(evdata + 1), evdata[3],
                   le16toh(evdata + 4), le16toh(evdata + 6));
        break;
    case BLE_HCI_OCF_IP_RD_LOC_SUPP_CMD:
        BLE_HS_LOG(DEBUG, "supp_cmds=");
        dptr = evdata;
        for (i = 0; i < 8; ++i) {
            BLE_HS_LOG(DEBUG, "%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:",
                       dptr[0], dptr[1], dptr[2], dptr[3],
                       dptr[4], dptr[5], dptr[6], dptr[7]);
            dptr += 8;
        }
        break;
    case BLE_HCI_OCF_IP_RD_LOC_SUPP_FEAT:
        BLE_HS_LOG(DEBUG, "supp_feat=0x%lx%08lx",
                   le32toh(evdata + 4), le32toh(evdata));
        break;
    case BLE_HCI_OCF_IP_RD_BD_ADDR:
        BLE_HS_LOG(DEBUG, "bd_addr=%x:%x:%x:%x:%x:%x",
                   evdata[5], evdata[4], evdata[3],
                   evdata[2], evdata[1], evdata[0]);
        break;
    default:
        break;
    }
}

static void
ble_hs_dbg_cmd_complete_disp(uint8_t *evdata, uint8_t len)
{
    uint8_t cmd_pkts;
    uint8_t ogf;
    uint8_t ocf;
    uint8_t status;
    uint16_t opcode;

    if (len < 3) {
        BLE_HS_LOG(DEBUG, "Invalid command complete: len=%d "
                          "(expected >= 3)", len);
        goto done;
    }

    cmd_pkts = evdata[0];
    opcode = le16toh(evdata + 1);
    ogf = BLE_HCI_OGF(opcode);
    ocf = BLE_HCI_OCF(opcode);

    BLE_HS_LOG(DEBUG, "Command complete: cmd_pkts=%u ogf=0x%x ocf=0x%x",
               cmd_pkts, ogf, ocf);

    if (len == 3) {
        goto done;
    }

    status = evdata[3];
    BLE_HS_LOG(DEBUG, " status=%u ", status);

    /* Move past header and status */
    evdata += 4;

    /* Display parameters based on command. */
    switch (ogf) {
    case BLE_HCI_OGF_INFO_PARAMS:
        ble_hs_dbg_cmd_comp_info_params(status, ocf, evdata);
        break;
    case BLE_HCI_OGF_STATUS_PARAMS:
        switch (ocf) {
        case BLE_HCI_OCF_RD_RSSI:
            BLE_HS_LOG(DEBUG, "handle=%u rssi=%d", le16toh(evdata),
                       (int8_t)evdata[2]);
            break;
        default:
            break;
        }
        break;
    case BLE_HCI_OGF_LE:
        switch (ocf) {
        case BLE_HCI_OCF_LE_RD_CHAN_MAP:
            BLE_HS_LOG(DEBUG, "handle=%u chanmap=%x.%x.%x.%x.%x",
                       le16toh(evdata), evdata[2], evdata[3], evdata[4],
                       evdata[5], evdata[6]);
            break;
        case BLE_HCI_OCF_LE_RD_MAX_DATA_LEN:
            BLE_HS_LOG(DEBUG, "txoct=%u txtime=%u rxoct=%u rxtime=%u",
                       le16toh(evdata), le16toh(evdata + 2),
                       le16toh(evdata + 4), le16toh(evdata + 6));
            break;
        case BLE_HCI_OCF_LE_RD_SUPP_STATES:
            BLE_HS_LOG(DEBUG, "states=0x%lx%08lx", le32toh(evdata + 4),
                       le32toh(evdata));
            break;
        case BLE_HCI_OCF_LE_ENCRYPT:
            BLE_HS_LOG(DEBUG, "encdata=0x%02x%02x%02x%02x%02x%02x%02x%02x",
                       evdata[15], evdata[14], evdata[13], evdata[12],
                       evdata[11], evdata[10], evdata[9], evdata[8]);
            BLE_HS_LOG(DEBUG, "%02x%02x%02x%02x%02x%02x%02x%02x",
                       evdata[7], evdata[6], evdata[5], evdata[4],
                       evdata[3], evdata[2], evdata[1], evdata[0]);

            break;
        case BLE_HCI_OCF_LE_RAND:
            BLE_HS_LOG(DEBUG, "rand=0x%02x%02x%02x%02x%02x%02x%02x%02x",
                       evdata[0], evdata[1], evdata[2], evdata[3],
                       evdata[4], evdata[5], evdata[6], evdata[7]);
            break;
        case BLE_HCI_OCF_LE_RD_SUGG_DEF_DATA_LEN:
            BLE_HS_LOG(DEBUG, "txoct=%u txtime=%u", le16toh(evdata),
                       le16toh(evdata + 2));
            break;
        case BLE_HCI_OCF_LE_LT_KEY_REQ_REPLY:
        case BLE_HCI_OCF_LE_LT_KEY_REQ_NEG_REPLY:
        case BLE_HCI_OCF_LE_SET_DATA_LEN:
            BLE_HS_LOG(DEBUG, "handle=%u", le16toh(evdata));
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }

done:
    BLE_HS_LOG(DEBUG, "\n");
}

static void
ble_hs_dbg_cmd_status_disp(uint8_t *evdata, uint8_t len)
{
    uint8_t ogf;
    uint8_t ocf;
    uint16_t opcode;

    opcode = le16toh(evdata + 2);
    ogf = BLE_HCI_OGF(opcode);
    ocf = BLE_HCI_OCF(opcode);

    BLE_HS_LOG(DEBUG, "Command Status: status=%u cmd_pkts=%u ocf=0x%x "
                      "ogf=0x%x\n", evdata[0], evdata[1], ocf, ogf);
}

void
ble_hs_dbg_event_disp(uint8_t *evbuf)
{
#if MYNEWT_VAL(LOG_LEVEL) > LOG_LEVEL_DEBUG
    return;
#endif

    uint8_t *evdata;
    uint8_t evcode;
    uint8_t len;

    /* Extract event code and length; move pointer to event parameter data */
    evcode = evbuf[0];
    len = evbuf[1];
    evdata = evbuf + BLE_HCI_EVENT_HDR_LEN;

    switch (evcode) {
    case BLE_HCI_EVCODE_DISCONN_CMP:
        ble_hs_dbg_disconn_comp_disp(evdata, len);
        break;
    case BLE_HCI_EVCODE_ENC_KEY_REFRESH:
        ble_hs_dbg_encrypt_refresh_disp(evdata, len);
        break;
    case BLE_HCI_EVCODE_ENCRYPT_CHG:
        ble_hs_dbg_encrypt_chg_disp(evdata, len);
        break;
    case BLE_HCI_EVCODE_RD_REM_VER_INFO_CMP:
        ble_hs_dbg_rd_rem_ver_disp(evdata, len);
        break;
    case BLE_HCI_EVCODE_COMMAND_COMPLETE:
        ble_hs_dbg_cmd_complete_disp(evdata, len);
        break;
    case BLE_HCI_EVCODE_COMMAND_STATUS:
        ble_hs_dbg_cmd_status_disp(evdata, len);
        break;
    case BLE_HCI_EVCODE_NUM_COMP_PKTS:
        ble_hs_dbg_num_comp_pkts_disp(evdata, len);
        break;
    case BLE_HCI_EVCODE_LE_META:
        ble_hs_dbg_le_event_disp(evdata[0], len, evdata + 1);
        break;
    case BLE_HCI_EVCODE_AUTH_PYLD_TMO:
        ble_hs_dbg_auth_pyld_tmo_disp(evdata, len);
        break;
    default:
        BLE_HS_LOG(DEBUG, "Unknown event 0x%x len=%u\n", evcode, len);
        break;
    }
}

void
ble_hs_dbg_set_sync_state(uint8_t sync_state)
{
    ble_hs_sync_state = sync_state;
}
