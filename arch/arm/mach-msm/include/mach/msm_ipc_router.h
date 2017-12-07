/* Copyright (c) 2012-2013, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef _MSM_IPC_ROUTER_H
#define _MSM_IPC_ROUTER_H

#include <linux/types.h>
#include <linux/socket.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/list.h>
#include <linux/wakelock.h>
#include <linux/msm_ipc.h>

#define MAX_WAKELOCK_NAME_SZ 32

/**
 * enum msm_ipc_router_event - Events that will be generated by IPC Router
 */
enum msm_ipc_router_event {
	MSM_IPC_ROUTER_READ_CB = 0,
	MSM_IPC_ROUTER_WRITE_DONE,
	MSM_IPC_ROUTER_RESUME_TX,
};

struct comm_mode_info {
	int mode;
	void *xprt_info;
};

struct msm_ipc_port {
	struct list_head list;

	struct msm_ipc_port_addr this_port;
	struct msm_ipc_port_name port_name;
	uint32_t type;
	unsigned flags;
	spinlock_t port_lock;
	struct comm_mode_info mode_info;

	struct list_head port_rx_q;
	struct mutex port_rx_q_lock_lhb3;
	char rx_wakelock_name[MAX_WAKELOCK_NAME_SZ];
	struct wake_lock port_rx_wake_lock;
	wait_queue_head_t port_rx_wait_q;

	int restart_state;
	spinlock_t restart_lock;
	wait_queue_head_t restart_wait;

	void *endpoint;
	void (*notify)(unsigned event, void *priv);
	int (*check_send_permissions)(void *data);

	uint32_t num_tx;
	uint32_t num_rx;
	unsigned long num_tx_bytes;
	unsigned long num_rx_bytes;
	void *priv;
};

#ifdef CONFIG_MSM_IPC_ROUTER
/**
 * msm_ipc_router_create_port() - Create a IPC Router port/endpoint
 * @notify: Callback function to notify any event on the port.
 * @priv: Private info to be passed while the notification is generated.
 *
 * @return: Pointer to the port on success, NULL on error.
 */
struct msm_ipc_port *msm_ipc_router_create_port(
	void (*notify)(unsigned event, void *priv),
	void *priv);

/**
 * msm_ipc_router_lookup_server_name() - Resolve server address
 * @srv_name: Name<service:instance> of the server to be resolved.
 * @srv_info: Buffer to hold the resolved address.
 * @num_entries_in_array: Number of server info the buffer can hold.
 * @lookup_mask: Mask to specify the range of instances to be resolved.
 *
 * @return: Number of server addresses resolved on success, < 0 on error.
 */
#ifdef CONFIG_MSM8960_ONLY
int msm_ipc_router_lookup_server_name(struct msm_ipc_port_name *srv_name,
				      struct msm_ipc_port_addr *port_addr,
				      int num_entries_in_array,
				      uint32_t lookup_mask);
#else
int msm_ipc_router_lookup_server_name(struct msm_ipc_port_name *srv_name,
				      struct msm_ipc_server_info *srv_info,
				      int num_entries_in_array,
				      uint32_t lookup_mask);
#endif //CONFIG_MSM8960_ONLY


/**
 * msm_ipc_router_send_msg() - Send a message/packet
 * @src: Sender's address/port.
 * @dest: Destination address.
 * @data: Pointer to the data to be sent.
 * @data_len: Length of the data to be sent.
 *
 * @return: 0 on success, < 0 on error.
 */
int msm_ipc_router_send_msg(struct msm_ipc_port *src,
			    struct msm_ipc_addr *dest,
			    void *data, unsigned int data_len);

/**
 * msm_ipc_router_get_curr_pkt_size() - Get the packet size of the first
 *                                      packet in the rx queue
 * @port_ptr: Port which owns the rx queue.
 *
 * @return: Returns the size of the first packet, if available.
 *          0 if no packets available, < 0 on error.
 */
int msm_ipc_router_get_curr_pkt_size(struct msm_ipc_port *port_ptr);

/**
 * msm_ipc_router_read_msg() - Read a message/packet
 * @port_ptr: Receiver's port/address.
 * @data: Pointer containing the address of the received data.
 * @src: Address of the sender/source.
 * @len: Length of the data being read.
 *
 * @return: 0 on success, < 0 on error.
 */
int msm_ipc_router_read_msg(struct msm_ipc_port *port_ptr,
			    struct msm_ipc_addr *src,
			    unsigned char **data,
			    unsigned int *len);

/**
 * msm_ipc_router_close_port() - Close the port
 * @port_ptr: Pointer to the port to be closed.
 *
 * @return: 0 on success, < 0 on error.
 */
int msm_ipc_router_close_port(struct msm_ipc_port *port_ptr);

#else

struct msm_ipc_port *msm_ipc_router_create_port(
	void (*notify)(unsigned event, void *priv),
	void *priv)
{
	return NULL;
}

int msm_ipc_router_lookup_server_name(struct msm_ipc_port_name *srv_name,
				      struct msm_ipc_server_info *srv_info,
				      int num_entries_in_array,
				      uint32_t lookup_mask)
{
	return -ENODEV;
}

int msm_ipc_router_send_msg(struct msm_ipc_port *src,
			    struct msm_ipc_addr *dest,
			    void *data, unsigned int data_len)
{
	return -ENODEV;
}

int msm_ipc_router_get_curr_pkt_size(struct msm_ipc_port *port_ptr)
{
	return -ENODEV;
}

int msm_ipc_router_read_msg(struct msm_ipc_port *port_ptr,
			    struct msm_ipc_addr *src,
			    unsigned char **data,
			    unsigned int *len)
{
	return -ENODEV;
}

int msm_ipc_router_close_port(struct msm_ipc_port *port_ptr)
{
	return -ENODEV;
}

#endif

#endif
