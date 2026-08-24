/*
The code was developed to complete course assigments and an in-lab demo for ELCT403 Senior Design I.

This code demonstrates:
  1. reception of data from a mock field sensor via nRF24 radio module
  2. storage of that data to internal array in format comparable to sample data in water.h
  3. transmission of sample data contained in water.h via RFM96W LoRa radio module
  4. adjustment of LoRa radio bandwidth setting to observe packet-time-on-air (TOA)
of a Teensy 3.5 MCU servicing the PHY aspects of a field station (rx/tx radio packets).

By field station it is meant residing in the vicinity of nRF24 compatible, deployed field sensors, which
having collected data wirelessly from these sensors, sends edge-processed data values to a base station
of an equivalent architecture via LoRa radio interface.

It is acknowledged that data in array "log_buffer" should be relayed to RPi in final project iterations,
through development of state logic to interface with a supervisory single board computer.
*/

#include <RadioLib.h>
#include <SPI.h>
#include "water.h"

#define MAX_HEIGHT_VAL 14   // water.h sample data contains 14 columns

#define RFM96W_CS 10
#define RFM96W_IRQ 24
#define RFM96W_RST 25
#define RFM96W_FREQ 433.6
#define RFM96W_BANDWIDTH 125
#define RFM96W_SF 6
#define RFM96W_CODING_RATE 5
#define RFM96W_SYNC_WORD 0x34  // default for LoRaWAN
#define RFM96W_OUTPUT_POWER 2  // dBm
#define RFM96W_PREAMBLE_LEN 60  // expressed in symbols
#define RFM96W_GAIN 0  // of the receiver
#define RFM96W_CONFIG \
  RFM96W_FREQ, RFM96W_BANDWIDTH, RFM96W_SF, RFM96W_CODING_RATE, \
  RFM96W_SYNC_WORD, RFM96W_OUTPUT_POWER, RFM96W_PREAMBLE_LEN, RFM96W_GAIN

#define NRF24_PAYLOAD_LEN 8
#define NRF24_PK_LEN 11
#define NUM_SENSOR 2
#define LOG_BUF_LEN 1
#define LORA_PAYLOAD_LEN 8
#define LORA_PK_LEN 19

struct LoRa_PK_t;
struct nRF24_PK_t;
void decode_nRF24(nRF24_PK_t * raw_pks, double * _log_buffer, int RAW_PK_LEN = NRF24_PAYLOAD_LEN, int LOG_ROW_NUM = LOG_BUF_LEN, int LOG_COL_NUM = MAX_HEIGHT_VAL);
void cast_double_uint8_t_payload(double * in, LoRa_PK_t * pyld_in, int pyld_len, uint8_t opt);

/******************************************************************************
 * nRF24 payload structure
 */
struct nRF24_PK_t{
  uint8_t sens_id;      // supports unique field stations ids
  uint8_t type;         // number according to column number in header table
  uint8_t base10_exp;   // allow noninteger payload values
  uint8_t payload[NRF24_PAYLOAD_LEN];   // 8 bytes
};

/******************************************************************************
 * LoRa payload structure
 */
struct LoRa_PK_t{
  uint8_t fstn_id;    // supports unique base station ids
  uint8_t type;       // supports generic type, such as mean water level
  uint8_t base10_exp;   // allow noninteger payload values
  uint8_t timestamp[LORA_PAYLOAD_LEN];    // 8 bytes
  uint8_t payload[LORA_PAYLOAD_LEN];      // 8 bytes
};

LoRa_PK_t LoRa_PK;
nRF24_PK_t nRF24_PK;

RFM96 lr_radio = new Module(RFM96W_CS, RFM96W_IRQ, RFM96W_RST);

nRF24 rf24_rad = new Module(9, 17, 16);

// flag to indicate that a packet was received
volatile bool nRF24_rx_flg = false;
void setFlag_nrfRX(void) { nRF24_rx_flg = true; }

