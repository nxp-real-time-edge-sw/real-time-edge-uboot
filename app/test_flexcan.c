// SPDX-License-Identifier: GPL-2.0+
/*
 * Flexcan or CANopen test code.
 *
 * Copyright 2018-2023 NXP
 *
 * Author: Jianchao Wang <jianchao.wang@nxp.com>
 *
 */

#include <linux/delay.h>
#include <common.h>
#include <flexcan.h>
#include <dm.h>
#include <ftm_alarm.h>
#include <env.h>

#if CONFIG_CANFESTIVAL
/* the head file of canfestival */
#include "canfestival.h"
#include "data.h"
#include "TestSlave.h"
#include "callback.h"
#endif

void flexcan_rx_irq(struct can_module *canx)
{
	struct can_frame rx_message;

	flexcan_receive(canx, &rx_message);
#if CONFIG_CANFESTIVAL
	/* the CAN message structure that is defined by canopen */
	Message m;
	u8 i = 0;

	if (TestSlave_Data.nodeState == Unknown_state)
		setState(&TestSlave_Data, Stopped);

	if (rx_message.can_id & CAN_EFF_FLAG) {
		printf("Error: the current canopen "
		       "doesn't support extended frame!\n");
		return;
	}
	m.cob_id = rx_message.can_id & CAN_SFF_MASK;
	if (rx_message.can_id & CAN_RTR_FLAG)
		m.rtr = 1;
	else if (rx_message.can_id & CAN_ERR_FLAG)
		printf("Warn: this is a error CAN message!\n");
	else
		m.rtr = 0;
	m.len = rx_message.can_dlc;
	for (i = 0; i < rx_message.can_dlc; i++)
		m.data[i] = rx_message.data[i];

	canDispatch(&TestSlave_Data, &m);
#else
	/* test code */
	rx_message.can_id++;
	flexcan_send(canx, &rx_message);
#endif
}

/* entry into the interrupt of flextimer one time per 100us */
void flextimer_overflow_irq(void)
{
#if CONFIG_CANFESTIVAL
	timerForCan();
#else
	/* test code */
	static unsigned int flag;
	static struct can_frame canfram_tx = {
		.can_id = 0x00000003,
		.can_dlc = 6,
		.data = "timer!"
	};
	flag++;
	if (flag >= 100000) {/* 10s */
		flexcan_send(CAN3, &canfram_tx);
		flag = 0;
	}
#endif
}

static int get_ftm_alarm_idx(char *comp_name)
{
    struct uclass *uc;
    struct udevice *dev;
    int target_idx = -1, idx = 0;

    uclass_id_foreach_dev(UCLASS_RTC, dev, uc) {
        if (!strcmp(dev_read_string(dev, "compatible"), comp_name)) {
            target_idx = idx;
            break;
        }
        idx++;
    }
    return target_idx;
}

void test_flexcan(void)
{
	char *comp_name = "fsl,ls1046a-ftm-alarm";
	int ftm_dev_idx, ret;
	struct udevicd *dev;
	flexcan_rx_handle = flexcan_rx_irq;

	ftm_dev_idx = get_ftm_alarm_idx(comp_name);
	if (ftm_dev_idx == -1) {
		printf("[ERROR] No available ftm alarm device.\n");
		return;
	}

	ret = uclass_get_device(UCLASS_RTC, ftm_dev_idx, &dev);
	if (ret) {
		printf("Cannot find RTC #%d, err=%d\n", ftm_dev_idx, ret);
		return;
	}

	ftm_rtc_set_alarm(dev, 2000, flextimer_overflow_irq);

#if CONFIG_CANFESTIVAL
	u8 node_id = 0x02;

	printf("Note: the CANopen protocol starts to run!\n");
	callback_list_init();
	setNodeId(&TestSlave_Data, node_id);
	/* setState(&TestSlave_Data, Stopped); */
#else
	/* test code */
	struct can_frame canfram_tx = {
		.can_id = 0x00000003,
		.can_dlc = 6,
		.data = "start!"
	};

	flexcan_send(CAN3, &canfram_tx);
#endif

	ftm_rtc_alarm_stop(dev);
}
