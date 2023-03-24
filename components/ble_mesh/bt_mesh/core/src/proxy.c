/*  Bluetooth Mesh */

/*
 * Copyright (c) 2017 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <ble_os.h>
#include <misc/byteorder.h>

#include <net/buf.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/conn.h>
#include <bluetooth/gatt.h>
#include <api/mesh.h>
#include <bluetooth/hci.h>
#include <host/conn_internal.h>

#define BT_DBG_ENABLED IS_ENABLED(CONFIG_BT_MESH_DEBUG_PROXY)
#include "common/log.h"

#include "mesh.h"
#include "adv.h"
#include "net.h"
#include "prov.h"
#include "beacon.h"
#include "foundation.h"
#include "access.h"
#include "proxy.h"
#include "errno.h"

#ifdef CONFIG_GENIE_OTA
#include "genie_mesh_internal.h"
#endif

#ifdef CONFIG_GENIE_OTA
extern int genie_ota_pre_init(void);
extern void genie_ais_adv_init(uint8_t ad_structure[14], uint8_t is_silent);
#endif

#ifdef CONFIG_BT_MESH_IBEACON_PROXY
#define UNPROVISIONED_IBEACON_PROXY_SEND_INTERVAL K_MSEC(100)
#define UNPROVISIONED_IBEACON_PROXY_SEND_DURATION K_MSEC(5)
#define PROVISIONED_IBEACON_PROXY_SEND_INTERVAL K_MSEC(100)
#define PROVISIONED_IBEACON_PROXY_SEND_DURATION K_MSEC(5)

extern bool bt_mesh_is_adv_sending(void);
#endif

#ifdef CONFIG_BT_MESH_GATT_PROXY

#define PDU_TYPE(data)     (data[0] & BIT_MASK(6))
#define PDU_SAR(data)      (data[0] >> 6)

#define SAR_COMPLETE       0x00
#define SAR_FIRST          0x01
#define SAR_CONT           0x02
#define SAR_LAST           0x03

#define CFG_FILTER_SET     0x00
#define CFG_FILTER_ADD     0x01
#define CFG_FILTER_REMOVE  0x02
#define CFG_FILTER_STATUS  0x03

#define PDU_HDR(sar, type) (sar << 6 | (type & BIT_MASK(6)))

#define CLIENT_BUF_SIZE 68

#if defined(CONFIG_BT_MESH_DEBUG_USE_ID_ADDR)
#define ADV_OPT (BT_LE_ADV_OPT_CONNECTABLE | BT_LE_ADV_OPT_ONE_TIME | \
		 BT_LE_ADV_OPT_USE_IDENTITY)
#else
#define ADV_OPT (BT_LE_ADV_OPT_CONNECTABLE | BT_LE_ADV_OPT_ONE_TIME)
#endif

#if defined(CONFIG_BT_MESH_PB_GATT)
static const struct bt_le_adv_param slow_adv_param = {
	.options = ADV_OPT,
#ifdef CONFIG_GENIE_MESH_ENABLE
		.interval_min = BT_GAP_ADV_FAST_INT_MIN_2,
		.interval_max = BT_GAP_ADV_FAST_INT_MAX_2,
#else
		.interval_min = BT_GAP_ADV_FAST_INT_MIN_2,
		.interval_max = BT_GAP_ADV_FAST_INT_MAX_2,
#endif
};
#endif

/*[Genie begin] add by wenbing.cwb at 2021-10-15*/
#ifdef CONFIG_BT_MESH_NPS_OPT
#define PROXY_ADV_INT_MIN BT_GAP_ADV_FAST_INT_MIN_3
#define PROXY_ADV_INT_MAX BT_GAP_ADV_FAST_INT_MAX_3
// #define PROXY_ADV_INT_MIN BT_GAP_ADV_SLOW_INT_MIN
// #define PROXY_ADV_INT_MAX BT_GAP_ADV_SLOW_INT_MAX
#endif

static const struct bt_le_adv_param unprov_fast_adv_param = {
	.options = ADV_OPT,
	.interval_min = BT_GAP_ADV_FAST_INT_MIN_2,
	.interval_max = BT_GAP_ADV_FAST_INT_MAX_2,
};

static struct bt_le_adv_param fast_adv_param = {
	.options = ADV_OPT,
	.interval_min = BT_GAP_ADV_FAST_INT_MIN_2,
	.interval_max = BT_GAP_ADV_FAST_INT_MAX_2,
};
/*[Genie end] add by wenbing.cwb at 2021-10-15*/

static bool proxy_adv_enabled;

#if defined(CONFIG_BT_MESH_GATT_PROXY)
static void proxy_send_beacons(struct k_work *work);
static u16_t proxy_ccc_val;
#endif

#if defined(CONFIG_BT_MESH_PB_GATT)
static u16_t prov_ccc_val;
static bool prov_fast_adv;
#endif

static struct bt_mesh_proxy_client {
	struct bt_conn *conn;
	u16_t filter[CONFIG_BT_MESH_PROXY_FILTER_SIZE];
	enum __packed {
		NONE,
		WHITELIST,
		BLACKLIST,
		PROV,
	} filter_type;
	u8_t msg_type;
#if defined(CONFIG_BT_MESH_GATT_PROXY)
	struct k_work send_beacons;
#endif
#if defined(CONFIG_BT_MESH_IBEACON_PROXY)
	struct k_delayed_work send_ibeacon;
#endif
	struct net_buf_simple buf;
} clients[CONFIG_BT_MAX_CONN] = {
	[0 ... (CONFIG_BT_MAX_CONN - 1)] = {
#if defined(CONFIG_BT_MESH_GATT_PROXY)
		//.send_beacons = _K_WORK_INITIALIZER(proxy_send_beacons),
#endif
	},
};

static u8_t __noinit client_buf_data[CLIENT_BUF_SIZE * CONFIG_BT_MAX_CONN];

/* Track which service is enabled */
static enum {
	MESH_GATT_NONE,
	MESH_GATT_PROV,
	MESH_GATT_PROXY,
} gatt_svc = MESH_GATT_NONE;

static struct bt_mesh_proxy_client *find_client(struct bt_conn *conn)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(clients); i++) {
		if (clients[i].conn == conn) {
			return &clients[i];
		}
	}

	return NULL;
}

#if defined(CONFIG_BT_MESH_GATT_PROXY)
/* Next subnet in queue to be advertised */
static int next_idx;

static int proxy_segment_and_send(struct bt_conn *conn, u8_t type,
				  struct net_buf_simple *msg);

static int filter_set(struct bt_mesh_proxy_client *client,
		      struct net_buf_simple *buf)
{
	u8_t type;

	if (buf->len < 1) {
		BT_WARN("Too short Filter Set message");
		return -EINVAL;
	}