volatile bool LoRa_tx_flg = true;
void setFlag_loraTX(void) { LoRa_tx_flg = true; }

int nRF24_rx_ret; int nRF24_sz;

int LoRa_tx_ret;

// memory location to receive raw nRF24 packets
nRF24_PK_t nRF24_buffer[MAX_HEIGHT_VAL] = {};

// memory location to populate in height data byte stream; first in, last out
double log_buffer[LOG_BUF_LEN][MAX_HEIGHT_VAL] = {};
double * log_buf_ptr = &(log_buffer[0][0]);

/******************************************************************************
 * nRF24 packet structure to height data header (>32 byte)
 */
void decode_nRF24(nRF24_PK_t * raw_pks, double * _log_buffer_ptr, 
                  int RAW_PK_LEN = NRF24_PAYLOAD_LEN,
                  int LOG_ROW_NUM = LOG_BUF_LEN, 
                  int LOG_COL_NUM = MAX_HEIGHT_VAL) {

  uint64_t uint64_payload = 0;
  double d_payload = 0;
  int n = 0;
  int dec_num = 0;

  for (int i = 0; i < LOG_COL_NUM; ++i) {
    for (int j = 0; j < RAW_PK_LEN; ++j) {
      uint64_payload = (uint64_payload << 8) | (uint64_t) raw_pks[i].payload[j];
    }

    n = static_cast<double>(raw_pks[i].type);
    dec_num = static_cast<double>(raw_pks[i].base10_exp);

    d_payload = (double) (uint64_payload / pow(10.0, dec_num));
  
    *(_log_buffer_ptr + n) = d_payload;

  }
}

/******************************************************************************
 * input double value, separate into 8 bytes, pack into LoRa_PK_t instance
 */
void cast_double_uint8_t_payload(double * in, LoRa_PK_t * pyld_in, int pyld_len, uint8_t exp) {
  uint64_t temp_in; double temp_ind;
  temp_ind = (double) ((*in) * pow(10.0, (double)(exp)));
  temp_in =  (uint64_t) temp_ind;
  if (exp == 0) {
    for (int j = 0; j < pyld_len; ++j) {
      pyld_in->timestamp[pyld_len-j-1] = (uint8_t)( (temp_in >> 8*j) & 0xFFFFFFFFFFFFFFFF); // big endian
    } 
  } else {
  for (int j = 0; j < pyld_len; ++j) {
    pyld_in->payload[pyld_len-j-1] = (uint8_t)( (temp_in >> 8*j) & 0xFFFFFFFFFFFFFFFF); // big endian
  }
  }
}

float bw_param[10] = {7.8, 10.4, 15.6, 20.8, 31.25, 41.7, 62.5, 125, 250, 500};
int pwr_param[16] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17};

