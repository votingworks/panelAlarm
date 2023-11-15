/* HID / TTY Panel Alarm
   by: Jesse DeWald
   Company: VotingWorks
   date: 11/14/2023
   license: MIT License - Feel free to use this code for any purpose.
   This code checks the state of an i2c button (at a default address)
   It sends the state to a host via an HID emulation
   and waits for acknowledgements and commands on UART
*/

#include <SparkFun_Qwiic_Button.h>  // Include the library for the SparkFun Qwiic Button
#include <Keyboard.h>               // Include the library for keyboard HID emulation
#define DEBUG true

#ifdef DEBUG
#define DEBUG_PRINT(x) Serial.print(x)      // Define macro for printing debug messages
#define DEBUG_PRINTLN(x) Serial.println(x)  // Define macro for printing debug messages with a newline
#else
#define DEBUG_PRINT(x)    // Empty macro if DEBUG is not defined
#define DEBUG_PRINTLN(x)  // Empty macro if DEBUG is not defined
#endif

// Button and state enumeration declarations
QwiicButton panelSwitch;  // Object for the Qwiic Button
enum PanelState { SECURED,
                  UNSECURED };  // States of the panel
enum AlarmState { ARMED,
                  UNARMED };  // States of the alarm
enum Command { CMD_NONE,
               CMD_ARM,
               CMD_UNARM,
               CMD_ACK,
               CMD_UNKNOWN };  // Types of commands that can be received

AlarmState currentAlarm;  // state of the alarm. probably doesn't need to be global but I'm being lazy

// Constants
const uint8_t BUFFERSIZE = 100;       //command buffer size
const uint8_t MAXRETRIES = 500;       //maximum number of state alerts. not currently implemented
const uint16_t ACKTIMEOUT_MS = 5000;  //time between alert spamming
const uint16_t DEBOUNCE = 1000;       //button debounce time. not currenlty implemented.
const uint16_t BAUDRATE = 115200;     //uart comms baud rate



char txBuffer[BUFFERSIZE];  // Buffer for serial communication

void setup() {
  Keyboard.begin();        // Start keyboard HID emulation
  Wire.begin();            // Start I2C communication
  Serial.begin(BAUDRATE);  // Start serial communication at baud rate
  delay(1000);             // Delay to stabilize I2C communication
  // Check if the Qwiic Button is connected and initialized properly
  if (panelSwitch.begin() == false) {
    DEBUG_PRINTLN("ERROR: ALARM FAILED TO INITIALIZE");  // Print error message if initialization fails
    while (true) {}                                      // Infinite loop to stop further execution
  }
  // DEBUG_PRINTLN(panelSwitch.setDebounceTime(DEBOUNCE));
  DEBUG_PRINTLN("STATUS: ALARM READY");  // Print status message when ready
  currentAlarm = UNARMED;                // initialize alarm state to unarmed
  delay(100);                            // Short delay after setup
}

void loop() {
  // Main loop of the program
  char alert;               // Variable to store the alert character
  PanelState currentPanel;  // Variable to store the current panel state

  // Check if the alarm is unarmed and wait for an ARM command
  if (currentAlarm == UNARMED) {
    DEBUG_PRINTLN("UNARMED: WAITING FOR ARM COMMAND");
    while (waitForCMD() != CMD_ARM) {}  // Loop until ARM command is received
    currentAlarm = ARMED;               // Change alarm state to armed
  }

  // Get the current state of the panel
  currentPanel = getPanelState();
  // Handle different states of the panel
  switch (currentPanel) {
    case SECURED:  // If the panel is secured
      DEBUG_PRINTLN("ARMED: PANEL IS SECURED");
      alert = ']';  // Set alert character for secured state
      panelSwitch.LEDoff();
      break;
    case UNSECURED:  // If the panel is unsecured
      DEBUG_PRINTLN("ARMED: PANEL IS UNSECURED");
      alert = '[';  // Set alert character for unsecured state
      panelSwitch.LEDon();
      break;
    default:  // If the panel state is unknown
      DEBUG_PRINTLN("ERROR: UNKNOWN PANEL STATE");
      DEBUG_PRINTLN("HALTING");
      while (true) {}  // Infinite loop to halt further execution
  }

  // Send alert and wait for acknowledgment
  spamAndAck(alert);

  // Continue checking the panel state and wait for change or UNARM command
  while (currentPanel == getPanelState()) {
    if (Serial.available()) {         // Check if there is serial data available
      Command newCMD = waitForCMD();  // Wait for a new command
      if (newCMD == CMD_UNARM) {      // If the command is UNARM
        currentAlarm = UNARMED;       // Change the alarm state to unarmed
        break;                        // Break the loop to process new state
      }
    }
  }
}

