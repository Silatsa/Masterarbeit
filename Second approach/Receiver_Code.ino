#include <esp_now.h>
#include <WiFi.h>
#include <map>
// Structure to receive data
// Must match the sender structure
typedef struct struct_message {
  int id;
  int computedValue;
  int y;
} struct_message;

long  startBoardTime, endBoardTime;

// Create a struct_message called myData
struct_message myData;

// Create a structure to hold the readings from each board
struct_message board1;
struct_message board2;
struct_message board3;
struct_message board4;
struct_message board5;

bool isWeightOnceFilled;

TaskHandle_t PerformanceMeasurment;
TaskHandle_t redundancy;

//create a weight map for switching that stores bool value indexed by id
std::map<int, bool> mapWeight;

std::map<int, int> mapValue;

// Create an array with all the structures
struct_message boardsStruct[5] = {board1, board2, board3, board4, board5};

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len) {
  char macStr[18];
  //  Serial.print("Packet received from: ");
  //  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
  //   mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  //  Serial.println(macStr);
  memcpy(&myData, incomingData, sizeof(myData));
  //Serial.printf("Board ID %u: %u bytes\n", myData.id, len);
  // Update the structures with the new incoming data
  boardsStruct[myData.id - 1].computedValue = myData.computedValue;
  boardsStruct[myData.id - 1].id = myData.id;
  //For debug purposes
  //    Serial.printf("id value: %d \n", boardsStruct[myData.id - 1].id);
  //    Serial.printf("value: %d \n", boardsStruct[myData.id - 1].computedValue);
  //    Serial.println();
}

void setup() {
  //Initialize Serial Monitor
  Serial.begin(115200);

  isWeightOnceFilled = true;

  //Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  //Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for recv CB to get recv packer info
  esp_now_register_recv_cb(OnDataRecv);

  //This is commented because it was used for performance measurement purposes
  //  xTaskCreatePinnedToCore(
  //    PerformanceMeasurmentCode, /* Function to implement the task */
  //    "PerformanceMeasurment", /* Name of the task */
  //    10000,  /* Stack size in words */
  //    NULL,  /* Task input parameter */
  //    2,  /* Priority of the task */
  //    &PerformanceMeasurment,  /* Task handle. */
  //    1); /* Core where the task should run */

  xTaskCreatePinnedToCore(
    redundancyCode, /* Function to implement the task */
    "redundancy", /* Name of the task */
    10000,  /* Stack size in words */
    NULL,  /* Task input parameter */
    4,  /* Priority of the task */
    &redundancy,  /* Task handle. */
    1); /* Core where the task should run */
}

void loop() {

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

void redundancyManagement() {

  delay(10000);

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
    //clear all boardId to not considere old value
    boardsStruct[0].id = 0;
    boardsStruct[1].id = 0;
    boardsStruct[2].id = 0;
    boardsStruct[3].id = 0;
    boardsStruct[4].id = 0;

    //clear all boardValue to not considere old value
    boardsStruct[0].computedValue = 0;
    boardsStruct[1].computedValue = 0;
    boardsStruct[2].computedValue = 0;
    boardsStruct[3].computedValue = 0;
    boardsStruct[4].computedValue = 0;

    // clear array to not considere old value
    std::fill_n(arr, elementCount, 0);
    mapValue.clear();

    //delay(10000);
    //For debug purposes
    Serial.printf("common value: %d \n", commonValue);
  }
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
          //Serial.printf(" common value in performanTask %d:\n", res );
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
          //Serial.printf(" common value in performanTask %d:\n", res );
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
          //Serial.printf(" common value in performanTask %d:\n", res );
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
    //clear all boardId to not considere old value
    boardsStruct[0].id = 0;
    boardsStruct[1].id = 0;
    boardsStruct[2].id = 0;
    boardsStruct[3].id = 0;
    boardsStruct[4].id = 0;

    //clear all boardValue to not considere old value
    boardsStruct[0].computedValue = 0;
    boardsStruct[1].computedValue = 0;
    boardsStruct[2].computedValue = 0;
    boardsStruct[3].computedValue = 0;
    boardsStruct[4].computedValue = 0;

    // clear array to not considere old value
    std::fill_n(arr, elementCount, 0);
    mapValue.clear();

    //delay(10000);
    //For debug purposes
    Serial.printf("common value: %d \n", commonValue);
  }
}


void redundancyCode( void * pvParameters ) {

  for (;;) {

    redundancyManagement();

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

    //For debug purposes
    Serial.printf("performanceMeasurementTaskTime is running since %d microseconds\n", endBoardTime - startBoardTime);
  }
}
