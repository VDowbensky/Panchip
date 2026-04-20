#ifndef PAN_ERR_H
#define PAN_ERR_H

typedef int pan_err_t;

#define PAN_OK                   0
#define PAN_FAIL                 (-1)

//net 1xxxx
#define NET_SEND_DATA_CHECK_ERR   10001           //data is invalid
#define NET_RF_INIT_FAIL          10002           //rf init fail

//transmit 2xxxx
#define TRANS_SEND_FAIL           20001           //send fail
#define TRANS_SEND_WAIT           20002           //wait send finish
#define TRANS_SEND_TIMEOUT        20003           //send timeout
#define TRANS_RECV_NODATA         20004           //There is no data to receive
#define TRANS_RECV_NO_COMPLETE    20005           //data is not complete
#define TRANS_RF_BUSY             20006           //rf busy
#define TRANS_RECV_DATA_ERROR     20007           //recv data fail
#define TRANS_SNED_SUCCESS        20008           //send data success
#define TRANS_RECV_SUCCESS        20009           //recv data success

#endif //PAN_ERR_H
