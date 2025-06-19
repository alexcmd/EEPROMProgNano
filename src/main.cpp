#include <Arduino.h>

// Pin definitions for 74HC595 shift register
#define SHIFT_DATA 10     // DS pin (Serial data input)
#define SHIFT_CLK 12     // SH_CP pin (Shift register clock)
#define SHIFT_LATCH 11    // ST_CP pin (Storage register clock)

// Pin definitions for EEPROM
#define EEPROM_D0 2     // First data pin (D0)
#define EEPROM_D7 9     // Last data pin (D7)
#define WRITE_EN 13      // Write enable pin (/WE)

// ACK byte to send when data is written
#define ACK_BYTE 0x06

// EEPROM constants
#define EEPROM_SIZE 8192  // 8K bytes for AT28C64
#define CHUNK_SIZE 16     // Chunk size for data transfer

// Function declarations
void setAddress(uint16_t address);
byte readEEPROM(uint16_t address);
void writeEEPROM(uint16_t address, byte data);
void printContents(uint16_t start, uint16_t length);
void processCommand();
uint16_t readHexAddress();
uint16_t readHexLength();
byte readHexByte();

// Read a 4-digit hexadecimal address (0000-FFFF)
uint16_t readHexAddress() {
  uint16_t address = 0;
  
  // Wait for 4 hex characters
  for (uint8_t i = 0; i < 4; i++) {
    while (!Serial.available()) {}
    
    char c = Serial.read();
    uint8_t val = 0;
    
    // Convert ASCII to hex value
    if (c >= '0' && c <= '9') {
      val = c - '0';
    } else if (c >= 'A' && c <= 'F') {
      val = c - 'A' + 10;
    } else if (c >= 'a' && c <= 'f') {
      val = c - 'a' + 10;
    } else {
      // Invalid character, treat as 0
      val = 0;
    }
    
    // Shift and add
    address = (address << 4) | val;
  }
  
  return address;
}

// Read a 4-digit hexadecimal length (0000-FFFF)
uint16_t readHexLength() {
  return readHexAddress(); // Same format as address
}

// Read a 2-digit hexadecimal byte (00-FF)
byte readHexByte() {
  byte data = 0;
  
  // Wait for 2 hex characters
  for (uint8_t i = 0; i < 2; i++) {
    while (!Serial.available()) {}
    
    char c = Serial.read();
    uint8_t val = 0;
    
    // Convert ASCII to hex value
    if (c >= '0' && c <= '9') {
      val = c - '0';
    } else if (c >= 'A' && c <= 'F') {
      val = c - 'A' + 10;
    } else if (c >= 'a' && c <= 'f') {
      val = c - 'a' + 10;
    } else {
      // Invalid character, treat as 0
      val = 0;
    }
    
    // Shift and add
    data = (data << 4) | val;
  }
  
  return data;
}

void setup() {
  // Configure pins
  pinMode(SHIFT_DATA, OUTPUT);
  pinMode(SHIFT_CLK, OUTPUT);
  pinMode(SHIFT_LATCH, OUTPUT);
  pinMode(WRITE_EN, OUTPUT);
  
  // Set write enable high (disabled) by default
  digitalWrite(WRITE_EN, HIGH);
  
  // Configure data pins as inputs initially
  for (uint8_t pin = EEPROM_D0; pin <= EEPROM_D7; pin++) {
    pinMode(pin, INPUT);
  }
  
  // Initialize serial communication
  Serial.begin(115200);
  Serial.println("AT28C64 EEPROM Programmer Ready");
  Serial.println("Commands:");
  Serial.println("  R[4-digit address] - Read byte from address (e.g., R00FF)");
  Serial.println("  W[4-digit address][2-digit data] - Write byte to address (e.g., W00FF42)");
  Serial.println("  D[4-digit address][4-digit length] - Dump EEPROM contents (e.g., D00000100)");
  Serial.println("  P[4-digit address] - Program EEPROM with data from serial (e.g., P0000)");
}

