#include <zephyr/kernel.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/types.h>
#include <stddef.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/util.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/gatt.h>
//#include <zephyr/services/hids.h>
#include <zephyr/bluetooth/uuid.h>

#include <zephyr/logging/log.h>
#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)
// = BT_UUID_INIT_128(0x03, 0xFE, 0x00, 0x10, 0x00, 0x80, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB)
static struct bt_uuid_128 avs_gatt_service_uuid;
static const struct bt_uuid_128 avs_gatt_characteristic_tx_uuid = BT_UUID_INIT_128(0xF0, 0x4E, 0xB1, 0x77, 0x30, 0x05, 0x43, 0xA7, 0xAC, 0x61, 0xA3, 0x90, 0xDD, 0xF8, 0x30, 0x76);
static const struct bt_uuid_128 avs_gatt_characteristic_rx_uuid = BT_UUID_INIT_128(0x2B, 0xEE, 0xA0, 0x5B, 0x18, 0x79, 0x4B, 0xB4, 0x8A, 0x2F, 0x72, 0x64, 0x1F, 0x82, 0x42, 0x0B);


static struct bt_gatt_attr avs_gatt_attrs[] = {
    BT_GATT_PRIMARY_SERVICE(&avs_gatt_service_uuid.uuid),
    BT_GATT_CHARACTERISTIC(&avs_gatt_characteristic_tx_uuid.uuid, BT_GATT_CHRC_WRITE_WITHOUT_RESP, BT_GATT_PERM_WRITE, NULL, NULL, NULL),
    BT_GATT_CHARACTERISTIC(&avs_gatt_characteristic_rx_uuid.uuid, BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY, BT_GATT_PERM_READ, NULL, NULL, NULL),
    BT_GATT_CCC(NULL, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE, NULL, avs_gatt_ccc_cfg_changed, NULL),
};

static struct bt_gatt_service avs_gatt_service = BT_GATT_SERVICE(avs_gatt_attrs);
/*static void avs_gatt_ccc_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
    ARG_UNUSED(attr);

    bool notifications_enabled = (value == BT_GATT_CCC_NOTIFY);

    // Handle notifications enabled/disabled event
    if (notifications_enabled) {
        // Notifications enabled, perform necessary actions
    } else {
        // Notifications disabled, perform necessary actions
    }
}*/


void advertising_start(void)
{
    int err;

    struct bt_data ad[] = {
        BT_DATA(BT_DATA_FLAGS, BT_LE_AD_NO_BREDR,sizeof(BT_LE_AD_NO_BREDR)),
        BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, sizeof(DEVICE_NAME) - 1),
    };

    struct bt_le_adv_param adv_param = {
        .id = BT_ID_DEFAULT,
        .interval_min = BT_GAP_ADV_FAST_INT_MIN_2,
        .interval_max = BT_GAP_ADV_FAST_INT_MAX_2,
    };

    err = bt_le_adv_start(&adv_param, ad, ARRAY_SIZE(ad), NULL, 0);
    if (err) {
        printk("Advertising failed to start (err %d)\n", err);
        return;
    }

    printk("Advertising started\n");
}

void main(void)
{
    int err;

    err = bt_enable(NULL);
    if (err) {
        printk("Bluetooth initialization failed (err %d)\n", err);
        return;
    }

    err = bt_set_name("Dongle");
    if (err) {
        printk("Failed to set device name (err %d)\n", err);
        return;
    }

    err = bt_gatt_service_register(&avs_gatt_service);
    if (err) {
        printk("Failed to register GATT service (err %d)\n", err);
        return;
    }
    /*static struct bt_gatt_ccc_cfg avs_gatt_ccc_cfg[BT_GATT_CCC_MAX] = {};
    avs_gatt_ccc_cfg[0].value = BT_GATT_CCC_NOTIFY;
    avs_gatt_ccc_cfg[0].cfg_changed = avs_gatt_ccc_cfg_changed;*/
    

    err = bt_gatt_subscribe(NULL, &avs_gatt_attrs[2], &avs_gatt_ccc_cfg[0]);
    if (err) {
        printk("Failed to subscribe to GATT attribute (err %d)\n", err);
        return;
    }

    advertising_start();

    while (1) {
        k_sleep(K_FOREVER);
    }
}
