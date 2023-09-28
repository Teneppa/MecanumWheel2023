#include <Bluepad32.h>

#ifndef _BTLECONTROLLER_
#define _BTLECONTROLLER_

#include <Preferences.h>

class BTLEController {
public:
  bool isConnected;
  GamepadPtr myGamepad;
  GamepadProperties properties;

  // For saving things
  Preferences pref;

  // Setup callbacks, Preferences namespace and allow for new connections
  void setup();

  // Global pointer to BTLEController instance
  // This was a mess to setup, because Bluepad32 requires static functions
  // as callbacks, which can't access the application memory properly.
  static BTLEController* controllerInstance;

  // These are just the wrappers so that we can actually save and use the controller
  // information in the functions below.
  static void onConnectedGamepadWrapper(GamepadPtr gp);
  static void onDisconnectedGamepadWrapper(GamepadPtr gp);

  // These are the real functions
  void onConnectedGamepad(GamepadPtr gp);
  void onDisconnectedGamepad(GamepadPtr gp);

  // And therese are used to save and load controller bluetooth address from persistent
  // memory.
  void saveCurrentController();
  void getSavedController();

// This is the structure that will be saved into persistent memory.
// You can add more stuff there, but make sure the saved settings correspond
// to this one, so that the loading function will not try to access garbage data!
private:
  typedef struct {
    uint8_t btArray[6];
  } controller_settings_t;
};

// Setup function, always run this after creating the object!
void BTLEController::setup() {

  // Use namespace "joystick" for saving controller information
  this->pref.begin("joystick");

  // Setup the Bluepad32 callbacks using the wrapper functions
  BP32.setup(&BTLEController::onConnectedGamepadWrapper, &BTLEController::onDisconnectedGamepadWrapper);
  BP32.forgetBluetoothKeys();
}

// Initialize the static member
// This is also some weird trickery...
BTLEController* BTLEController::controllerInstance = nullptr;

// Wrappers are here
void BTLEController::onConnectedGamepadWrapper(GamepadPtr gp) {
  if (controllerInstance) {
    controllerInstance->onConnectedGamepad(gp);
  }
}

void BTLEController::onDisconnectedGamepadWrapper(GamepadPtr gp) {
  if (controllerInstance) {
    controllerInstance->onDisconnectedGamepad(gp);
  }
}

// Actual functions are here
void BTLEController::onConnectedGamepad(GamepadPtr gp) {
  // NOTE: Does the library always pair all controllers?
  // TODO: Add a check here for the gamepad's bluetooth address
  Serial.println("CALLBACK: Gamepad is connected!");

  myGamepad = gp;
  this->properties = gp->getProperties();

  Serial.printf("Gamepad model: %s, VID=0x%04x, PID=0x%04x\n",
                gp->getModelName().c_str(), this->properties.vendor_id,
                this->properties.product_id);
  const uint8_t* addr = this->properties.btaddr;

  String sAddr = "";
  for (uint8_t i = 0; i <= 5; i++) {
    sAddr += String(addr[i]);
    if (i != 5) {
      sAddr += ":";
    }
  }
  //Serial.printf("BT Addr: %2X:%2X:%2X:%2X:%2X:%2X\n", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
  Serial.println(sAddr);

  isConnected = true;

  // Disable new connections, for some reason there is a lot of errors if an unknown device is nearby
  // and the ESP32 is in scan mode.
  BP32.enableNewBluetoothConnections(false);
}

// When the controller disconnects
void BTLEController::onDisconnectedGamepad(GamepadPtr gp) {
  Serial.println("CALLBACK: Gamepad is disconnected!");
  myGamepad = nullptr;
  isConnected = false;
}

/* -------------------------------- PERSISTENT MEMORY STUFF --------------------------------- */

// Save address
void BTLEController::saveCurrentController() {
  // First let's check if there is a controller
  if (this->myGamepad != nullptr && this->isConnected) {
    // Then we can get the address
    const uint8_t* CADDR = this->properties.btaddr;

    Serial.println("BEGIN SAVE");
    Serial.println(CADDR[0]);

    // Then we need to construct a settings struct
    controller_settings_t newController = {
      { CADDR[0], CADDR[1], CADDR[2], CADDR[3], CADDR[4], CADDR[5] }
    };

    // Next up is saving the settings
    //pref.putString("BTLE_CTRL_ADDR", msg);
    this->pref.putBytes("BTLE_CTRL_ADDR", &newController, sizeof(newController));
  }
}

// Load address and print it
void BTLEController::getSavedController() {
  Serial.println("retrieving saved controller address...");
  // TODO: Add check if the address exists in memory
  controller_settings_t savedController = { 0, 0, 0, 0, 0, 0 };
  bool isKey = this->pref.isKey("BTLE_CTRL_ADDR");
  Serial.println(isKey);
  this->pref.getBytes("BTLE_CTRL_ADDR", &savedController, sizeof(savedController));

  for (uint8_t i = 0; i <= 5; i++) {
    Serial.print(savedController.btArray[i]);
    if (i != 5) {
      Serial.print(":");
    } else {
      Serial.println();
    }
  }
}



#endif