	type = net_buf_simple_pull_u8(buf);
	BT_DBG("type 0x%02x", type);

	switch (type) {
	case 0x00:
		memset(client->filter, 0, sizeof(client->filter));
		client->filter_type = WHITELIST;
		break;
	case 0x01:
		memset(client->filter, 0, sizeof(client->filter));
		client->filter_type = BLACKLIST;
		break;
	default:
		BT_WARN("Prohibited Filter Type 0x%02x", type);
		return -EINVAL;
	}

	return 0;
}

static void filter_add(struct bt_mesh_proxy_client *client, u16_t addr)
{
	int i;

	BT_DBG("addr 0x%04x", addr);

	if (addr == BT_MESH_ADDR_UNASSIGNED) {
		return;
	}

	for (i = 0; i < ARRAY_SIZE(client->filter); i++) {
		if (client->filter[i] == addr) {
			return;
		}
	}

	for (i = 0; i < ARRAY_SIZE(client->filter); i++) {
		if (client->filter[i] == BT_MESH_ADDR_UNASSIGNED) {
			client->filter[i] = addr;
			return;
		}
	}
}

static void filter_remove(struct bt_mesh_proxy_client *client, u16_t addr)
{
	int i;

	BT_DBG("addr 0x%04x", addr);

	if (addr == BT_MESH_ADDR_UNASSIGNED) {
		return;
	}

	for (i = 0; i < ARRAY_SIZE(client->filter); i++) {
		if (client->filter[i] == addr) {
			client->filter[i] = BT_MESH_ADDR_UNASSIGNED;
			return;
		}
	}
}

static void send_filter_status(struct bt_mesh_proxy_client *client,
			       struct bt_mesh_net_rx *rx,
			       struct net_buf_simple *buf)
{
	struct bt_mesh_net_tx tx = {
		.sub = rx->sub,
		.ctx = &rx->ctx,
		.src = bt_mesh_primary_addr(),
	};
	u16_t filter_size;
	int i, err;

	/* Configuration messages always have dst unassigned */
	tx.ctx->addr = BT_MESH_ADDR_UNASSIGNED;

	net_buf_simple_reset(buf);
	net_buf_simple_reserve(buf, 10);

	net_buf_simple_add_u8(buf, CFG_FILTER_STATUS);

	if (client->filter_type == WHITELIST) {
		net_buf_simple_add_u8(buf, 0x00);
	} else {
		net_buf_simple_add_u8(buf, 0x01);
	}

	for (filter_size = 0, i = 0; i < ARRAY_SIZE(client->filter); i++) {
		if (client->filter[i] != BT_MESH_ADDR_UNASSIGNED) {
			filter_size++;
		}
	}

	net_buf_simple_add_be16(buf, filter_size);

	BT_DBG("%u bytes: %s", buf->len, bt_hex(buf->data, buf->len));

	err = bt_mesh_net_encode(&tx, buf, true);
	if (err) {
		BT_ERR("Encoding Proxy cfg message failed (err %d)", err);
		return;
	}

	err = proxy_segment_and_send(client->conn, BT_MESH_PROXY_CONFIG, buf);
	if (err) {
		BT_ERR("Failed to send proxy cfg message (err %d)", err);
	}
}

static void proxy_cfg(struct bt_mesh_proxy_client *client)
{
	struct bt_mesh_net_rx rx;
	u8_t opcode;
	int err;

	if (client->buf.len > BT_MESH_NET_MAX_PDU_LEN) {
		BT_ERR("Proxy cfg data is too long %d(%d)", client->buf.len, BT_MESH_NET_MAX_PDU_LEN);
		return;
	}

	NET_BUF_SIMPLE_DEFINE(buf, BT_MESH_NET_MAX_PDU_LEN);

	err = bt_mesh_net_decode(&client->buf, BT_MESH_NET_IF_PROXY_CFG,
				 &rx, &buf);
	if (err) {
		BT_ERR("Failed to decode Proxy Configuration (err %d)", err);
		return;
	}

	/* Remove network headers */
	net_buf_simple_pull(&buf, BT_MESH_NET_HDR_LEN);

	BT_DBG("%u bytes: %s", buf.len, bt_hex(buf.data, buf.len));

	if (buf.len < 1) {
		BT_WARN("Too short proxy configuration PDU");
		return;
	}

	opcode = net_buf_simple_pull_u8(&buf);
	switch (opcode) {
	case CFG_FILTER_SET:
		filter_set(client, &buf);
		send_filter_status(client, &rx, &buf);
		break;
	case CFG_FILTER_ADD:
		while (buf.len >= 2) {
			u16_t addr;

			addr = net_buf_simple_pull_be16(&buf);
			filter_add(client, addr);
		}
		send_filter_status(client, &rx, &buf);
		break;
	case CFG_FILTER_REMOVE:
		while (buf.len >= 2) {
			u16_t addr;

			addr = net_buf_simple_pull_be16(&buf);
			filter_remove(client, addr);
		}
		send_filter_status(client, &rx, &buf);
		break;
	default:
		BT_WARN("Unhandled configuration OpCode 0x%02x", opcode);
		break;
	}
}

static int beacon_send(struct bt_conn *conn, struct bt_mesh_subnet *sub)
{
	NET_BUF_SIMPLE_DEFINE(buf, 23);

	net_buf_simple_reserve(&buf, 1);
	bt_mesh_beacon_create(sub, &buf);

	return proxy_segment_and_send(conn, BT_MESH_PROXY_BEACON, &buf);
}

static void proxy_send_beacons(struct k_work *work)
{
	struct bt_mesh_proxy_client *client;
	int i;

	client = CONTAINER_OF(work, struct bt_mesh_proxy_client, send_beacons);

	for (i = 0; i < ARRAY_SIZE(bt_mesh.sub); i++) {
		struct bt_mesh_subnet *sub = &bt_mesh.sub[i];

		if (sub->net_idx != BT_MESH_KEY_UNUSED) {
			beacon_send(client->conn, sub);
		}
	}
}

#ifdef CONFIG_BT_MESH_IBEACON_PROXY
static void proxy_send_ibeacon(struct k_work *work)
{
	static bool is_proxy_sending = true;
	uint32_t delay_work_timeout = 0;
	struct bt_mesh_proxy_client *client;

	client = CONTAINER_OF(work, struct bt_mesh_proxy_client, send_ibeacon);

	if (is_proxy_sending)
	{
		bt_mesh_proxy_adv_start();
	}

	if (is_proxy_sending)
	{
		if (bt_mesh_is_provisioned())
		{
			delay_work_timeout = PROVISIONED_IBEACON_PROXY_SEND_DURATION;
		}
		else
		{
			delay_work_timeout = UNPROVISIONED_IBEACON_PROXY_SEND_DURATION;
		}
	}
	else
	{
		if (bt_mesh_is_provisioned())
		{
			delay_work_timeout = PROVISIONED_IBEACON_PROXY_SEND_INTERVAL - PROVISIONED_IBEACON_PROXY_SEND_DURATION;
		}
		else
		{
			delay_work_timeout = UNPROVISIONED_IBEACON_PROXY_SEND_INTERVAL - UNPROVISIONED_IBEACON_PROXY_SEND_DURATION;
		}
	}

	k_delayed_work_submit(&client->send_ibeacon, delay_work_timeout);

	if (!is_proxy_sending && !bt_mesh_is_adv_sending())
	{
		bt_mesh_proxy_adv_stop();
	}

	is_proxy_sending = !is_proxy_sending;
}
#endif

