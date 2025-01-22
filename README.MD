# Nordic 5340 Dev Kit + E710 MCU + Zephyr RTOS Reader

Create a Yukon reader using a Nordic nRf5340 development kit (or related) and an e710 kit.

## Assembling the Test Rig

Dev Kit + E710 Prototype

### Step 1. Connect Adapter to Dev Kit Arduino Connector

Adapter connects Arduino signals to the E710s RPi hat connector.

1. Insert adapter into dev kit arduino headers.
3. Attach R807 to 36 or 40 pin header on top of adapter. If using a 36-pin header, pins 37-40 are not connected.

### Dev Kit Connections to R807

For reference here are the adapter arduino to R807 RPi hat connections.

| Signal | Arduino Pin  | Yukon RPi Header Pin |
|---|---|---|
| SPI SS/CS | P1.12 | 24 |
| SPI MOSI | P1.13 | 19 |
| SPI MISO | P1.14 | 21 |
| SPI SCLK | P1.15 | 23 |
| PWR_EN | P0.4 | 29 Input |                                                 
| RESET_N | P0.5 | 33 Input |
| ENABLE | P0.6 | 32 Input |
| READY_N | P1.11 | 26 Output then Input |
| IRQ_N | P0.25 | 22 Output |
| GND | GND | 25 |


## Building and Running the Code

### Install Tools

1. Install VS Code.
2. Install Nordic nRf SDK and required extensions.
3. Install Segger JTAG tools and Ozone debugger.

### Create Build Configuration

Note: ask for help if you don't already have an "application" listed in step 2 below.

1. In VS Code, open workspace in project folder.
2. Pick nRf extension, then add a build under the impinj_nrf5340_r807_reader application.
3. Click build.
4. Click flash.
5. Click Debug In Ozone.

You should see debug output in the Ozone terminal giving tag read stats and Zephyr thread stats.


===============================================================
1) Uart: Output Description and Instructions in the terminal
2) Uart: input the parameter same as command-line of E710 SDK
3) Uart: Input "STOP" command in the terminal
4) Key1: Restart; Key2: Stop
5) LED: Standby-0.5s flash; Running-Flash with round loop