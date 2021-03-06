#include <esp_now.h>
#include <WiFi.h>
#include <map>


// RECEIVER'S MAC Address
uint8_t broadcastAddress1[] = {0x30, 0xAE, 0xA4, 0xCC, 0x2C, 0x18};//id = 1  value = 22
uint8_t broadcastAddress2[] = {0x30, 0xAE, 0xA4, 0xCC, 0x39, 0x60};//id = 2  value = 22
uint8_t broadcastAddress3[] = {0x30, 0xAE, 0xA4, 0xCC, 0x26, 0x58};//id = 3   value = 22
uint8_t broadcastAddress4[] = {0x30, 0xAE, 0xA4, 0xCC, 0x27, 0x64};//id = 4   value = 23
uint8_t broadcastAddress5[] = {0x24, 0x6F, 0x28, 0x7B, 0xC5, 0x64};//id = 5   value = 21
uint8_t broadcastAddress6[] = {0x24, 0x0A, 0xC4, 0x5F, 0xFC, 0xD0};//id = 6 value = 21


int boardId = 5;
int boardValue = 22;
time_t startTime, endTime;
long  startBoardTime, endBoardTime;
bool isRedundancyManagement ;
bool isElectionBlockRunning;
int voterId, failledCounter;
uint8_t broadcastAddressVoter[] = {0x24, 0x6F, 0x28, 0x7B, 0xC5, 0x64};
using namespace std;
string broadcastAddressVoterName = "24:6f:28:7b:c5:64";

//------------------------performance Variable--------------------
bool isWeightOnceFilled;
std::map<int, bool> mapWeight;

//--------------------------------------------

//create a delivery map that stores bool value indexed by strings
std::map<std::string, bool> mapDelivery;

// Structure example to send data
// Must match the receiver structure
typedef struct struct_message {
  int id; // must be unique for each sender board
  int computedValue;
  int y;
} struct_message;


TaskHandle_t PerformanceMeasurment;
TaskHandle_t redundancy;

//Create a struct_message called myData
struct_message myData;

std::map<int, int> mapValue;

// Create a structure to hold the readings from each board
struct_message board1;
struct_message board2;
struct_message board3;
struct_message board4;
struct_message board5;
struct_message board6;


// Create an array with all the structures
struct_message boardsStruct[6] = {board1, board2, board3, board4, board5, board6};

// callback when data is sent
void OnDataSentToManyBord(const uint8_t *mac_addr, esp_now_send_status_t status) {
  char macStr[18];
  //For debug purposes
  //Serial.print("Packet to: ");
  // Copies the sender mac address to a string
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  //For debug purposes
  //Serial.print(macStr);
  //Serial.print(" send status:\t");
  //Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  // save status delivery
  mapDelivery[macStr] = (status == ESP_NOW_SEND_SUCCESS);
}

void OnDataRecvFromManyBoad(const uint8_t * mac_addr, const uint8_t *incomingData, int len) {

  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  //Serial.println(macStr);
  //Copy the content of the incomingData data variable into the myData variable.
  memcpy(&myData, incomingData, sizeof(myData));

  // Update the structures with the new incoming data
  boardsStruct[myData.id - 1].id = myData.id;
  boardsStruct[myData.id - 1].computedValue = myData.computedValue;

}


// LED pins
const int led1 = 4;
const int sonsor = 15;