void bt_mesh_proxy_beacon_send(struct bt_mesh_subnet *sub)
{
	int i;

	if (!sub) {
		/* NULL means we send on all subnets */
		for (i = 0; i < ARRAY_SIZE(bt_mesh.sub); i++) {
			if (bt_mesh.sub[i].net_idx != BT_MESH_KEY_UNUSED) {
				bt_mesh_proxy_beacon_send(&bt_mesh.sub[i]);
			}
		}

		return;
	}

	for (i = 0; i < ARRAY_SIZE(clients); i++) {
		if (clients[i].conn) {
			beacon_send(clients[i].conn, sub);
		}
	}
}

void bt_mesh_proxy_identity_start(struct bt_mesh_subnet *sub)
{
	sub->node_id = BT_MESH_NODE_IDENTITY_RUNNING;
	sub->node_id_start = k_uptime_get_32();

	/* Prioritize the recently enabled subnet */
	next_idx = sub - bt_mesh.sub;
}

void bt_mesh_proxy_identity_stop(struct bt_mesh_subnet *sub)
{
	sub->node_id = BT_MESH_NODE_IDENTITY_STOPPED;
	sub->node_id_start = 0;
}

int bt_mesh_proxy_identity_enable(void)
{
	int i, count = 0;

	BT_DBG("");

	if (!bt_mesh_is_provisioned()) {
		return -EAGAIN;
	}

	for (i = 0; i < ARRAY_SIZE(bt_mesh.sub); i++) {
		struct bt_mesh_subnet *sub = &bt_mesh.sub[i];

		if (sub->net_idx == BT_MESH_KEY_UNUSED) {
			continue;
		}

		if (sub->node_id == BT_MESH_NODE_IDENTITY_NOT_SUPPORTED) {
			continue;
		}

		bt_mesh_proxy_identity_start(sub);
		count++;
	}

	if (count) {
		bt_mesh_adv_update();
	}

	return 0;
}

int bt_mesh_proxy_identity_disable(void)
{
	int i, count = 0;

	BT_DBG("");

	if (!bt_mesh_is_provisioned()) {
		return -EAGAIN;
	}

	for (i = 0; i < ARRAY_SIZE(bt_mesh.sub); i++) {
		struct bt_mesh_subnet *sub = &bt_mesh.sub[i];

		if (sub->net_idx == BT_MESH_KEY_UNUSED) {
			continue;
		}

		if (sub->node_id == BT_MESH_NODE_IDENTITY_NOT_SUPPORTED) {
			continue;
		}

		bt_mesh_proxy_identity_stop(sub);
		count++;
	}

	if (count) {
		bt_mesh_adv_update();
	}

	return 0;
}

#endif /* GATT_PROXY */

static void proxy_complete_pdu(struct bt_mesh_proxy_client *client)
{
	switch (client->msg_type) {
#if defined(CONFIG_BT_MESH_GATT_PROXY)
	case BT_MESH_PROXY_NET_PDU:
		BT_DBG("Mesh Network PDU");
		bt_mesh_net_recv(&client->buf, 0, BT_MESH_NET_IF_PROXY);
		break;
	case BT_MESH_PROXY_BEACON:
		BT_DBG("Mesh Beacon PDU");
		bt_mesh_beacon_recv(&client->buf);
		break;
	case BT_MESH_PROXY_CONFIG:
		BT_DBG("Mesh Configuration PDU");
		proxy_cfg(client);
		break;
#endif
#if defined(CONFIG_BT_MESH_PB_GATT)
	case BT_MESH_PROXY_PROV:
		BT_DBG("Mesh Provisioning PDU");
		bt_mesh_pb_gatt_recv(client->conn, &client->buf);
		break;
#endif
	default:
		BT_WARN("Unhandled Message Type 0x%02x", client->msg_type);
		break;
	}

	net_buf_simple_reset(&client->buf);
}

#define ATTR_IS_PROV(attr) (attr->user_data != NULL)

static ssize_t proxy_recv(struct bt_conn *conn,
			  const struct bt_gatt_attr *attr, const void *buf,
			  u16_t len, u16_t offset, u8_t flags)
{
	struct bt_mesh_proxy_client *client = find_client(conn);
	const u8_t *data = buf;

	if (!client) {
		return -ENOTCONN;
	}

	if (len < 1) {
		BT_WARN("Too small Proxy PDU");
		return -EINVAL;
	}

	if (ATTR_IS_PROV(attr) != (PDU_TYPE(data) == BT_MESH_PROXY_PROV)) {
		BT_WARN("Proxy PDU type doesn't match GATT service");
		return -EINVAL;
	}

	if (len - 1 > net_buf_simple_tailroom(&client->buf)) {
		BT_WARN("Too big proxy PDU");
		return -EINVAL;
	}

	switch (PDU_SAR(data)) {
	case SAR_COMPLETE:
		if (client->buf.len) {
			BT_WARN("Complete PDU while a pending incomplete one");
			return -EINVAL;
		}

		client->msg_type = PDU_TYPE(data);
		net_buf_simple_add_mem(&client->buf, data + 1, len - 1);
		proxy_complete_pdu(client);
		break;

	case SAR_FIRST:
		if (client->buf.len) {
			BT_WARN("First PDU while a pending incomplete one");
			return -EINVAL;
		}

		client->msg_type = PDU_TYPE(data);
		net_buf_simple_add_mem(&client->buf, data + 1, len - 1);
		break;

	case SAR_CONT:
		if (!client->buf.len) {
			BT_WARN("Continuation with no prior data");
			return -EINVAL;
		}

		if (client->msg_type != PDU_TYPE(data)) {
			BT_WARN("Unexpected message type in continuation");
			return -EINVAL;
		}

		net_buf_simple_add_mem(&client->buf, data + 1, len - 1);
		break;

	case SAR_LAST:
		if (!client->buf.len) {
			BT_WARN("Last SAR PDU with no prior data");
			return -EINVAL;
		}

		if (client->msg_type != PDU_TYPE(data)) {
			BT_WARN("Unexpected message type in last SAR PDU");
			return -EINVAL;
		}

		net_buf_simple_add_mem(&client->buf, data + 1, len - 1);
		proxy_complete_pdu(client);
		break;
	}

	return len;
}