Command waitForCMD() {
  // Function to wait for a command from serial input
  while (!Serial.available()) {};      // Wait until there is data on the serial
  getTX();                             // Read the data from serial
  Command parsed = parseTX(txBuffer);  // Parse the command from the data
  resetBuffer();                       // Reset the buffer for the next command
  return parsed;                       // Return the parsed command
}

void spamAndAck(char alert) {
  // Function to send alert and wait for acknowledgment
  // DEBUG_PRINTLN(alert);  // Print the alert character
  do {
    Keyboard.write(alert);        // Send the alert character as keyboard input
    delay(5000);                  // Delay between sending alerts
  } while (!Serial.available());  // Loop until there is data on the serial
  getTX();                        // Read the data from serial
  // Check if the received command is acknowledgment
  if (parseTX(txBuffer) == CMD_ACK) {
    int length = strlen(txBuffer);   // Get the length of the received data
    char ackResponse = txBuffer[5];  // Extract the acknowledgment response
    resetBuffer();                   // Reset the buffer
    // Check if the acknowledgment response matches the alert
    if (ackResponse != alert) {
      DEBUG_PRINTLN("WRONG ACK");  // Print error message if acknowledgment is wrong
      spamAndAck(alert);           // Resend the alert and wait for acknowledgment
    }
  } else {
    DEBUG_PRINTLN("NEED TO ACK FIRST");  // Print error message if acknowledgment is not received
    spamAndAck(alert);                   // Resend the alert and wait for acknowledgment
  }
  DEBUG_PRINTLN("ALERT ACKNOWLEDGED");  // Print acknowledgment received message
}

void resetBuffer() {
  // Function to reset the transmission buffer
  memset(txBuffer, '\0', BUFFERSIZE);  // Clear the buffer by setting all values to null character
}

PanelState getPanelState() {
  // Function to get the current state of the panel
  delay(20);  // Short delay for debounce
  // Check if the button is pressed and return the corresponding state
  if (panelSwitch.isPressed() == true) return SECURED;  // Return SECURED if button is pressed
  else return UNSECURED;                                // Return UNSECURED if button is not pressed
}

char* getTX() {
  // Function to read the transmitted data from serial
  uint8_t bufferIndex = 0;  // Index for buffer array
  char newChar;             // Variable to store the incoming character
  // Loop to read each character from serial
  while (Serial.available()) {
    newChar = Serial.read();  // Read the incoming character
    // Check if buffer is not full and add the character to buffer
    if (bufferIndex < BUFFERSIZE - 1) {
      txBuffer[bufferIndex] = newChar;
      bufferIndex++;  // Increment buffer index
    } else {
      break;  // Break the loop if buffer is full
    }
    delay(1);  // Short delay for buffer processing
  }
  txBuffer[bufferIndex] = '\0';  // Null-terminate the string in buffer
  DEBUG_PRINTLN(txBuffer);       // Print the received data for debugging
  return txBuffer;               // Return the buffer
}

Command parseTX(char* strBuffer) {
  // Function to parse the transmitted data and extract command
  // Check if the data is enclosed in '<' and '>'
  uint8_t length = strlen(strBuffer);
  char message[length - 2 + 1];  // Buffer to store the actual message excluding '<' and '>'
  if (length > 2 && strBuffer[0] == '<' && strBuffer[length - 1] == '>') {
    strncpy(message, strBuffer + 1, length - 2);  // Copy the message to buffer
    message[length - 2] = '\0';                   // Null-terminate the message

    // Compare the message with predefined commands and return the corresponding enum value
    if (strcmp(message, "ARM") == 0) {
      DEBUG_PRINTLN("ARM command received.");
      return CMD_ARM;
    } else if (strcmp(message, "UNARM") == 0) {
      DEBUG_PRINTLN("UNARM command received.");
      return CMD_UNARM;
    } else if (strncmp(message, "ACK", 3) == 0) {
      DEBUG_PRINTLN("ACK command received.");
      return CMD_ACK;
    } else {
      DEBUG_PRINT("Unknown command: ");
      DEBUG_PRINTLN(message);
      return CMD_UNKNOWN;
    }
  } else {
    DEBUG_PRINTLN("No valid message found.");
    return CMD_NONE;  // Return CMD_NONE if the data does not match the format
  }
}
