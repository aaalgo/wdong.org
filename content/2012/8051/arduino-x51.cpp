#include <Arduino.h>
#include <util/delay.h>

#define BAUD_RATE 9600
#define SERIAL_TIMEOUT 10000

#define REST_PIN 9
#define ERROR_PIN  10
#define MOSI_PIN 11
#define MISO_PIN 12
#define SYNC_PIN 13

#define PAGE_SIZE 0x100L

#define F_CPU_51    F_CPU
// all times are in microsecond

#define T_CLCL (1000000/double(F_CPU_51)) 
#define T_RESET (64 * T_CLCL)
#define T_SHSL (8 * T_CLCL)
#define T_SLSH (8 * T_CLCL)
#define T_OVSH T_CLCL
#define T_SHOX (2 * T_CLCL)
#define T_SLIV (0.032 * 10)

#define T_CHIP_ERASE    500
#define T_SWC   (64 * T_CLCL + 0.4)

#define BIT_ORDER MSBFIRST

void serial_setup () {
    pinMode(REST_PIN, OUTPUT);
    pinMode(ERROR_PIN, OUTPUT);
    pinMode(MOSI_PIN, OUTPUT);
    pinMode(MISO_PIN, INPUT);
    pinMode(SYNC_PIN, OUTPUT);
    digitalWrite(ERROR_PIN, HIGH);
    digitalWrite(SYNC_PIN, LOW);
}

uint8_t serial_io (uint8_t in) {
	uint8_t out = 0;
	uint8_t i;
	for (i = 0; i < 8; ++i) {
        // setup MOSI
        _delay_ms(T_SLIV);
        out |= digitalRead(MISO_PIN) << (7 - i);
        digitalWrite(MOSI_PIN, !!(in & (1 << (7 - i))));
        _delay_ms(T_OVSH);
		digitalWrite(SYNC_PIN, HIGH);
        _delay_ms(T_SHSL);
		digitalWrite(SYNC_PIN, LOW);
        _delay_ms(T_SLSH);
	}
	return out;
}

void begin_download () {
    digitalWrite(ERROR_PIN, LOW);
    digitalWrite(SYNC_PIN, LOW);
    digitalWrite(REST_PIN, HIGH);
    _delay_ms(T_RESET);
}

void end_download () {
    digitalWrite(REST_PIN, LOW);
}

void sync () {
    int n = 0;
    for (;;) {
        while (Serial.available() <= 0);
        int c = Serial.read();
        if (c == '5') {
            n = 1;
            continue;
        }
        if (n == 1 && c == '1') {
            break;
        }
        n = 0;
    }
}

void do_write (int page) {
    int r;
    // programming enable
    serial_io(0xAC);
    serial_io(0x53); 
    serial_io(0x00);
    r = serial_io(0x00);
    if (r != 0x69) {
        digitalWrite(ERROR_PIN, HIGH);
        return;
    }
    // chip erase
    serial_io(0xAC);
    serial_io(0x80);
    serial_io(0x00);
    serial_io(0x00);

    _delay_ms(T_CHIP_ERASE);
    Serial.write(uint8_t(0));

    char n_block;
    r = Serial.readBytes(&n_block, 1);
    if (r != 1) {
        return;
    }
    Serial.write(uint8_t(0));

    char buf[PAGE_SIZE];
    for (char n = 0; n < n_block; ++n) {
        int c = Serial.readBytes(buf, PAGE_SIZE);
        if (c != PAGE_SIZE) break;
        char checksum = 0;
        if (page) {
            serial_io(0x50);
            serial_io(n);
        }
        uint8_t i = 0;
        do {
            if (!page) {
                serial_io(0x40);
                serial_io(n);
                serial_io(i);
            }
            serial_io(buf[i]);
            _delay_ms(T_SWC);
            checksum = checksum ^ buf[i];
            ++i;
        }
        while (i);
        Serial.write(checksum);
    }
}

void do_read (int page) {
    int r;
    // programming enable
    serial_io(0xAC);
    serial_io(0x53); 
    serial_io(0x00);
    r = serial_io(0x00);
    if (r != 0x69) {
        digitalWrite(ERROR_PIN, HIGH);
        return;
    }
    char n_block;
    r = Serial.readBytes(&n_block, 1);
    if (r != 1) {
        digitalWrite(ERROR_PIN, HIGH);
        return;
    }
    // init SPI
    uint8_t buf[PAGE_SIZE];
    for (char n = 0; n < n_block; ++n) {
        char checksum = 0;
        if (page) {
            serial_io(0x30);
            serial_io(n);
        }
        uint8_t i = 0;
        do {
            if (!page) {
                serial_io(0x20);
                serial_io(n);
                serial_io(i);
            }
            buf[i] = serial_io(0);
            checksum = checksum ^ buf[i];
            ++i;
        }
        while (i);
        Serial.write(buf, PAGE_SIZE);
        Serial.write(checksum);
    }
}

void setup () {
    Serial.setTimeout(SERIAL_TIMEOUT);
    serial_setup();
}

void loop () {
    Serial.begin(BAUD_RATE);
    sync();
    char n[2];
    Serial.readBytes(n, 2);
    begin_download();
    if (n[0] == 1) {
        do_write(n[1]);
    }
    if (n[0] == 2) {
        do_read(n[1]);
    }
    end_download();
    Serial.end();
}

