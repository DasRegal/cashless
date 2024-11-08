/**
  *******************************************************************************
  * Includes
  *******************************************************************************
  */
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include "mdb.h"

/**
  *******************************************************************************
  * Defines
  *******************************************************************************
  */
#define MDB_CASHLESS_DEV_1_ADDR     0x10        /* 0x10 or 0x60 for Cashless */
#define MDB_MODE_BIT                0x100
#define MDB_ACK                     0x00
#define MDB_NAK                     0xFF

/* Responce */
#define MDB_JUST_RESET_RESP         0x00
#define MDB_CONFIG_DATA_RESP        0x01
#define MDB_VEND_APPROVED_RESP      0x05
#define MDB_PERIPH_ID_RESP          0x09

/**
  *******************************************************************************
  * Local Types and Typedefs
  *******************************************************************************
  */

typedef struct
{
    int item;
    int price;
    bool is_start;
} mdb_vend_t;

/**
  *******************************************************************************
  * Macros
  *******************************************************************************
  */

/**
  *******************************************************************************
  * Global Variables
  *******************************************************************************
  */

/**
  *******************************************************************************
  * Static Variables
  *******************************************************************************
  */
static mdb_t mdb_dev = {
    .MdbQueueCmdPushCB = NULL,
    .MdbQueueCmdPullCB = NULL,
    .MdbTxBufCB        = NULL,
    .MdbReconfigCB     = NULL,
    .is_wait_rx        = false,
    .amount            = 0,
};

static mdb_vend_t mdb_vend_struct = { .is_start = false };
static mdb_cmd_t  force_cmd;

/**
  *******************************************************************************
  * Static Function Prototypes
  *******************************************************************************
  */
static void     MdbReset(void);
static void     MdbTxBufCB(uint16_t * buf, uint8_t len);
static uint16_t MdbCalcChk(uint16_t * buf, uint8_t len);
static bool     MdbIsValidateChk(uint16_t * buf, uint8_t len);

/**
  *******************************************************************************
  * Functions
  *******************************************************************************
  */

/**
  * @brief  Send VEND REQUEST
  * @param  mdb_t MDB device struct
  * @retval None
  */
void MdbInit(mdb_t mdb_struct)
{
    mdb_dev.MdbQueueCmdPushCB       = mdb_struct.MdbQueueCmdPushCB;
    mdb_dev.MdbQueueCmdPullCB       = mdb_struct.MdbQueueCmdPullCB;
    mdb_dev.MdbTxBufCB              = mdb_struct.MdbTxBufCB;
    mdb_dev.MdbReconfigCB           = mdb_struct.MdbReconfigCB;
    mdb_dev.p_info                  = mdb_struct.p_info;
    mdb_dev.vmc_level               = mdb_struct.vmc_level;
    mdb_dev.conf.cashless_level     = 1;
    MdbReset();
}

/**
  * @brief  MDB reset
  * @param  None
  * @retval None
  */
static void MdbReset(void)
{
    mdb_cmd_t mdb_cmd_struct;
    force_cmd.is_force = MDB_NO_FORCE_CMD;

    mdb_dev.is_wait_rx         = false;
    mdb_dev.rx_data_len        = 0;
    mdb_dev.conf.max_resp_time = 0xFF;

    /* RESET */
    mdb_cmd_struct.cmd    = MDB_RESET_CMD_E;
    mdb_dev.MdbQueueCmdPushCB(mdb_cmd_struct);

    /* POLL */
    mdb_cmd_struct.cmd    = MDB_POLL_CMD_E;
    mdb_dev.MdbQueueCmdPushCB(mdb_cmd_struct);

    /* SETUP */
    mdb_cmd_struct.cmd    = MDB_SETUP_CMD_E;

    mdb_cmd_struct.subcmd = MDB_SETUP_CONFIG_SUBCMD_E;
    mdb_dev.MdbQueueCmdPushCB(mdb_cmd_struct);

    mdb_cmd_struct.subcmd = MDB_SETUP_PRICE_SUBCMD_E;
    mdb_dev.MdbQueueCmdPushCB(mdb_cmd_struct);

    /* EXPANSION REQUEST ID */
    mdb_cmd_struct.cmd    = MDB_EXPANSION_CMD_E;
    mdb_cmd_struct.subcmd = MDB_REQUEST_ID_SUBCMD_E;
    mdb_dev.MdbQueueCmdPushCB(mdb_cmd_struct);

    /* POLL */
    mdb_cmd_struct.cmd    = MDB_POLL_CMD_E;
    mdb_dev.MdbQueueCmdPushCB(mdb_cmd_struct);

    /* READER ENABLE */
    mdb_cmd_struct.cmd    = MDB_READER_CMD_E;
    mdb_cmd_struct.subcmd = MDB_READER_ENABLE_SUBCMD_E;
    mdb_dev.MdbQueueCmdPushCB(mdb_cmd_struct);

    mdb_vend_struct.is_start = false;
}

