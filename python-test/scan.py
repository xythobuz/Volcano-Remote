import simplepyble

def ble_scan(addr = None, adapterIndex = 0, timeout = 1):
    adapters = simplepyble.Adapter.get_adapters()

    if len(adapters) == 0:
        print("No adapters found")
        return None

    adapter = adapters[adapterIndex]
    print("Selected adapter: {} [{}]".format(adapter.identifier(), adapter.address()))

    # TODO abort scan when found?
    print("Scanning for '{}' for {}s...".format(addr, timeout))
    adapter.scan_for(timeout * 1000)

    peripherals = adapter.scan_get_results()
    for peripheral in peripherals:
        if addr != None:
            if addr == peripheral.address():
                return peripheral
        else:
            if peripheral.identifier() == "S&B VOLCANO H":
                return peripheral

    print("No device found")
    return None

if __name__ == "__main__":
    import sys

    adapter = None
    mac = None
    if len(sys.argv) > 1:
        adapter = int(sys.argv[1])
    if len(sys.argv) > 2:
        mac = sys.argv[2]

    dev = ble_scan(mac, adapter)
    if dev != None:
        print("{} {}".format(dev.identifier(), dev.address()))