void setup() {
  Serial.begin(115200);

  pinMode(led1, OUTPUT);
  pinMode(sonsor, INPUT);
  isRedundancyManagement  = true;

  broadcastAddressVoter[0] = 0x24;
  broadcastAddressVoter[1] = 0x0A;
  broadcastAddressVoter[2] = 0xC4;
  broadcastAddressVoter[3] = 0x5F;
  broadcastAddressVoter[4] = 0xFC;
  broadcastAddressVoter[5] = 0xD0;
  broadcastAddressVoterName = "24:0a:c4:5f:fc:d0";
  voterId = 6;
  //This is commented because it was used for performance measurement purposes
  //  xTaskCreatePinnedToCore(
  //    PerformanceMeasurmentCode, /* Function to implement the task */
  //    "PerformanceMeasurment", /* Name of the task */
  //    10000,  /* Stack size in words */
  //    NULL,  /* Task input parameter */
  //    2,  /* Priority of the task */
  //    &PerformanceMeasurment,  /* Task handle. */
  //    1); /* Core where the task should run */
  //  //delay(500);
  xTaskCreatePinnedToCore(
    redundancyCode, /* Function to implement the task */
    "redundancy", /* Name of the task */
    10000,  /* Stack size in words */
    NULL,  /* Task input parameter */
    8,  /* Priority of the task */
    &redundancy,  /* Task handle. */
    1); /* Core where the task should run */

  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_send_cb(OnDataSentToManyBord);
  // register peer
  esp_now_peer_info_t peerInfo = {};
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  // Add peer
  memcpy(peerInfo.peer_addr, broadcastAddress1, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer: broadcastAddress1");
    //return;
  }

  memcpy(peerInfo.peer_addr, broadcastAddress2, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer: broadcastAddress2");
    //return;
  }
  memcpy(peerInfo.peer_addr, broadcastAddress3, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer: broadcastAddress3");
    //return;
  }
  memcpy(peerInfo.peer_addr, broadcastAddress4, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer: broadcastAddress4");
    //return;
  }
  memcpy(peerInfo.peer_addr, broadcastAddress5, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer: broadcastAddress5");
    //return;
  }
  memcpy(peerInfo.peer_addr, broadcastAddress6, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer: broadcastAddress6");
    //return;
  }
  memcpy(peerInfo.peer_addr, broadcastAddressVoter, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer: broadcastAddressVoter");
    //return;
  }
  //delay(10000);
  esp_now_register_recv_cb(OnDataRecvFromManyBoad);
  isWeightOnceFilled = true;


}


void loop() {
}