static int conn_count;

uint8_t is_proxy_connected()
{
  return conn_count > 0 ? 1 : 0 ;
}

static void proxy_connected(struct bt_conn *conn, u8_t err)
{
    if(conn->role == BT_HCI_ROLE_MASTER) {
        return;
    }
    struct bt_mesh_proxy_client *client;
    int i;

	conn_count++;
	BT_DBG("pconn:%p\r\n", conn);

	/* Since we use ADV_OPT_ONE_TIME */
	proxy_adv_enabled = false;

#if !(defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
	extern int bt_mesh_adv_disable();
	//make sure adv and scan stop first
	bt_mesh_adv_disable();
	bt_mesh_scan_disable();
#endif

    //enable scan again
	bt_mesh_scan_enable();

	/* Try to re-enable advertising in case it's possible */
	if (conn_count < CONFIG_BT_MAX_CONN) {
		bt_mesh_adv_update();
	}

	for (client = NULL, i = 0; i < ARRAY_SIZE(clients); i++) {
		if (!clients[i].conn) {
			client = &clients[i];
			break;
		}
	}

	if (!client) {
		BT_ERR("No free Proxy Client objects");
		return;
	}

	client->conn = bt_conn_ref(conn);
	client->filter_type = NONE;
	memset(client->filter, 0, sizeof(client->filter));
	net_buf_simple_reset(&client->buf);

#ifdef CONFIG_GENIE_OTA
	genie_ais_connect((struct bt_conn *)conn);
#endif

}

static void proxy_disconnected(struct bt_conn *conn, u8_t reason)
{
    if(conn->role == BT_HCI_ROLE_MASTER) {
        return;
    }
    int i;

	BT_DBG("proxy disconn:%p reason:0x%02x", conn, reason);

	conn_count--;

	for (i = 0; i < ARRAY_SIZE(clients); i++) {
		struct bt_mesh_proxy_client *client = &clients[i];

		if (client->conn == conn) {
			if (IS_ENABLED(CONFIG_BT_MESH_PB_GATT) &&
			    client->filter_type == PROV) {
				bt_mesh_pb_gatt_close(conn);
			}

			bt_conn_unref(client->conn);
			client->conn = NULL;
			break;
		}
	}

	bt_mesh_adv_update();
#ifdef CONFIG_GENIE_OTA
	genie_ais_disconnect(0);
#endif

}

struct net_buf_simple *bt_mesh_proxy_get_buf(void)
{
	struct net_buf_simple *buf = &clients[0].buf;

	net_buf_simple_reset(buf);

	return buf;
}

#if defined(CONFIG_BT_MESH_PB_GATT)
static ssize_t prov_ccc_write(struct bt_conn *conn,
			      const struct bt_gatt_attr *attr,
			      const void *buf, u16_t len,
			      u16_t offset, u8_t flags)
{
	struct bt_mesh_proxy_client *client;
	u16_t *value = attr->user_data;

	BT_DBG("len %u: %s", len, bt_hex(buf, len));

	if (len != sizeof(*value)) {
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_ATTRIBUTE_LEN);
	}

	*value = sys_get_le16(buf);
	if (*value != BT_GATT_CCC_NOTIFY) {
		BT_WARN("Client wrote 0x%04x instead enabling notify", *value);
		return len;
	}

	/* If a connection exists there must be a client */
	client = find_client(conn);
	__ASSERT(client, "No client for connection");

	if (client->filter_type == NONE) {
		client->filter_type = PROV;
		bt_mesh_pb_gatt_open(conn);
	}

	return len;
}

static ssize_t prov_ccc_read(struct bt_conn *conn,
			     const struct bt_gatt_attr *attr,
			     void *buf, u16_t len, u16_t offset)
{
	u16_t *value = attr->user_data;

	return bt_gatt_attr_read(conn, attr, buf, len, offset, value,
				 sizeof(*value));
}

/* Mesh Provisioning Service Declaration */
static struct bt_gatt_attr prov_attrs[] = {
	BT_GATT_PRIMARY_SERVICE(BT_UUID_MESH_PROV),

	BT_GATT_CHARACTERISTIC(BT_UUID_MESH_PROV_DATA_IN,
			       BT_GATT_CHRC_WRITE_WITHOUT_RESP,
			       BT_GATT_PERM_WRITE, NULL, proxy_recv,
			       (void *)1),

	BT_GATT_CHARACTERISTIC(BT_UUID_MESH_PROV_DATA_OUT,
			       BT_GATT_CHRC_NOTIFY, BT_GATT_PERM_NONE,
			       NULL, NULL, NULL),
	/* Add custom CCC as clients need to be tracked individually */
	BT_GATT_DESCRIPTOR(BT_UUID_GATT_CCC,
			   BT_GATT_PERM_WRITE | BT_GATT_PERM_READ,
			   prov_ccc_read, prov_ccc_write, &prov_ccc_val),
};

static struct bt_gatt_service prov_svc = BT_GATT_SERVICE(prov_attrs);

int bt_mesh_proxy_prov_enable(void)
{
	int i;

	BT_DBG("");

	if (gatt_svc == MESH_GATT_PROV) {
		return -EALREADY;
	}

	if (gatt_svc != MESH_GATT_NONE) {
		return -EBUSY;
	}

	bt_gatt_service_register(&prov_svc);
	gatt_svc = MESH_GATT_PROV;
	prov_fast_adv = true;

	for (i = 0; i < ARRAY_SIZE(clients); i++) {
		if (clients[i].conn) {
			clients[i].filter_type = PROV;
		}
	}


	return 0;
}

int bt_mesh_proxy_prov_disable(bool disconnect)
{
	int i;

	BT_DBG("");

	if (gatt_svc == MESH_GATT_NONE) {
		return -EALREADY;
	}

	if (gatt_svc != MESH_GATT_PROV) {
		return -EBUSY;
	}

	bt_gatt_service_unregister(&prov_svc);
	gatt_svc = MESH_GATT_NONE;

	for (i = 0; i < ARRAY_SIZE(clients); i++) {
		struct bt_mesh_proxy_client *client = &clients[i];

		if (!client->conn || client->filter_type != PROV) {
			continue;
		}

		if (disconnect) {
			bt_conn_disconnect(client->conn,
					   BT_HCI_ERR_REMOTE_USER_TERM_CONN);
		} else {
			bt_mesh_pb_gatt_close(client->conn);
			client->filter_type = NONE;
		}
	}

	bt_mesh_adv_update();

	return 0;
}

#endif /* CONFIG_BT_MESH_PB_GATT */