void loop() {
  if (Serial.available()) {
    processCommand();
  }
}

// Process incoming commands from serial
void processCommand() {
  char cmd = Serial.read();
  
  switch (cmd) {
    case 'R': {
      // Read single byte - format: R0000 (4-digit hex address)
      uint16_t address = readHexAddress();
      byte data = readEEPROM(address);
      Serial.print("Read from 0x");
      // Always print 4 digits for address
      if (address < 0x1000) Serial.print("0");
      if (address < 0x100) Serial.print("0");
      if (address < 0x10) Serial.print("0");
      Serial.print(address, HEX);
      Serial.print(": 0x");
      // Always print 2 digits for data
      if (data < 0x10) Serial.print("0");
      Serial.println(data, HEX);
      break;
    }
    
    case 'W': {
      // Write single byte - format: W000000 (4-digit hex address + 2-digit hex data)
      uint16_t address = readHexAddress();
      byte data = readHexByte();
      writeEEPROM(address, data);
      Serial.print("Wrote 0x");
      // Always print 2 digits for data
      if (data < 0x10) Serial.print("0");
      Serial.print(data, HEX);
      Serial.print(" to 0x");
      // Always print 4 digits for address
      if (address < 0x1000) Serial.print("0");
      if (address < 0x100) Serial.print("0");
      if (address < 0x10) Serial.print("0");
      Serial.println(address, HEX);
      break;
    }
    
    case 'D': {
      // Dump EEPROM contents - format: D00000100 (4-digit hex address + 4-digit hex length)
      uint16_t address = readHexAddress();
      uint16_t length = readHexLength();
      if (length == 0) length = 16; // Default to 16 bytes
      printContents(address, length);
      break;
    }
    
    case 'P': {
      // Program EEPROM with data from serial - format: P0000 (4-digit hex address)
      uint16_t startAddress = readHexAddress();
      
      // Calculate how many bytes we can program from the start address
      uint16_t maxBytes = EEPROM_SIZE - startAddress;
      
      Serial.print("Programming EEPROM with ");
      Serial.print(maxBytes);
      Serial.print(" bytes starting at address 0x");
      // Always print 4 digits for address
      if (startAddress < 0x1000) Serial.print("0");
      if (startAddress < 0x100) Serial.print("0");
      if (startAddress < 0x10) Serial.print("0");
      Serial.println(startAddress, HEX);
      
      Serial.print("Ready to receive data starting at address 0x");
      // Always print 4 digits for address
      if (startAddress < 0x1000) Serial.print("0");
      if (startAddress < 0x100) Serial.print("0");
      if (startAddress < 0x10) Serial.print("0");
      Serial.println(startAddress, HEX);
      Serial.println("Send data in 64-byte chunks...");
      Serial.flush(); // Ensure all output is sent before waiting for input
      
      uint16_t currentAddress = startAddress;
      unsigned long timeoutStart;
      boolean ackReceived = false;
      
      while (currentAddress < (startAddress + maxBytes)) {
        // Wait for data chunk with timeout
        timeoutStart = millis();
        while (Serial.available() < CHUNK_SIZE) {        
          
          // Check for timeout (5 seconds)
          if (millis() - timeoutStart > 60000) {
            Serial.println("Timeout waiting for data. Programming aborted.");
            return;
          }
          
          delay(10); // Small delay to prevent hammering the CPU
        }
        
        // Read a chunk of data from serial
        byte dataChunk[CHUNK_SIZE];
        Serial.readBytes(dataChunk, CHUNK_SIZE);
        
        // Write chunk to EEPROM
        for (uint8_t i = 0; i < CHUNK_SIZE; i++) {
          writeEEPROM(currentAddress + i, dataChunk[i]);
          
          // Small delay to allow EEPROM write cycle to complete
          delayMicroseconds(100);
        }
        
        // Send ACK after writing chunk
        Serial.write(ACK_BYTE);
        Serial.flush(); // Make sure ACK is sent immediately
        
        // Update address for next chunk
        currentAddress += CHUNK_SIZE;
        
        // Check if we've reached the end of EEPROM
        if (currentAddress > EEPROM_SIZE) {
          Serial.println("Reached end of EEPROM. Programming complete.");
          break;
        }
      }
      
      Serial.println("Programming complete.");
      break;
    }
    
    default:
      // Consume any remaining characters
      while (Serial.available()) {
        Serial.read();
      }
      break;
  }
}

