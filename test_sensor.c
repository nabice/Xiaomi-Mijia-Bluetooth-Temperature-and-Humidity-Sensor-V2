#include <stdlib.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

int main(int argc, char *argv[]) {
    int err, dd, i, result;
    bdaddr_t bdaddr;
    uint16_t interval, latency, max_ce_length, max_interval, min_ce_length;
    uint16_t min_interval, supervision_timeout, window, handle;
    uint8_t initiator_filter, own_bdaddr_type, peer_bdaddr_type;
    char buf[16];
    struct hci_filter flt;
    peer_bdaddr_type = LE_PUBLIC_ADDRESS;
    initiator_filter = 0; /* Use peer address */
    memset(&bdaddr, 0, sizeof(bdaddr_t));
    if (argv[1]) {
        str2ba(argv[1], &bdaddr);
    } else {
        printf("Usage: ./test_sensor A4:C1:38:XX:XX:XX\n");
        exit(1);
    }

    dd = hci_open_dev(0);
    if (dd < 0) {
        perror("Could not open device");
        exit(1);
    }

    interval = htobs(0x0004);
    window = htobs(0x0004);
    own_bdaddr_type = 0x00;
    min_interval = htobs(0x000F);
    max_interval = htobs(0x000F);
    latency = htobs(0x0000);
    supervision_timeout = htobs(0x0C80);
    min_ce_length = htobs(0x0001);
    max_ce_length = htobs(0x0001);

    err = hci_le_create_conn(dd, interval, window, initiator_filter,
            peer_bdaddr_type, bdaddr, own_bdaddr_type, min_interval,
            max_interval, latency, supervision_timeout,
            min_ce_length, max_ce_length, &handle, 25000);
    if (err < 0) {
        perror("Could not create connection");
        exit(1);
    }

    printf("Connection handle %d\n", handle);
    

    hci_filter_clear(&flt);
    hci_filter_set_ptype(HCI_ACLDATA_PKT, &flt);

    if (setsockopt(dd, SOL_HCI, HCI_FILTER, &flt, sizeof(flt)) < 0) {
        perror("Failed to set HCI filter");
        close(dd);
        return -1;
    }

    for(i = 0; i<10; i++) {
        memset(&buf, 0, sizeof(buf));
        result = read(dd, &buf, sizeof(buf));
        if(result == 16 && (buf[10] == 0x33 || buf[10] == 0x36)) {
            printf("%d %d\n", buf[12] | buf[13] << 8, buf[14]);
            break;
        }
    }
    
    err = hci_disconnect(dd, handle, HCI_OE_USER_ENDED_CONNECTION, 10000);
    if (err < 0) {
        perror("Could not disconnect");
        exit(1);
    }

    hci_close_dev(dd);
}