#if defined(CONFIG_BT_MESH_GATT_PROXY)
static ssize_t proxy_ccc_write(struct bt_conn *conn,
			       const struct bt_gatt_attr *attr,
			       const void *buf, u16_t len,
			       u16_t offset, u8_t flags)
{
	struct bt_mesh_proxy_client *client;
	u16_t value;

	BT_DBG("len %u: %s", len, bt_hex(buf, len));

	if (len != sizeof(value)) {
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_ATTRIBUTE_LEN);
	}

	value = sys_get_le16(buf);
	if (value != BT_GATT_CCC_NOTIFY) {
		BT_WARN("Client wrote 0x%04x instead enabling notify", value);
		return len;
	}

	/* If a connection exists there must be a client */
	client = find_client(conn);
	__ASSERT(client, "No client for connection");

	if (client->filter_type == NONE) {
		client->filter_type = WHITELIST;
		k_work_submit(&client->send_beacons);
	}

	return len;
}

static ssize_t proxy_ccc_read(struct bt_conn *conn,
			      const struct bt_gatt_attr *attr,
			      void *buf, u16_t len, u16_t offset)
{
	u16_t *value = attr->user_data;

	return bt_gatt_attr_read(conn, attr, buf, len, offset, value,
				 sizeof(*value));
}

/* Mesh Proxy Service Declaration */
static struct bt_gatt_attr proxy_attrs[] = {
	BT_GATT_PRIMARY_SERVICE(BT_UUID_MESH_PROXY),

	BT_GATT_CHARACTERISTIC(BT_UUID_MESH_PROXY_DATA_IN,
			       BT_GATT_CHRC_WRITE_WITHOUT_RESP,
			       BT_GATT_PERM_WRITE,
			       NULL, proxy_recv, NULL),

	BT_GATT_CHARACTERISTIC(BT_UUID_MESH_PROXY_DATA_OUT,
			       BT_GATT_CHRC_NOTIFY,
			       BT_GATT_PERM_NONE,
			       NULL, NULL, NULL),
	/* Add custom CCC as clients need to be tracked individually */
	BT_GATT_DESCRIPTOR(BT_UUID_GATT_CCC,
			   BT_GATT_PERM_READ | BT_GATT_PERM_WRITE,
			   proxy_ccc_read, proxy_ccc_write, &proxy_ccc_val),
};

static struct bt_gatt_service proxy_svc = BT_GATT_SERVICE(proxy_attrs);

int bt_mesh_proxy_gatt_enable(void)
{
	int i;

	BT_DBG("");

	if (gatt_svc == MESH_GATT_PROXY) {
		return -EALREADY;
	}

	if (gatt_svc != MESH_GATT_NONE) {
		return -EBUSY;
	}

	bt_gatt_service_register(&proxy_svc);
	gatt_svc = MESH_GATT_PROXY;

	for (i = 0; i < ARRAY_SIZE(clients); i++) {
		if (clients[i].conn) {
			clients[i].filter_type = WHITELIST;
		}
	}

	return 0;
}

void bt_mesh_proxy_gatt_disconnect(void)
{
	int i;

	BT_DBG("");

	for (i = 0; i < ARRAY_SIZE(clients); i++) {
		struct bt_mesh_proxy_client *client = &clients[i];

		if (client->conn && (client->filter_type == WHITELIST ||
				     client->filter_type == BLACKLIST)) {
			client->filter_type = NONE;
			bt_conn_disconnect(client->conn,
					   BT_HCI_ERR_REMOTE_USER_TERM_CONN);
		}
	}
}

int bt_mesh_proxy_gatt_disable(void)
{
	BT_DBG("");

	if (gatt_svc == MESH_GATT_NONE) {
		return -EALREADY;
	}

	if (gatt_svc != MESH_GATT_PROXY) {
		return -EBUSY;
	}

	bt_mesh_proxy_gatt_disconnect();

	bt_gatt_service_unregister(&proxy_svc);
	gatt_svc = MESH_GATT_NONE;

	return 0;
}

void bt_mesh_proxy_addr_add(struct net_buf_simple *buf, u16_t addr)
{
	struct bt_mesh_proxy_client *client =
		CONTAINER_OF(buf, struct bt_mesh_proxy_client, buf);

	BT_DBG("filter_type %u addr 0x%04x", client->filter_type, addr);

	if (client->filter_type == WHITELIST) {
		filter_add(client, addr);
	} else if (client->filter_type == BLACKLIST) {
		filter_remove(client, addr);
	}
}

static bool client_filter_match(struct bt_mesh_proxy_client *client,
				u16_t addr)
{
	int i;

	BT_DBG("filter_type %u addr 0x%04x", client->filter_type, addr);

	if (client->filter_type == WHITELIST) {
		for (i = 0; i < ARRAY_SIZE(client->filter); i++) {
			if (client->filter[i] == addr) {
				return true;
			}
		}
#ifdef CONFIG_GENIE_MESH_ENABLE
        /*[Genie begin] add by lgy at 2020-09-10*/
        /* add tmall genie groupcast address in filter list by default */
        if (addr == BT_MESH_ADDR_TMALL_GENIE)
        {
            return true;
        }
        /*[Genie end] add by lgy at 2020-09-10*/
#endif
		return false;
	}

	if (client->filter_type == BLACKLIST) {
		for (i = 0; i < ARRAY_SIZE(client->filter); i++) {
			if (client->filter[i] == addr) {
				return false;
			}
		}

		return true;
	}

	return false;
}

bool bt_mesh_proxy_relay(struct net_buf_simple *buf, u16_t dst)
{
	bool relayed = false;
	int i;

	BT_DBG("%u bytes to dst 0x%04x", buf->len, dst);

	for (i = 0; i < ARRAY_SIZE(clients); i++) {
		struct bt_mesh_proxy_client *client = &clients[i];
		NET_BUF_SIMPLE_DEFINE(msg, 32);

		if (!client->conn) {
			continue;
		}

		if (!client_filter_match(client, dst)) {
			continue;
		}

		/* Proxy PDU sending modifies the original buffer,
		 * so we need to make a copy.
		 */
		net_buf_simple_reserve(&msg, 1);
		net_buf_simple_add_mem(&msg, buf->data, buf->len);

		bt_mesh_proxy_send(client->conn, BT_MESH_PROXY_NET_PDU, &msg);
		relayed = true;
	}

	return relayed;
}

#endif /* CONFIG_BT_MESH_GATT_PROXY */