/**
  * @brief  Send VEND REQUEST
  * @param  item   Item for request
  * @param  price  Price for request
  * @retval None
  */
void MdbVendStart(int item, int price)
{
    mdb_cmd_t mdb_cmd_struct = { .cmd = MDB_VEND_CMD_E, .subcmd = MDB_VEND_REQ_SUBCMD_E };

    if ( mdb_vend_struct.is_start )
    {
        return;
    }

    mdb_vend_struct.is_start = true;
    mdb_vend_struct.item     = item;
    mdb_vend_struct.price    = price;

    if (mdb_dev.MdbQueueCmdPushCB != NULL)
    {
        mdb_dev.MdbQueueCmdPushCB(mdb_cmd_struct);
    }
}

/**
  * @brief  Check Vend request
  * @param  None
  * @retval An mdb_ret_t enumuration value:
  *     - MDB_RET_OK:            Launching the Vend request
  *     - MDB_RET_NOT_PURCHASE:  MdbVendStart function was not started
  *     - MDB_RET_BUSY_PURCHASE: Vend request in process. Need to wait for completion (MDB_RET_OK).
  *     - MDB_RET_NO_BALANCE:    Return Vend Denied
  */
int MdbVendCheck(void)
{
    if ( !mdb_vend_struct.is_start )
    {
        return MDB_RET_NOT_PURCHASE;
    }

    return MDB_RET_OK;
}

/**
  * @brief  Finish Vend
  * @param  is_vend_success:
  *     - true:  send VEND SUCCESS
  *     - false: send VEND FAILURE
  * @retval None
  */
void MdbVendFinish(bool is_vend_success)
{
    mdb_cmd_t mdb_cmd_struct = { .cmd = MDB_VEND_CMD_E, .subcmd = MDB_VEND_FAILURE_SUBCMD_E };

    if ( !mdb_vend_struct.is_start )
    {
        return;
    }

    if (mdb_dev.MdbQueueCmdPushCB != NULL)
    {
        mdb_dev.MdbQueueCmdPushCB(mdb_cmd_struct);
    }

    mdb_vend_struct.is_start = false;
}

/**
  * @brief  Send Command
  * @param  buf  Send buffer
  *         len  Buffer length
  * @retval None
  */
static void MdbTxBufCB(uint16_t * buf, uint8_t len)
{
    mdb_dev.MdbTxBufCB(buf, len);
}

/**
  * @brief  Poll to send command
  * @param  None
  * @retval None
  */
