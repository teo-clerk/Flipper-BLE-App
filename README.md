# Multi Beacon Emulator for Flipper Zero

![License](https://img.shields.io/badge/License-MIT-blue.svg)

This application allows you to emulate various BLE Beacons (iBeacon standard) using your Flipper Zero. You can define multiple beacons in a JSON file and select which one to broadcast from the app menu.

## Features

- **Multi-Beacon Support**: Load an unlimited (up to 20 in this version) number of beacons from a configuration file.
- **Easy Configuration**: Edit a simple JSON file to add or modify beacons.
- **User Interface**: Select beacons from a scrollable list.
- **Real-time Emulation**: Broadcasts standard iBeacon packets (UUID, Major, Minor, RSSI).

## Installation

### Option 1: Install Pre-compiled App (.fap)

If you don't want to compile the code yourself, you can simply install the provided `.fap` file.

1.  Download the `aula_m4_beacon.fap` file from the `dist/` folder in this repository.
2.  Connect your Flipper Zero to your computer via USB.
3.  Open **qFlipper**.
4.  Navigate to `SD Card` -> `apps` -> `Bluetooth`.
5.  Drag and drop the `aula_m4_beacon.fap` file into this folder.
6.  (Optional but recommended) Create the data folder: `SD Card` -> `apps_data` -> `aula_m4_beacon`.

### Option 2: Build from Source

1.  Clone this repository.
2.  Make sure you have `ufbt` installed.
3.  Run `ufbt build`.
4.  Run `ufbt launch` to install and run it on your Flipper.

## Configuration (Adding Beacons)

The app reads beacons from a JSON file on your Flipper's SD card.

1.  Create a file named `beacons.json` on your computer.
2.  Add your beacons in the following format:

    ```json
    [
      {
        "name": "My Beacon",
        "uuid": "E282171413654717B14D4845BE72ECE0",
        "major": 1,
        "minor": 1,
        "rssi_1m": -59
      },
      {
        "name": "Another Beacon",
        "uuid": "FDA50693A4E24FB1AFCFC6EB07647825",
        "major": 10,
        "minor": 5,
        "rssi_1m": -54
      }
    ]
    ```

3.  **Transfer the file to your Flipper:**
    - **Using qFlipper:** Go to `SD Card` -> `apps_data`. Create a folder named `aula_m4_beacon` if it doesn't exist. Drag and drop your `beacons.json` into that folder.
    - **Using CLI:** `ufbt cli` or other tools can also be used to upload files.

### Do I need to re-install the app when changing `beacons.json`?

**NO.** The app reads the JSON file every time it starts. You only need to update the `beacons.json` file on the SD card and restart the app on the Flipper to see the changes. You do **not** need to run `ufbt launch` or recompile.

## Usage

1.  Open the app on your Flipper Zero (under `Apps` -> `Bluetooth`).
2.  If the JSON file is found, you will see a list of your beacons.
3.  Select a beacon to start emulating.
4.  The screen will show the details of the active beacon.
5.  Press **Back** to stop emulating and return to the list.
6.  Press **Back** again to exit the app.

## Troubleshooting

- **No Beacons Found**: If the file is missing or malformed, the app will load a default "Aula M4" beacon. Check that the file is at `/ext/apps_data/aula_m4_beacon/beacons.json` and contains valid JSON.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