static int proxy_send(struct bt_conn *conn, const void *data, u16_t len)
{
	BT_DBG("%u bytes: %s", len, bt_hex(data, len));

	struct bt_gatt_notify_params params = {0};
	u16_t handle = 0;

	params.data = data;
	params.len = len;

#if defined(CONFIG_BT_MESH_GATT_PROXY)
	if (gatt_svc == MESH_GATT_PROXY) {
	    handle = bt_gatt_attr_value_handle(&proxy_attrs[3]);
	}
#endif

#if defined(CONFIG_BT_MESH_PB_GATT)
	if (gatt_svc == MESH_GATT_PROV) {
		handle = bt_gatt_attr_value_handle(&prov_attrs[3]);
	}
#endif
    extern int gatt_notify(struct bt_conn *conn, u16_t handle,
		       struct bt_gatt_notify_params *params);

	if (handle)
	{
		return gatt_notify(conn, handle, &params);
	}

	return 0;
}

static int proxy_segment_and_send(struct bt_conn *conn, u8_t type,
				  struct net_buf_simple *msg)
{
	u16_t mtu;

	BT_DBG("conn %p type 0x%02x len %u: %s", conn, type, msg->len,
	       bt_hex(msg->data, msg->len));

	/* ATT_MTU - OpCode (1 byte) - Handle (2 bytes) */
	mtu = bt_gatt_get_mtu(conn) - 3;
	if (mtu > msg->len) {
		net_buf_simple_push_u8(msg, PDU_HDR(SAR_COMPLETE, type));
		return proxy_send(conn, msg->data, msg->len);
	}

	net_buf_simple_push_u8(msg, PDU_HDR(SAR_FIRST, type));
	proxy_send(conn, msg->data, mtu);
	net_buf_simple_pull(msg, mtu);

	while (msg->len) {
		if (msg->len + 1 < mtu) {
			net_buf_simple_push_u8(msg, PDU_HDR(SAR_LAST, type));
			proxy_send(conn, msg->data, msg->len);
			break;
		}

		net_buf_simple_push_u8(msg, PDU_HDR(SAR_CONT, type));
		proxy_send(conn, msg->data, mtu);
		net_buf_simple_pull(msg, mtu);
	}

	return 0;
}

int bt_mesh_proxy_send(struct bt_conn *conn, u8_t type,
		       struct net_buf_simple *msg)
{
	struct bt_mesh_proxy_client *client = find_client(conn);

	if (!client) {
		BT_ERR("No Proxy Client found");
		return -ENOTCONN;
	}

	if ((client->filter_type == PROV) != (type == BT_MESH_PROXY_PROV)) {
		BT_ERR("Invalid PDU type for Proxy Client");
		return -EINVAL;
	}

	return proxy_segment_and_send(conn, type, msg);
}

#if defined(CONFIG_BT_MESH_PB_GATT)
static u8_t prov_svc_data[20] = { 0x27, 0x18, };

static const struct bt_data prov_ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA_BYTES(BT_DATA_UUID16_ALL, 0x27, 0x18),
	BT_DATA(BT_DATA_SVC_DATA16, prov_svc_data, sizeof(prov_svc_data)),
};

#ifdef CONFIG_GENIE_OTA
static u8_t g_ais_adv_data[14] = {
    0xa8, 0x01, //taobao
    0x85,       //vid & sub
    0x15,       //FMSK
    0x15, 0x11, 0x22, 0x33,             //PID
    0xAA, 0xBB, 0xCC, 0x11, 0x22, 0x33  //MAC
};

struct bt_data ais_prov_sd[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA_BYTES(BT_DATA_UUID16_SOME, 0xB3, 0xFE),
    BT_DATA(BT_DATA_MANUFACTURER_DATA, g_ais_adv_data, 14),
};
#endif

#endif /* PB_GATT */

#if defined(CONFIG_BT_MESH_GATT_PROXY)

#define ID_TYPE_NET  0x00
#define ID_TYPE_NODE 0x01

#define NODE_ID_LEN  19
#define NET_ID_LEN   11

#define NODE_ID_TIMEOUT K_SECONDS(CONFIG_BT_MESH_NODE_ID_TIMEOUT)

static u8_t proxy_svc_data[NODE_ID_LEN] = { 0x28, 0x18, };

static const struct bt_data node_id_ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA_BYTES(BT_DATA_UUID16_ALL, 0x28, 0x18),
	BT_DATA(BT_DATA_SVC_DATA16, proxy_svc_data, NODE_ID_LEN),
};

static const struct bt_data net_id_ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA_BYTES(BT_DATA_UUID16_ALL, 0x28, 0x18),
	BT_DATA(BT_DATA_SVC_DATA16, proxy_svc_data, NET_ID_LEN),
};

/*[Genie begin] add by wenbing.cwb at 2021-10-19*/
#ifdef CONFIG_BT_MESH_NPS_OPT
static void proxy_adv_interval_set(void)
{
	if (genie_provision_get_state() == GENIE_PROVISION_SUCCESS)
	{
		fast_adv_param.interval_min = PROXY_ADV_INT_MIN;
		fast_adv_param.interval_max = PROXY_ADV_INT_MAX;
	}
	else
	{
		fast_adv_param.interval_min = BT_GAP_ADV_FAST_INT_MIN_2;
		fast_adv_param.interval_max = BT_GAP_ADV_FAST_INT_MAX_2;
	}
}
#endif
/*[Genie end] add by wenbing.cwb at 2021-10-19*/

static int node_id_adv(struct bt_mesh_subnet *sub)
{
	u8_t tmp[16];
	int err;

	BT_DBG("");

	proxy_svc_data[2] = ID_TYPE_NODE;

	err = bt_rand(proxy_svc_data + 11, 8);
	if (err) {
		return err;
	}

	memset(tmp, 0, 6);
	memcpy(tmp + 6, proxy_svc_data + 11, 8);
	sys_put_be16(bt_mesh_primary_addr(), tmp + 14);

	err = bt_encrypt_be(sub->keys[sub->kr_flag].identity, tmp, tmp);
	if (err) {
		return err;
	}

	memcpy(proxy_svc_data + 3, tmp + 8, 8);

/*[Genie begin] add by wenbing.cwb at 2021-10-19*/
#ifdef CONFIG_BT_MESH_NPS_OPT
	proxy_adv_interval_set();
#endif
/*[Genie end] add by wenbing.cwb at 2021-10-19*/

#ifdef CONFIG_GENIE_OTA
    genie_crypto_adv_create(g_ais_adv_data, 1);
    err = bt_mesh_adv_enable(&fast_adv_param, node_id_ad,
                             ARRAY_SIZE(node_id_ad), ais_prov_sd, ARRAY_SIZE(ais_prov_sd));
#else
    err = bt_mesh_adv_enable(&fast_adv_param, node_id_ad,
                             ARRAY_SIZE(node_id_ad), NULL, 0);
#endif
	if (err) {
		BT_WARN("Failed to advertise using Node ID (err %d)", err);
		return err;
	}

	proxy_adv_enabled = true;

	return 0;
}

