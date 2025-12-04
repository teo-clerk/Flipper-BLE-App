# Multi Beacon Emulator for Flipper Zero

This application allows you to emulate various BLE Beacons (iBeacon standard) using your Flipper Zero. You can define multiple beacons in a JSON file and select which one to broadcast from the app menu.

## Features

- **Multi-Beacon Support**: Load an unlimited (up to 20 in this version) number of beacons from a configuration file.
- **Easy Configuration**: Edit a simple JSON file to add or modify beacons.
- **User Interface**: Select beacons from a scrollable list.
- **Real-time Emulation**: Broadcasts standard iBeacon packets (UUID, Major, Minor, RSSI).

## Installation

1. **Compile and Install**:

   - Clone this repository into `applications_user/`.
   - Build using `ufbt` or the standard Flipper build system.
   - Deploy the `.fap` to your Flipper.

2. **Configuration**:
   - Create a file named `beacons.json`.
   - Place it in the following directory on your Flipper's SD card:
     ```
     /ext/apps_data/aula_m4_beacon/beacons.json
     ```
     _(Note: You may need to create the directory `aula_m4_beacon` inside `apps_data` if it doesn't exist)_

## JSON Format

The `beacons.json` file should be a JSON array of objects. Each object represents a beacon.

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

- **name**: Display name in the menu (max 32 chars).
- **uuid**: 32-character hexadecimal string (no dashes required, but parser handles them if clean).
- **major**: Integer (0-65535).
- **minor**: Integer (0-65535).
- **rssi_1m**: Calibrated RSSI at 1 meter (Tx Power), usually between -50 and -80.

## Usage

1. Open the app on your Flipper Zero.
2. If the JSON file is found, you will see a list of your beacons.
3. Select a beacon to start emulating.
4. The screen will show the details of the active beacon.
5. Press **Back** to stop emulating and return to the list.
6. Press **Back** again to exit the app.

## Troubleshooting

- **No Beacons Found**: If the file is missing or malformed, the app will load a default "Aula M4" beacon. Check the file path and JSON syntax.