void setup() {
	
  Serial.begin(115200);
/* Setup nRF2401L+ module
 */
  Serial.print(("[nRF24] Initializing ... \r\n"));
  nRF24_rx_ret = rf24_rad.begin();
  if(nRF24_rx_ret == RADIOLIB_ERR_NONE) {
    Serial.print(("success!\r\n"));
  } else {
    Serial.print(("failed, code \r\n"));
    Serial.print(nRF24_rx_ret);
    while (true) { delay(10); }
  }

  Serial.print("[nRF24] Setting address for receive pipe 0 ... \r\n");
  byte addr[] = {0x01, 0x23, 0x45, 0x67, 0x89};
  nRF24_rx_ret = rf24_rad.setReceivePipe(0, addr);
  if(nRF24_rx_ret == RADIOLIB_ERR_NONE) {
    Serial.print(("success!\r\n"));
  } else {
    Serial.print(("failed, code \r\n"));
    Serial.print(nRF24_rx_ret);
    while (true) { delay(10); }
  }

  rf24_rad.setPacketReceivedAction(setFlag_nrfRX);

  // start listening
  Serial.print(("[nRF24] Starting to listen ... \r\n"));
  nRF24_rx_ret = rf24_rad.startReceive();
  if (nRF24_rx_ret == RADIOLIB_ERR_NONE) {
    Serial.print(("success!\r\n"));
  } else {
    Serial.print(("failed, code \r\n"));
    Serial.print(nRF24_rx_ret);
    while (true) { delay(10); }
  }

/******************************************************************************
 * Initizalize LoRa packet type structure
 */
  LoRa_PK.fstn_id = 11;
  LoRa_PK.base10_exp = 4;
  LoRa_PK.type = 34;
  cast_double_uint8_t_payload(&(height_data[0][0]), &(LoRa_PK), LORA_PAYLOAD_LEN, 0*LoRa_PK.base10_exp);
  
  cast_double_uint8_t_payload(&(height_data[0][1]), &(LoRa_PK), LORA_PAYLOAD_LEN, LoRa_PK.base10_exp);

 /*****************************************************************************
 * Setup LoRa module
 */	
  Serial.printf("[SX1276] initializing ...");
  // initialize SX1276 with user settings:
  //LoRa_tx_ret = lr_radio.begin(434.0, 125.0, 9, 7, RADIOLIB_SX127X_SYNC_WORD, 2, 8, 0);
  LoRa_tx_ret = lr_radio.begin(434.0, bw_param[0], 9, 7, RADIOLIB_SX127X_SYNC_WORD, 2, 8, 0);
  lr_radio.setOutputPower(pwr_param[0]);

  if (LoRa_tx_ret == RADIOLIB_ERR_NONE) { Serial.printf("success!"); } else {
    Serial.printf("failed, code ");
    Serial.println(LoRa_tx_ret);
    while (true) { delay(10); }
  }

  if (LoRa_tx_ret == RADIOLIB_ERR_NONE) { Serial.printf("[SX1276] standby\r\n"); }

  lr_radio.setPacketSentAction(setFlag_loraTX);
  //Serial.print("[SX1276] Sending first packet ... \r\n");
  //stat = lr_radio.startTransmit(LoRa_PK, sizeof(LoRa_PK)); 
}

int k_nRF24 = 0;
uint8_t nRF24_read_buf[NRF24_PK_LEN] = {};
nRF24_PK_t * nRF24_buf_ptr = &(nRF24_buffer[k_nRF24]);
int k_LoRa = 0;

uint8_t LoRa_write_buf[LORA_PK_LEN] = {};
uint8_t * LoRa_write_buf_ptr = &(LoRa_write_buf[0]);

int i_bw = 0;
int i_pwr = 0;