void electionAndRedundancyManagement() {

  if (isElectionBlockRunning) {

    //doing election
    //Serial.print(" doing election \n");
    myData.id = boardId;
    // send the boardId to all boards
    esp_err_t result = esp_now_send(0, (uint8_t *) &myData, sizeof(struct_message));

    //for debug purpose
    if (result == ESP_OK) {
      Serial.println("Sent with success: send the boardId to all board \n");
    }
    else {
      Serial.println("Error sending the data: send the boardId to all board \n");
    }
    delay(10000);

    //read all boardId and set the bigger id to be the new Voter
    int board1Id = boardsStruct[0].id;
    int board2Id = boardsStruct[1].id;
    int board3Id = boardsStruct[2].id;
    int board4Id = boardsStruct[3].id;
    int board5Id = boardsStruct[4].id;
    int board6Id = boardsStruct[5].id;


    //compute the next voter by choosing the board with higher ID number
    if (board1Id != 0) {
      voterId = board1Id;
      broadcastAddressVoter[0] = 0x30;
      broadcastAddressVoter[1] = 0xAE;
      broadcastAddressVoter[2] = 0xA4;
      broadcastAddressVoter[3] = 0xCC;
      broadcastAddressVoter[4] = 0x2C;
      broadcastAddressVoter[5] = 0x18;
      broadcastAddressVoterName = "30:ae:a4:cc:2c:18";

    }
    if (board2Id != 0 && board2Id > board1Id) {
      voterId = board2Id;
      //broadcastAddressVoter = {, , , , , };
      broadcastAddressVoter[0] = 0x30;
      broadcastAddressVoter[1] = 0xAE;
      broadcastAddressVoter[2] = 0xA4;
      broadcastAddressVoter[3] = 0xCC;
      broadcastAddressVoter[4] = 0x39;
      broadcastAddressVoter[5] = 0x60;
      broadcastAddressVoterName = "30:ae:a4:cc:39:60";
    }
    if (board3Id != 0 && board3Id > board2Id) {
      voterId = board3Id;
      //broadcastAddressVoter = {0x30, 0xAE, 0xA4, 0xCC, 0x26, 0x58};
      broadcastAddressVoter[0] = 0x30;
      broadcastAddressVoter[1] = 0xAE;
      broadcastAddressVoter[2] = 0xA4;
      broadcastAddressVoter[3] = 0xCC;
      broadcastAddressVoter[4] = 0x26;
      broadcastAddressVoter[5] = 0x58;
      broadcastAddressVoterName = "30:ae:a4:cc:26:58";
    }
    if (board4Id != 0 && board4Id > board3Id) {
      voterId = board4Id;
      // broadcastAddressVoter = {0x30, 0xAE, 0xA4, 0xCC, 0x27, 0x64};
      broadcastAddressVoter[0] = 0x30;
      broadcastAddressVoter[1] = 0xAE;
      broadcastAddressVoter[2] = 0xA4;
      broadcastAddressVoter[3] = 0xCC;
      broadcastAddressVoter[4] = 0x27;
      broadcastAddressVoter[5] = 0x64;
      broadcastAddressVoterName = "30:ae:a4:cc:27:64";
    }
    if (board5Id != 0 && board5Id > board4Id) {
      voterId = board5Id;
      //broadcastAddressVoter = {0x24, 0x6F, 0x28, 0x7B, 0xC5, 0x64};
      broadcastAddressVoter[0] = 0x24;
      broadcastAddressVoter[1] = 0x6F;
      broadcastAddressVoter[2] = 0x28;
      broadcastAddressVoter[3] = 0x7B;
      broadcastAddressVoter[4] = 0xC5;
      broadcastAddressVoter[5] = 0x64;
      broadcastAddressVoterName = "24:6f:28:7b:c5:64";
    }
    if (board6Id != 0 && board6Id > board5Id) {
      voterId = board6Id;
      //broadcastAddressVoter = {0x24, 0x0A, 0xC4, 0x5F, 0xFC, 0xD0};
      broadcastAddressVoter[0] = 0x24;
      broadcastAddressVoter[1] = 0x0A;
      broadcastAddressVoter[2] = 0xC4;
      broadcastAddressVoter[3] = 0x5F;
      broadcastAddressVoter[4] = 0xFC;
      broadcastAddressVoter[5] = 0xD0;
      broadcastAddressVoterName = "24:0a:c4:5f:fc:d0";
    }

    //clear all boardId
    boardsStruct[0].id = 0;
    boardsStruct[1].id = 0;
    boardsStruct[2].id = 0;
    boardsStruct[3].id = 0;
    boardsStruct[4].id = 0;
    boardsStruct[5].id = 0;

    isElectionBlockRunning = false;
    isRedundancyManagement  = true;
    //endBoardTime = micros();
    //For debug purposes
    Serial.printf(" elected voter id %d:\n", voterId);
  }

  if (isRedundancyManagement ) {
    //doing redundancy management
    //Serial.print(" doing redundancy management \n");

    if (voterId <= boardId) {
      delay(100);
      //compare all recieved values and set the output
      Serial.print("compare all recieved values and set the output \n");

      if (boardsStruct[0].computedValue != 0) {
        mapValue[1] = boardsStruct[0].computedValue;
      }
      if (boardsStruct[1].computedValue != 0) {
        mapValue [2] = boardsStruct[1].computedValue;
      }
      if (boardsStruct[2].computedValue != 0) {
        mapValue [3] = boardsStruct[2].computedValue;
      }
      if (boardsStruct[3].computedValue != 0) {
        mapValue [4] = boardsStruct[3].computedValue;
      }
      if (boardsStruct[4].computedValue != 0) {
        mapValue [5] = boardsStruct[4].computedValue;
      }
      if (mapValue.size() > 0) {
        int  allBoardValues[mapValue.size()];
        int i = 0;
        for ( auto item : mapValue )
        {
          allBoardValues[i] = item.second;
          i++;
        }

        //compare all value and set the GPIO accordinly
        int elementCount = sizeof(allBoardValues) / sizeof(allBoardValues[0]);
        int mostCommon = mostFrequent(allBoardValues, elementCount);

        //clear computed value to avoid to considere this value even when the board failed
        boardsStruct[0].computedValue = 0;
        boardsStruct[1].computedValue = 0;
        boardsStruct[2].computedValue = 0;
        boardsStruct[3].computedValue = 0;
        boardsStruct[4].computedValue = 0;

        //clear mapValue
        std::fill_n(allBoardValues, elementCount, 0);
        mapValue.clear();

        //For debug purposes
        Serial.printf(" most common value = %d:\n", mostCommon);
      }

    } else {
      //read the sensor value an send the computed value to the voter
      //Serial.print("read the sensor value and send the computed value to the voter \n");
      struct_message myData;
      if (digitalRead(sonsor) == HIGH) {
        myData.computedValue = boardValue;
      } else {
        myData.computedValue = boardValue;
      }
      myData.id = boardId;

      delay(500);
      esp_err_t result = esp_now_send
                         (broadcastAddressVoter,
                          (uint8_t *) &myData,
                          sizeof(struct_message));

      if (result == ESP_OK && !mapDelivery[broadcastAddressVoterName]) {
        //Sent to voter with success but delivery failed
        //Serial.println("Sent to voter with success but delivery failed \n");
        failledCounter++;
        if (failledCounter == 3) {
          //Serial.println("Sent to voter with success but delivery failed three time\n");
          isElectionBlockRunning = true;
          isRedundancyManagement  = false;

          //clear all boardId
          boardsStruct[0].id = 0;
          boardsStruct[1].id = 0;
          boardsStruct[2].id = 0;
          boardsStruct[3].id = 0;
          boardsStruct[4].id = 0;
          boardsStruct[5].id = 0;
          voterId = 0;
          failledCounter = 0;
        }
      }

      delay(10000);
    }
  }
}