void MdbPollSend(void)
{
    uint8_t     data_len = 0;
    mdb_cmd_t   mdb_cmd_struct;

    if ( mdb_dev.is_wait_rx )
    {
        return;
    }

    if (force_cmd.is_force)
    {
        force_cmd.is_force    = MDB_NO_FORCE_CMD;
        mdb_cmd_struct.cmd    = force_cmd.cmd;
        mdb_cmd_struct.subcmd = force_cmd.subcmd;

        switch(mdb_cmd_struct.cmd)
        {
            case MDB_ACK_CMD_E:
                mdb_dev.tx_data[0] = MDB_ACK;
                MdbTxBufCB(mdb_dev.tx_data, 1);
                return;
            case MDB_NAK_CMD_E:
                mdb_dev.tx_data[0] = MDB_NAK;
                MdbTxBufCB(mdb_dev.tx_data, 1);
                mdb_dev.is_wait_rx = true;
                return;
            default:
                break;
        }
    }
    else
    {
        mdb_cmd_struct = mdb_dev.MdbQueueCmdPullCB();
    }

    if ( mdb_cmd_struct.cmd == MDB_EMPTY_CMD_E)
    {
        mdb_cmd_struct.cmd = MDB_POLL_CMD_E;
    }

    mdb_dev.tx_data[0] = mdb_cmd_struct.cmd + MDB_CASHLESS_DEV_1_ADDR + MDB_MODE_BIT;

    /* Without Sub-Command */
    if (mdb_cmd_struct.cmd == MDB_RESET_CMD_E || mdb_cmd_struct.cmd == MDB_POLL_CMD_E)
    {
        mdb_dev.tx_data[1] = MdbCalcChk(mdb_dev.tx_data, 1);
        MdbTxBufCB(mdb_dev.tx_data, 2);
        mdb_dev.is_wait_rx = true;
        return;
    }
    
    mdb_dev.tx_data[1] = mdb_cmd_struct.subcmd;

    if (mdb_cmd_struct.cmd == MDB_SETUP_CMD_E)
    {
        switch (mdb_cmd_struct.subcmd)
        {
            case MDB_SETUP_CONFIG_SUBCMD_E:
                data_len = 6;
                mdb_dev.tx_data[2] = 2;     /* VMC Feature Level.   */
                mdb_dev.tx_data[3] = 0;     /* Columns on Display.  */
                mdb_dev.tx_data[4] = 0;     /* Rows on Display.     */
                mdb_dev.tx_data[5] = 0;     /* Display Information. */
                mdb_dev.tx_data[6] = MdbCalcChk(mdb_dev.tx_data, data_len);
                break;

            case MDB_SETUP_PRICE_SUBCMD_E:
                data_len = 6;
                mdb_dev.tx_data[2] = 0xFF;  /* Maximum Price. */
                mdb_dev.tx_data[3] = 0xFF;
                mdb_dev.tx_data[4] = 0x00;  /* Minimum Price. */
                mdb_dev.tx_data[5] = 0x00;
                mdb_dev.tx_data[6] = MdbCalcChk(mdb_dev.tx_data, data_len);
                break;

            default:
                return;
        }
    }

    if (mdb_cmd_struct.cmd == MDB_VEND_CMD_E)
    {
        switch (mdb_cmd_struct.subcmd)
        {
            case MDB_VEND_REQ_SUBCMD_E:
                data_len = 6;
                mdb_dev.tx_data[2] = (mdb_vend_struct.price >> 8) & 0xFF;
                mdb_dev.tx_data[3] = mdb_vend_struct.price & 0xFF;
                mdb_dev.tx_data[4] = (mdb_vend_struct.item >> 8) & 0xFF;
                mdb_dev.tx_data[5] = mdb_vend_struct.item & 0xFF;
                mdb_dev.tx_data[6] = MdbCalcChk(mdb_dev.tx_data, data_len);
                break;

            case MDB_VEND_SUCCESS_SUBCMD_E:
                data_len = 4;
                mdb_dev.tx_data[2] = (mdb_vend_struct.item >> 8) & 0xFF;
                mdb_dev.tx_data[3] = mdb_vend_struct.item & 0xFF;
                mdb_dev.tx_data[4] = MdbCalcChk(mdb_dev.tx_data, data_len);
                break;

            case MDB_VEND_FAILURE_SUBCMD_E:
                data_len = 2;
                mdb_dev.tx_data[2] = MdbCalcChk(mdb_dev.tx_data, data_len);
                break;

            default:
                return;
        }
    }

    if (mdb_cmd_struct.cmd == MDB_READER_CMD_E)
    {
        switch (mdb_cmd_struct.subcmd)
        {
            case MDB_READER_ENABLE_SUBCMD_E:
                data_len = 2;
                mdb_dev.tx_data[2] = MdbCalcChk(mdb_dev.tx_data, data_len);
                break;

            default:
                return;
        }
    }

    if (mdb_cmd_struct.cmd == MDB_EXPANSION_CMD_E)
    {
        switch (mdb_cmd_struct.subcmd)
        {
            case MDB_REQUEST_ID_SUBCMD_E:
                data_len = 2 + 3 + 12 + 12 + 2;

                for (size_t i = 0; i < 29; i++)
                {
                    mdb_dev.tx_data[i + 2] = mdb_dev.p_info[i];
                }
                mdb_dev.tx_data[data_len] = MdbCalcChk(mdb_dev.tx_data, data_len);
                break;

            default:
                return;
        }
    }

    MdbTxBufCB(mdb_dev.tx_data, data_len + 1);
    mdb_dev.is_wait_rx = true;
}

/**
  * @brief  Poll to receive command
  * @param  None
  * @retval None
  */