static int net_id_adv(struct bt_mesh_subnet *sub)
{
	int err;

	BT_DBG("");

	proxy_svc_data[2] = ID_TYPE_NET;

	BT_DBG("Advertising with NetId %s",
	       bt_hex(sub->keys[sub->kr_flag].net_id, 8));

	memcpy(proxy_svc_data + 3, sub->keys[sub->kr_flag].net_id, 8);

/*[Genie begin] add by wenbing.cwb at 2021-10-19*/
#ifdef CONFIG_BT_MESH_NPS_OPT
	proxy_adv_interval_set();
#endif
/*[Genie end] add by wenbing.cwb at 2021-10-19*/

#ifdef CONFIG_GENIE_OTA
    genie_crypto_adv_create(g_ais_adv_data, 1);
    err = bt_mesh_adv_enable(&fast_adv_param, net_id_ad,
                             ARRAY_SIZE(net_id_ad),ais_prov_sd, ARRAY_SIZE(ais_prov_sd));
#else
    err = bt_mesh_adv_enable(&fast_adv_param, net_id_ad,
                             ARRAY_SIZE(net_id_ad), NULL, 0);
#endif

	if (err) {
		BT_WARN("Failed to advertise using Network ID (err %d)", err);
		return err;
	}

	proxy_adv_enabled = true;

	return 0;
}

bool bt_mesh_proxy_adv_enable()
{
	return proxy_adv_enabled;
}

static bool advertise_subnet(struct bt_mesh_subnet *sub)
{
	if (sub->net_idx == BT_MESH_KEY_UNUSED) {
		return false;
	}

	return (sub->node_id == BT_MESH_NODE_IDENTITY_RUNNING ||
		bt_mesh_gatt_proxy_get() == BT_MESH_GATT_PROXY_ENABLED);
}

static struct bt_mesh_subnet *next_sub(void)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(bt_mesh.sub); i++) {
		struct bt_mesh_subnet *sub;

		sub = &bt_mesh.sub[(i + next_idx) % ARRAY_SIZE(bt_mesh.sub)];
		if (advertise_subnet(sub)) {
			next_idx = (next_idx + 1) % ARRAY_SIZE(bt_mesh.sub);
			return sub;
		}
	}

	return NULL;
}

static int sub_count(void)
{
	int i, count = 0;

	for (i = 0; i < ARRAY_SIZE(bt_mesh.sub); i++) {
		struct bt_mesh_subnet *sub = &bt_mesh.sub[i];

		if (advertise_subnet(sub)) {
			count++;
		}
	}

	return count;
}

static s32_t gatt_proxy_advertise(struct bt_mesh_subnet *sub)
{
	s32_t remaining = K_FOREVER;
	int subnet_count;

	BT_DBG("");

	if (conn_count == CONFIG_BT_MAX_CONN) {
		BT_WARN("Connectable advertising deferred (max connections)");
		return remaining;
	}

	if (!sub) {
		BT_DBG("No subnets to advertise on");
		return remaining;
	}

	if (sub->node_id == BT_MESH_NODE_IDENTITY_RUNNING) {
		u32_t active = k_uptime_get_32() - sub->node_id_start;

		if (active < NODE_ID_TIMEOUT) {
			remaining = NODE_ID_TIMEOUT - active;
			BT_DBG("Node ID active for %u ms, %d ms remaining",
			       active, remaining);
			node_id_adv(sub);
		} else {
			bt_mesh_proxy_identity_stop(sub);
			BT_DBG("Node ID stopped");
		}
	}

	if (sub->node_id == BT_MESH_NODE_IDENTITY_STOPPED) {
		if (bt_mesh_gatt_proxy_get() == BT_MESH_GATT_PROXY_ENABLED) {
			net_id_adv(sub);
		} else {
			return gatt_proxy_advertise(next_sub());
		}
	}

	subnet_count = sub_count();
	BT_DBG("sub_count %u", subnet_count);
	if (subnet_count > 1) {
		s32_t max_timeout;

		/* We use NODE_ID_TIMEOUT as a starting point since it may
		 * be less than 60 seconds. Divide this period into at least
		 * 6 slices, but make sure that a slice is at least one
		 * second long (to avoid excessive rotation).
		 */
		max_timeout = NODE_ID_TIMEOUT / MAX(subnet_count, 6);
		max_timeout = MAX(max_timeout, K_SECONDS(1));

		if (remaining > max_timeout || remaining < 0) {
			remaining = max_timeout;
		}
	}

	BT_DBG("Advertising %d ms for net_idx 0x%04x", remaining, sub->net_idx);

	return remaining;
}
#endif /* GATT_PROXY */

#if defined(CONFIG_BT_MESH_PB_GATT)
static size_t gatt_prov_adv_create(struct bt_data prov_sd[2])
{
	const struct bt_mesh_prov *prov = bt_mesh_prov_get();
	const char *name = bt_get_name();
	size_t name_len = strlen(name);
	size_t prov_sd_len = 0;
	size_t sd_space = 31;

	memcpy(prov_svc_data + 2, prov->uuid, 16);
	sys_put_be16(prov->oob_info, prov_svc_data + 18);

	if (prov->uri) {
		size_t uri_len = strlen(prov->uri);

		if (uri_len > 29) {
			/* There's no way to shorten an URI */
			BT_WARN("Too long URI to fit advertising packet");
		} else {
			prov_sd[0].type = BT_DATA_URI;
			prov_sd[0].data_len = uri_len;
			prov_sd[0].data = (const u8_t *)prov->uri;
			sd_space -= 2 + uri_len;
			prov_sd_len++;
		}
	}

	if (sd_space > 2 && name_len > 0) {
		sd_space -= 2;

		if (sd_space < name_len) {
			prov_sd[prov_sd_len].type = BT_DATA_NAME_SHORTENED;
			prov_sd[prov_sd_len].data_len = sd_space;
		} else {
			prov_sd[prov_sd_len].type = BT_DATA_NAME_COMPLETE;
			prov_sd[prov_sd_len].data_len = name_len;
		}

		prov_sd[prov_sd_len].data = (const u8_t *)name;
		prov_sd_len++;
	}

	return prov_sd_len;
}
#endif /* CONFIG_BT_MESH_PB_GATT */

