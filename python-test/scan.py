import simplepyble

def ble_scan(addr):
    adapters = simplepyble.Adapter.get_adapters()

    if len(adapters) == 0:
        print("No adapters found")
        return None

    # TODO allow selection of bluetooth adapter
    adapter = adapters[0]
    print("Selected adapter: {} [{}]".format(adapter.identifier(), adapter.address()))

    # TODO support longer scans?
    print("Scanning for '{}' for 1s...".format(addr))
    adapter.scan_for(1000)

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

    arg = None
    if len(sys.argv) > 1:
        arg = sys.argv[1]

    dev = ble_scan(arg)
    if dev != None:
        print("{} {}".format(dev.identifier(), dev.address()))