void MdbPollReceive(uint16_t ch)
{
    uint8_t resp_header;

    /* Buffer overflow */
    if (mdb_dev.rx_data_len > MDB_MAX_BUF_LEN)
    {
        mdb_dev.rx_data[0]  = 0;
        mdb_dev.rx_data_len = 0;
    }

    mdb_dev.rx_data[mdb_dev.rx_data_len] = ch;
    mdb_dev.rx_data_len++;

    if (ch & 0x100)
    {
        /* ACK/NAK handling */
        if (mdb_dev.rx_data_len == 1)
        {
            switch(mdb_dev.rx_data[0] & 0xFF)
            {
                case MDB_ACK:
                    /* Reconfigure after MdbReset() function */
                    if ((mdb_dev.tx_data[0] & 0x00F) == MDB_READER_CMD_E && 
                        (mdb_dev.tx_data[1] & 0x0FF) == MDB_READER_ENABLE_SUBCMD_E)
                    {
                        mdb_dev.MdbReconfigCB((void*)&mdb_dev);
                    }
                    mdb_dev.is_wait_rx = false;
                    break;
                default:
                    MdbReset();
                    return;
            }
            mdb_dev.rx_data[0]  = 0;
            mdb_dev.rx_data_len = 0;
            return;
        }

        /* Other commands handling */
        if (MdbIsValidateChk(mdb_dev.rx_data, mdb_dev.rx_data_len))
        {
            resp_header = mdb_dev.rx_data[0];

            switch(resp_header)
            {
                case MDB_JUST_RESET_RESP:
                    /* TODO: Implement reinitialization */

                    /* If a device sends a just reset response,
                     * the VMC should re-initialize the device
                     * (request setup information, re-enable the device,
                     * etc.). Don’t send a reset command.
                     */
                    break;

                case MDB_CONFIG_DATA_RESP:
                    /* Reader Feature Level. */
                    mdb_dev.conf.cashless_level = mdb_dev.rx_data[1];
                    /* Application Maximum Response Time */
                    mdb_dev.conf.max_resp_time = mdb_dev.rx_data[6];
                    break;

                case MDB_VEND_APPROVED_RESP:
                    mdb_dev.amount += (mdb_dev.rx_data[1] << 8) | mdb_dev.rx_data[2];
                    break;

                case MDB_PERIPH_ID_RESP:
                    for (uint8_t i = 0; i < 3; i++)
                        mdb_dev.conf.manufact_code[i] = mdb_dev.rx_data[i + 1];
                    for (uint8_t i = 0; i < 12; i++)
                        mdb_dev.conf.serial_num[i] = mdb_dev.rx_data[i + 1 + 3];
                    for (uint8_t i = 0; i < 12; i++)
                        mdb_dev.conf.model_num[i] = mdb_dev.rx_data[i + 1 + 3 + 12];
                    mdb_dev.conf.sw_version = (mdb_dev.rx_data[28] << 8) + mdb_dev.rx_data[29];
                    break;

                default:
                    break;
            }

            force_cmd.cmd      = MDB_ACK_CMD_E;
            force_cmd.is_force = MDB_FORCE_CMD;
        }
        else
        {
            /* Incorrect checksum */
            force_cmd.cmd      = MDB_NAK_CMD_E;
            force_cmd.is_force = MDB_FORCE_CMD;
        }
        mdb_dev.rx_data[0]  = 0;
        mdb_dev.rx_data_len = 0;
        mdb_dev.is_wait_rx = false;
        return;
    }
}

/**
  * @brief  Calculate checksum
  * @param  buf  Send buffer
  *         len  Buffer length
  * @retval Checksum
  */
static uint16_t MdbCalcChk(uint16_t * buf, uint8_t len)
{
    uint16_t check_sum = 0;

    for(uint8_t i = 0; i < len; i++)
    {
        check_sum += buf[i] & 0xFF;
    }

    return check_sum & 0xFF;
}

/**
  * @brief  Validate checksum
  * @param  buf  Send buffer
  *         len  Buffer length
  * @retval Check result
  */
static bool MdbIsValidateChk(uint16_t * buf, uint8_t len)
{
    uint16_t calc_sum = 0;
    uint16_t rec_sum  = buf[len - 1] & 0xFF;

    for(uint8_t i = 0; i < len - 1; i++)
    {
        calc_sum += buf[i] & 0xFF;
    }

    calc_sum = calc_sum & 0xFF;

    return (calc_sum == rec_sum) ? true : false;
}