int mostFrequent(int nums[], int n)
{
  int max_count = 0, res = nums[0];
  for (int i = 0; i < n; i++)
  {
    int count = 1;
    for (int j = i + 1; j < n; j++)
      if (nums[i] == nums[j])
        count++;
    if (count > max_count)
      max_count = count;
  }

  for (int i = 0; i < n; i++)
  {
    int count = 1;
    for (int j = i + 1; j < n; j++)
      if (nums[i] == nums[j])
        count++;
    if (count == max_count)
      res = nums[i];
  }

  return res;
}


void PerformanceMeasurmentTest() {

  double a, b, c, d;
  a = 52;
  b = 312;
  c = 34;
  int nums[] = { 1, 5, 2, 1, 3, 2, 1, 4, 5, 5, 4, 6, 7, 7, 8};
  int n = 15;
  for (int i = 0; i < 100000000; i++)
  {
    for (int i = 0; i < 80000000; i++)
    {
      for (int i = 0; i < 80000000; i++)
      {
        d = ((a + b) * (a - b)) / (a + b);
        int max_count = 0, res = nums[0];
        for (int i = 0; i < n; i++)
        {
          int count = 1;
          for (int j = i + 1; j < n; j++)
            if (nums[i] == nums[j])
              count++;
          if (count > max_count)
            max_count = count;
        }

        for (int i = 0; i < n; i++)
        {
          int count = 1;
          for (int j = i + 1; j < n; j++)
            if (nums[i] == nums[j])
              count++;
          if (count == max_count)
            res = nums[i];
        }
      }
    }
  }

  for (int i = 0; i < 100000000; i++)
  {
    for (int i = 0; i < 80000000; i++)
    {
      for (int i = 0; i < 80000000; i++)
      {
        d = ((a + b) * (a - b)) / (a + b);
        int max_count = 0, res = nums[0];
        for (int i = 0; i < n; i++)
        {
          int count = 1;
          for (int j = i + 1; j < n; j++)
            if (nums[i] == nums[j])
              count++;
          if (count > max_count)
            max_count = count;
        }

        for (int i = 0; i < n; i++)
        {
          int count = 1;
          for (int j = i + 1; j < n; j++)
            if (nums[i] == nums[j])
              count++;
          if (count == max_count)
            res = nums[i];
        }
      }
    }
  }

  for (int i = 0; i < 100000000; i++)
  {
    for (int i = 0; i < 80000000; i++)
    {
      for (int i = 0; i < 80000000; i++)
      {
        d = ((a + b) * (a - b)) / (a + b);
        int max_count = 0, res = nums[0];
        for (int i = 0; i < n; i++)
        {
          int count = 1;
          for (int j = i + 1; j < n; j++)
            if (nums[i] == nums[j])
              count++;
          if (count > max_count)
            max_count = count;
        }

        for (int i = 0; i < n; i++)
        {
          int count = 1;
          for (int j = i + 1; j < n; j++)
            if (nums[i] == nums[j])
              count++;
          if (count == max_count)
            res = nums[i];
        }
      }
    }
  }


  //delay(10000);
  if (isWeightOnceFilled) {
    mapWeight[boardsStruct[0].id] = true;
    mapWeight[boardsStruct[1].id] = true;
    mapWeight[boardsStruct[2].id] = true;
    mapWeight[boardsStruct[3].id] = true;
    mapWeight[boardsStruct[4].id] = true;
    isWeightOnceFilled = false;
  }

  //compute the size of the array of value
  if (mapWeight[boardsStruct[0].id] && boardsStruct[0].computedValue != 0) {
    mapValue[1] = boardsStruct[0].computedValue;
  }
  if (mapWeight[boardsStruct[1].id] && boardsStruct[1].computedValue != 0) {
    mapValue[2] = boardsStruct[1].computedValue;
  }
  if (mapWeight[boardsStruct[2].id] && boardsStruct[2].computedValue != 0) {
    mapValue[3] = boardsStruct[2].computedValue;
  }
  if (mapWeight[boardsStruct[3].id] && boardsStruct[3].computedValue != 0) {
    mapValue[4] = boardsStruct[3].computedValue;
  }
  if (mapWeight[boardsStruct[4].id] && boardsStruct[4].computedValue != 0) {
    mapValue[5] = boardsStruct[4].computedValue;
  }
  if (mapValue.size() > 0) {
    //check the switch and create the array of value
    int  arr[mapValue.size()];
    int i = 0;
    for ( auto item : mapValue )
    {
      arr[i] = item.second;
      i++;
    }

    //get the common value
    int elementCount = sizeof(arr) / sizeof(arr[0]);
    int commonValue = mostFrequent(arr, elementCount);

    //refresh the map with new weight
    if (boardsStruct[0].computedValue == commonValue) {
      mapWeight[boardsStruct[0].id] = true;
    } else {
      mapWeight[boardsStruct[0].id] = false;
    }

    if (boardsStruct[1].computedValue == commonValue) {
      mapWeight[boardsStruct[1].id] = true;
    } else {
      mapWeight[boardsStruct[1].id] = false;
    }

    if (boardsStruct[2].computedValue == commonValue) {
      mapWeight[boardsStruct[2].id] = true;
    } else {
      mapWeight[boardsStruct[2].id] = false;
    }

    if (boardsStruct[3].computedValue == commonValue) {
      mapWeight[boardsStruct[3].id] = true;
    } else {
      mapWeight[boardsStruct[3].id] = false;
    }

    if (boardsStruct[4].computedValue == commonValue) {
      mapWeight[boardsStruct[4].id] = true;
    } else {
      mapWeight[boardsStruct[4].id] = false;
    }

    //delay(10000);
    //clear all boardId
    boardsStruct[0].id = 0;
    boardsStruct[1].id = 0;
    boardsStruct[2].id = 0;
    boardsStruct[3].id = 0;
    boardsStruct[4].id = 0;

    //clear all boardValue
    boardsStruct[0].computedValue = 0;
    boardsStruct[1].computedValue = 0;
    boardsStruct[2].computedValue = 0;
    boardsStruct[3].computedValue = 0;
    boardsStruct[4].computedValue = 0;

    // clear array
    std::fill_n(arr, elementCount, 0);
    mapValue.clear();

    //delay(100);
    //For debug purposes
    //Serial.printf("common value: %d \n", commonValue);
  }
}

void PerformanceMeasurmentCode( void * pvParameters ) {

  long  startBoardTime, endBoardTime;
  for (;;) {
    startBoardTime = micros();
    for (int i = 0; i < 10000; i++) {
      PerformanceMeasurmentTest();
      PerformanceMeasurmentTest();
      PerformanceMeasurmentTest();
      PerformanceMeasurmentTest();
    }
    endBoardTime = micros();
    delay(100);
    Serial.printf("performanceMeasurementTaskTime is running since %d microseconds\n", endBoardTime - startBoardTime);
  }
}


void redundancyCode( void * pvParameters ) {

  long  startBoardTime, endBoardTime;
  for (;;) {
    electionAndRedundancyManagement();
  }

}