s32_t bt_mesh_proxy_adv_start(void)
{
	BT_DBG("");
	proxy_adv_enabled = false;
	if (gatt_svc == MESH_GATT_NONE) {
		return K_FOREVER;
	}

#if defined(CONFIG_BT_MESH_PB_GATT)
	int err;
	if (!bt_mesh_is_provisioned() && (conn_count < CONFIG_BT_MAX_CONN)) {
		const struct bt_le_adv_param *param;
		static struct bt_data prov_sd[2];
#ifndef CONFIG_GENIE_OTA
		static size_t prov_sd_len;
#endif
		if (prov_fast_adv) {
			param = &unprov_fast_adv_param;
		} else {
			param = &slow_adv_param;
		}
#ifndef CONFIG_GENIE_OTA
		prov_sd_len = gatt_prov_adv_create(prov_sd);
#else
		gatt_prov_adv_create(prov_sd);
#endif
/*[Genie begin] add by wenbing.cwb at 2021-10-19*/
#ifdef CONFIG_BT_MESH_NPS_OPT
		err = bt_mesh_adv_enable(param, prov_ad, ARRAY_SIZE(prov_ad), NULL, 0);
#else
#ifdef CONFIG_GENIE_OTA
		genie_crypto_adv_create(g_ais_adv_data, 0);
		err = bt_mesh_adv_enable(param, prov_ad, ARRAY_SIZE(prov_ad),
				ais_prov_sd, ARRAY_SIZE(ais_prov_sd));
#else
		err = bt_mesh_adv_enable(param, prov_ad, ARRAY_SIZE(prov_ad),
				    prov_sd, prov_sd_len);
#endif
#endif
/*[Genie end] add by wenbing.cwb at 2021-10-19*/
		if (err == 0) {
			proxy_adv_enabled = true;

			/* Advertise 60 seconds using fast interval */
			if (prov_fast_adv) {
				prov_fast_adv = false;
				return K_SECONDS(60);
			}
		}
		else
		{
			BT_ERR("proxy adv err %d", err);
		}
	}
#endif /* PB_GATT */

#if defined(CONFIG_BT_MESH_GATT_PROXY)
	if (bt_mesh_is_provisioned() && (conn_count < CONFIG_BT_MAX_CONN)) {
		return gatt_proxy_advertise(next_sub());
	}
#endif /* GATT_PROXY */

	return K_FOREVER;
}

int bt_mesh_proxy_adv_stop(void)
{
	int err;

	BT_DBG("adv_enabled %u", proxy_adv_enabled);

	if (!proxy_adv_enabled) {
		return -EALREADY;
	}

	err = bt_mesh_adv_disable();
	if (err && err != -EALREADY) {
		BT_ERR("Failed to stop advertising (err %d)", err);
	} else {
		proxy_adv_enabled = false;
	}
	return err;
}

static struct bt_conn_cb proxy_conn_callbacks = {
	.connected = proxy_connected,
	.disconnected = proxy_disconnected,
};

/*[Genie begin] add by wenbing.cwb at 2021-10-19*/
#ifdef CONFIG_BT_MESH_NPS_OPT
#define PROXY_CID CONFIG_BT_MESH_VENDOR_COMPANY_ID
#define PROXY_VID 0x0F
#define PROXY_UPDATE_INTERVAL_TYPE 0x08
#define PROXY_RESTORE_TIMEROUT K_MSEC(60)
static k_timer_t restore_timer;
static uint8_t update_interval = 0;
int bt_mesh_proxy_update_interval(uint8_t frame_type, void *frame_buf)
{
	uint16_t cid = 0;
	uint8_t vid = 0;
	uint8_t ctrltype = 0;
	uint8_t netid = 0;
	uint8_t msgid = 0;
	uint8_t packnum = 0;
	uint8_t length = 0;
	uint16_t interval = 0;
	struct net_buf_simple *buf = NULL;
	struct bt_mesh_subnet *sub = &bt_mesh.sub[0];

	if ((genie_provision_get_state() != GENIE_PROVISION_SUCCESS) || (update_interval == 1))
	{
		return -1;
	}

	buf = (struct net_buf_simple *)frame_buf;
	if ((frame_type != BT_DATA_MANUFACTURER_DATA) || (frame_type != BT_DATA_SVC_DATA128) || (buf == NULL))
	{
		return -1;
	}

	if (buf->len < 10)
	{
		return -1;
	}

	cid = net_buf_simple_pull_be16(buf);
	vid = net_buf_simple_pull_u8(buf);
	ctrltype = net_buf_simple_pull_u8(buf);
	if ((cid == PROXY_CID) && (vid == PROXY_VID) && (ctrltype == PROXY_UPDATE_INTERVAL_TYPE))
	{
		netid = net_buf_simple_pull_u8(buf);
		if ((netid == sub->keys[0].nid) || (netid == sub->keys[1].nid))
		{
			msgid = net_buf_simple_pull_u8(buf);
			(void)msgid;
			packnum = net_buf_simple_pull_u8(buf);
			length = net_buf_simple_pull_u8(buf);
			if ((packnum == 0x11) && (length == 2))
			{
				interval = net_buf_simple_pull_be16(buf);
				if ((interval <= 400) && (interval >= 100))
				{
					fast_adv_param.interval_min = interval * 8 / 5;
					fast_adv_param.interval_max = fast_adv_param.interval_min;
					BT_DBG("update proxy interval: %u\n", fast_adv_param.interval_min);

					bt_mesh_proxy_adv_stop();
					bt_mesh_proxy_adv_start();
					update_interval = 1;

					k_timer_start(&restore_timer, PROXY_RESTORE_TIMEROUT);
				}
			}
		}
	}

	return 0;
}

void restore_timer_cb(void *timer, void *arg)
{
	k_timer_stop(&restore_timer);

	fast_adv_param.interval_min = PROXY_ADV_INT_MIN;
	fast_adv_param.interval_max = PROXY_ADV_INT_MAX;
	BT_DBG("restore proxy interval: %u\n", fast_adv_param.interval_min);

	bt_mesh_proxy_adv_stop();
	bt_mesh_proxy_adv_start();
	update_interval = 0;
}
#endif
/*[Genie end] add by wenbing.cwb at 2021-10-19*/

int bt_mesh_proxy_init(void)
{
	int i;

	/* Initialize the client receive buffers */
	for (i = 0; i < ARRAY_SIZE(clients); i++) {
		struct bt_mesh_proxy_client *client = &clients[i];

		client->buf.size = CLIENT_BUF_SIZE;
		client->buf.__buf = client_buf_data + (i * CLIENT_BUF_SIZE);
#if defined(CONFIG_BT_MESH_GATT_PROXY)
        k_work_init(&client->send_beacons, proxy_send_beacons);
#endif
#if defined(CONFIG_BT_MESH_IBEACON_PROXY)
		k_delayed_work_init(&client->send_ibeacon, proxy_send_ibeacon);
		k_delayed_work_submit(&client->send_ibeacon, PROVISIONED_IBEACON_PROXY_SEND_INTERVAL);
#endif
	}

	bt_conn_cb_register(&proxy_conn_callbacks);

#ifdef CONFIG_GENIE_OTA
    genie_ota_pre_init();
#endif

/*[Genie begin] add by wenbing.cwb at 2021-10-19*/
#ifdef CONFIG_BT_MESH_NPS_OPT
    k_timer_init(&restore_timer, restore_timer_cb, NULL);
#endif
/*[Genie end] add by wenbing.cwb at 2021-10-19*/

	return 0;
}

#endif