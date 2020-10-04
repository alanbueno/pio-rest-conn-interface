<p align="center">
  <a aria-label="Pio-RCI is free to use" href="https://github.com/alanbueno/pio-rest-conn-interface/blob/master/LICENSE" target="_blank">
    <img alt="License: MIT" src="https://img.shields.io/badge/License-MIT-success.svg?style=flat-square&color=33CC12" target="_blank" />
  </a>

  <a aria-label="CI Build for PlatformIO Core" href="https://docs.platformio.org/page/core/index.html" target="_blank">
    <img alt="CI Build for PlatformIO Core" src="https://github.com/platformio/platformio-core/workflows/Core/badge.svg" target="_blank" />
  </a>
  <a aria-label="CI Build for Docs" href="https://docs.platformio.org?utm_source=github&utm_medium=core" target="_blank">
    <img alt="CI Build for Docs" src="https://github.com/platformio/platformio-core/workflows/Docs/badge.svg" target="_blank" />
  </a>
  <a aria-label="Community Forums" href="https://community.platformio.org?utm_source=github&utm_medium=core" target="_blank">
    <img alt="Community Forums" src="https://img.shields.io/badge/PlatformIO-Community-orange.svg"/>
  </a>
</p>

# Pio-Rest-Conn-Interface &middot; [![GitHub license](https://img.shields.io/badge/license-MIT-blue.svg)](https://github.com/alanbueno/pio-rest-conn-interface/blob/master/LICENSE)

Pio-RCI is an experimental set of utilities for connection management of ESP boards through a REST interface.

---

- [📚 Documentation](#-documentation)
- [🏅 Features](#-features)
- [👏 Contributing](#-contributing)
- [❓ Questions](#-questions)
- [License](#license)

---

## 📚 Documentation

<p>This module makes use of <a aria-label="platformio" href="https://platformio.org/">platformio</a> to leverage the awesome connection features of the ESP family boards, it's been tested with the <a aria-label="esp32" href="https://www.espressif.com/en/products/socs/esp32">Esp32</a> but can definitely be expanded to other models over time.</p>

- [:hammer_and_wrench: Getting Started](#getting-started)
- [🚀 API Reference](#-api-reference)

---

## Getting Started

<p>To run this project and start interacting with your board, you'll have to setup <a aria-label="platformio" href="https://platformio.org/install/integration">platformio accordingly to your IDE</a> of choice.

Once that's done, grab your <a aria-label="esp32" href="https://www.espressif.com/en/products/socs/esp32">board with a Esp32-MCU</a>, plug it in and make sure it shows up on your PlatformIO > Devices section.

Important to note that this project is pre-set with the `esp32doit-devkit-v1` board setting in the [platformio.ini](platformio.ini#L15), which is the board we've been testing it with. If you're using a different board, make sure to find it on the PlatformIO > Board Explorer, and replace `esp32doit-devkit-v1` with the PlatformIO board name matching yours.

</p>

---

## 🚀 API Reference

The endpoints exposed once you've done a successful build and uploaded your code to the board are dynamic and set based on the Network Mode the board is currently running, that means:

\* <PRE_SET_DNS_DOMAIN_NAME> = `DOMAIN_NAME` pre set on [NetworkManager.cpp](src/LocalWebServer/NetworkManager.cpp#L8)

\* <AP_SSID> = name for the AP network that the board creates, pre set on [NetworkManager.cpp](src/LocalWebServer/NetworkManager.cpp#L7)

- AP mode - Access Point network mode (local net from board):

  - you should be connected to the `AP_SSID` wifi network

  - `GET` - http://<PRE_SET_DNS_DOMAIN_NAME>/ap/ping

    Response `200 Ok`:

    ```
    {
      "message": "pong"
    }
    ```

  - `POST` - http://<PRE_SET_DNS_DOMAIN_NAME>/ap/switch-connection-to/sta

    Payload:

    ```
    {
      "ssid": "Your-SSID-to-connect-to",
      "password": "the-password-for-it"
    }
    ```

    Response `200 Ok`:

    ```
    {
      "ssid": "Your-SSID-to-connect-to",
      "ip": "192.168.1.124", // this will change according to your network
      "message": "Started Sta mode"
    }
    ```

  - `POST` - http://<PRE_SET_DNS_DOMAIN_NAME>/sta/switch-connection-to/ap

    `Fallback route (reverse mode), in case of redundant attempts of connection, when the mode's already up but the client's still trying to enable it`

    Payload: no payload

    Response `409 Conflict`:

    ```
    {
      "message": "you're trying to enable the current active mode, please check"
    }
    ```

```
Note: AP is the mode the board will automatically switch to if:
  - it doesn't have SSID/Password already stored in flash (persistent over resets or power off) to connect to;
  - tried connecting but failed;
  - connected, but eventually the connection provider's offline and the connection was lost;
```

\* <BOARD_IP_ADDRESS> = the ip that the board assumes after successfully connecting to a network provider (e.g. wifi router), it's logged to the Serial Monitor and can be actively sent from the board to any storage system, e.g. remote databse, IoT info broker, ...

- STA mode - Station network mode (internet):

  - you should be connected to the same wifi network as the board successfully did

  - `GET` - http://<BOARD_IP_ADDRESS>/sta/ping

    Response `200 Ok`:

    ```
    {
      "message": "pong"
    }
    ```

  - `POST` - http://<BOARD_IP_ADDRESS>/sta/switch-connection-to/ap

    Payload: no payload

    Response `200 Ok`:

    ```
    {
      "message": "Ap mode set successfully!"
    }
    ```

  - `POST` - http://<BOARD_IP_ADDRESS>/ap/switch-connection-to/sta

    `Fallback route (reverse mode), in case of redundant attempts of connection, when the mode's already up but the client's still trying to enable it`

    Payload: both, with or without payload will provide the same response

    Response `409 Conflict`:

    ```
    {
      "message": "you're trying to enable the current active mode, please check"
    }
    ```

---

## 🏅 Features

- Multi-task implementation using two cores to execute connection health-check loop;
- DNS server while in Ap mode, so the client can call a conventioned url instead of an ip;
- Fallback routes (reverse mode), in case of redundant attempts of connection, when the mode's already up but the client's still trying to enable it;
- Progressive Retry Mechanism, first reconnection attempt to the network provider after N seconds `RECONNECTION_INTERVAL`, then 2 \* N seconds, 3 \* N seconds, ... all the way to the `MAX_RETRY_ATTEMPTS`;
- Optional remote logging placeholder at `Utils.cpp`;

---

## 👏 Contributing

If you like Pio-RCI and want to help make it better then check out the [Pio-RCI repo](https://github.com/alanbueno/pio-rest-conn-interface). Any help wether improving the implementation, new features, bugs, supported boards, docs, (...) is very much appreciated.

---

## ❓ Questions

If you have questions about Pio-RCI, suggestions or comments, for now write'em on our [issues](https://github.com/alanbueno/pio-rest-conn-interface/issues), later we'll open a discord or slack for it.

---

## License

Pio-RCI source code is made available under the [MIT license](LICENSE). Some of the dependencies are licensed differently, with the BSD license, for example.