// Set the address using two 74HC595 shift registers
void setAddress(uint16_t address) {
  // First, bring latch low to begin data transfer
  digitalWrite(SHIFT_LATCH, LOW);
  
  // Shift out the high byte (A15-A8)
  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, (address >> 8));
  
  // Shift out the low byte (A7-A0)
  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, address & 0xFF);
  
  // Bring latch high to update the output
  digitalWrite(SHIFT_LATCH, HIGH);
}

// Read a byte from the EEPROM at the specified address
byte readEEPROM(uint16_t address) {
  // Configure data pins as inputs
  for (uint8_t pin = EEPROM_D0; pin <= EEPROM_D7; pin++) {
    pinMode(pin, INPUT);
  }
  
  // Set the address
  setAddress(address);
  
  // Read the data
  byte data = 0;
  for (uint8_t i = 0; i < 8; i++) {
    // Read each bit and put it in the correct position in the byte
    if (digitalRead(EEPROM_D0 + i)) {
      data |= (1 << i);
    }
  }
  
  return data;
}

// Write a byte to the EEPROM at the specified address
void writeEEPROM(uint16_t address, byte data) {
  // Configure data pins as outputs
  for (uint8_t pin = EEPROM_D0; pin <= EEPROM_D7; pin++) {
    pinMode(pin, OUTPUT);
  }
  
  // Set the address
  setAddress(address);
  
  // Put data on the bus
  for (uint8_t i = 0; i < 8; i++) {
    digitalWrite(EEPROM_D0 + i, (data >> i) & 1);
  }
  
  // Toggle write enable to write the data
  digitalWrite(WRITE_EN, LOW);
  delayMicroseconds(1);  // Pulse width
  digitalWrite(WRITE_EN, HIGH);
  
  // Wait for write cycle to complete (max 10ms for AT28C64)
  delay(10);
}

// Print a formatted hex dump of EEPROM contents
void printContents(uint16_t start, uint16_t length) {
  const uint8_t BYTES_PER_LINE = 16;
  
  for (uint16_t base = start; base < start + length; base += BYTES_PER_LINE) {
    // Print address (always 4 digits)
    Serial.print("0x");
    if (base < 0x1000) Serial.print("0");
    if (base < 0x100) Serial.print("0");
    if (base < 0x10) Serial.print("0");
    Serial.print(base, HEX);
    Serial.print(": ");
    
    // Print hex values
    for (uint8_t offset = 0; offset < BYTES_PER_LINE; offset++) {
      if ((uint32_t)(base + offset) < (uint32_t)(start + length)) {
        byte data = readEEPROM(base + offset);
        
        // Always print 2 digits for data
        if (data < 0x10) Serial.print("0");
        Serial.print(data, HEX);
        Serial.print(" ");
      } else {
        Serial.print("   ");
      }
    }
    
    // Print ASCII representation
    Serial.print(" | ");
    for (uint8_t offset = 0; offset < BYTES_PER_LINE; offset++) {
      if ((uint32_t)(base + offset) < (uint32_t)(start + length)) {
        byte data = readEEPROM(base + offset);
        
        if ((uint8_t)data >= 32 && (uint8_t)data < 127) {
          Serial.write(data);
        } else {
          Serial.print(".");
        }
      } else {
        Serial.print(" ");
      }
    }
    
    Serial.println();
  }
}