void loop() {
    if(nRF24_rx_flg) {

      nRF24_rx_flg = false;
      rf24_rad.finishReceive();
      nRF24_sz = rf24_rad.getPacketLength();
      nRF24_rx_ret = rf24_rad.readData( nRF24_read_buf, NRF24_PK_LEN);

      Serial.printf("\nnRF24_sz:\t%d\r\n", nRF24_sz);
      // transfer to 2D array with MAX_HEIGHT_VAL number of rows
      memcpy(&(nRF24_buffer[k_nRF24]), nRF24_read_buf, NRF24_PK_LEN);

      if (nRF24_rx_ret == RADIOLIB_ERR_NONE) {
      // packet was successfully received
      // print raw data of the packet
        Serial.printf("[nRF24] sens_id:\t%d\r\n", nRF24_buffer[k_nRF24].sens_id);
        Serial.printf("[nRF24] type:\t%d\r\n", nRF24_buffer[k_nRF24].type);
        Serial.printf("[nRF24] base10_exp:\t%d\r\n", nRF24_buffer[k_nRF24].base10_exp);
        //Serial.printf("[nRF24] payload:\t%d\r\n", nRF24_buffer[k_nRF24].payload);
        Serial.printf("[nRF24] payload:\t%u %u %u %u %u %u %u %u\t\r\n", nRF24_buffer[k_nRF24].payload[0], nRF24_buffer[k_nRF24].payload[1], nRF24_buffer[k_nRF24].payload[2], nRF24_buffer[k_nRF24].payload[3], nRF24_buffer[k_nRF24].payload[4], nRF24_buffer[k_nRF24].payload[5], nRF24_buffer[k_nRF24].payload[6], nRF24_buffer[k_nRF24].payload[7]);
        Serial.printf("RX events:\t%d\r\n\n", k_nRF24);
      } else {
      // some other error occurred
        Serial.print(("[nRF24] Failed, code \r\n"));
        Serial.print(nRF24_rx_ret);
      }

      if (k_nRF24 == (MAX_HEIGHT_VAL-1)) {
        Serial.print("[nRF24] nRF24_buffer full:\r\n");
        Serial.print("[nRF24] decoding received packets:\r\n");
        k_nRF24 = 0;
        decode_nRF24(nRF24_buffer, log_buf_ptr);

        for (int i = 0; i < MAX_HEIGHT_VAL; ++i) {
          Serial.printf("[node] %s:\t%f\r\n", height_header[i], log_buffer[0][i]);
        }
      } else {
        ++k_nRF24;
      }
      nRF24_buf_ptr = &(nRF24_buffer[k_nRF24]);
      rf24_rad.startReceive();
    }

  if(LoRa_tx_flg) {
    LoRa_tx_flg = false;

	  if (LoRa_tx_ret == RADIOLIB_ERR_NONE) { Serial.print("Packet sent successfully\r\n"); }
	  else {
      Serial.print(("[SX1276] Failed, code \r\n"));
      Serial.print(LoRa_tx_ret);
	  }
	
	  lr_radio.finishTransmit();

    delay(4000);

    cast_double_uint8_t_payload(&(height_data[0][k_LoRa]), &(LoRa_PK), LORA_PAYLOAD_LEN, LoRa_PK.base10_exp);

    memcpy(LoRa_write_buf_ptr, &(LoRa_PK), sizeof(LoRa_PK));

    LoRa_tx_ret = lr_radio.startTransmit(LoRa_write_buf_ptr, sizeof(LoRa_PK));

    Serial.print("[SX1276] Transmitting packet:\r\n");
    Serial.printf("\tfstn_id:\t%d\r\n", LoRa_PK.fstn_id);
    Serial.printf("\tcontent_type:\t%d\r\n", LoRa_PK.type);
    Serial.printf("\tbase10_exp:\t%d\r\n", LoRa_PK.base10_exp);
    Serial.printf("\traw timestamp:\t%u %u %u %u %u %u %u %u\t\r\n", LoRa_PK.timestamp[0], LoRa_PK.timestamp[1], LoRa_PK.timestamp[2], LoRa_PK.timestamp[3], LoRa_PK.timestamp[4], LoRa_PK.timestamp[5], LoRa_PK.timestamp[6], LoRa_PK.timestamp[7]);
    Serial.printf("\traw payload:\t%u %u %u %u %u %u %u %u\t\r\n", LoRa_PK.payload[0], LoRa_PK.payload[1], LoRa_PK.payload[2], LoRa_PK.payload[3], LoRa_PK.payload[4], LoRa_PK.payload[5], LoRa_PK.payload[6], LoRa_PK.payload[7]);
    Serial.printf("\tTX events:\t%d\r\n\n", k_LoRa);
    Serial.printf("\ttime on air:\t%d\r\n", lr_radio.getTimeOnAir(sizeof(LoRa_PK)));
    Serial.printf("\tpower:\t%d\r\n", pwr_param[i_pwr]);
    ++k_LoRa;
    if (k_LoRa == MAX_HEIGHT_VAL) { k_LoRa = 0; }

    if ( ((k_LoRa % 4) == 0) ) { ++i_bw; }

    if ( i_bw == 10) { i_bw = 0; }
  }

}