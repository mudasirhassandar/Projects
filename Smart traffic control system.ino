// ---------- PIN DEFINITIONS ----------
const int trigPin[4]  = {2, 8, 5, 11};
const int echoPin[4]  = {9, 3, 4, 12};

const int redLED[4]   = {A0, 13, 6, A2};
const int greenLED[4] = {A1, 10, 7, A3};

// ---------- VARIABLES ----------
int currentLane = -1;                 // Active GREEN lane
unsigned long greenStartTime = 0;     // Time when GREEN started
const unsigned long GREEN_TIME = 15000; // 15 seconds max
const int DETECT_DIST = 20;           // cm

// ---------- SETUP ----------
void setup() {
  Serial.begin(9600);
  for (int i = 0; i < 4; i++) {
    pinMode(trigPin[i], OUTPUT);
    pinMode(echoPin[i], INPUT);
    pinMode(redLED[i], OUTPUT);
    pinMode(greenLED[i], OUTPUT);
  }
  setAllRed();
}

// ---------- MAIN LOOP ----------
void loop() {
  bool traffic[4];

  // Read traffic presence
  for (int i = 0; i < 4; i++) {
    traffic[i] = detectVehicle(i);
    Serial.print("Lane "); Serial.print(i); 
    Serial.print(": "); Serial.println(traffic[i] ? "Vehicle" : "No Vehicle");
  }

  int trafficCount = countTraffic(traffic);

  if (trafficCount == 0) {
    setAllRed();
    currentLane = -1;
    return;
  }

  if (trafficCount == 1) {
    for (int i = 0; i < 4; i++) {
      if (traffic[i]) {
        setGreen(i);
        currentLane = i;
      }
    }
    return;
  }

  handleMultipleTraffic(traffic);
}

// ---------- FUNCTIONS ----------
bool detectVehicle(int lane) {
  digitalWrite(trigPin[lane], LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin[lane], HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin[lane], LOW);

  long duration = pulseIn(echoPin[lane], HIGH, 25000);
  int distance = duration * 0.034 / 2;

  return (distance > 0 && distance <= DETECT_DIST);
}

int countTraffic(bool t[]) {
  int count = 0;
  for (int i = 0; i < 4; i++) if (t[i]) count++;
  return count;
}

void handleMultipleTraffic(bool traffic[]) {
  unsigned long now = millis();

  // Pick next lane if none active
  if (currentLane == -1) {
    currentLane = getNextLane(traffic, 0);
    greenStartTime = now;
    setGreen(currentLane);
    return;
  }

  // If current lane cleared
  if (!traffic[currentLane]) {
    setRed(currentLane);
    currentLane = getNextLane(traffic, currentLane + 1);
    greenStartTime = now;
    setGreen(currentLane);
    return;
  }

  // If GREEN time exceeded
  if (now - greenStartTime >= GREEN_TIME) {
    setRed(currentLane);
    currentLane = getNextLane(traffic, currentLane + 1);
    greenStartTime = now;
    setGreen(currentLane);
  }
}

int getNextLane(bool traffic[], int start) {
  for (int i = 0; i < 4; i++) {
    int lane = (start + i) % 4;
    if (traffic[lane]) return lane;
  }
  return -1;
}

// ---------- LED CONTROL ----------
void setAllRed() {
  for (int i = 0; i < 4; i++) {
    digitalWrite(redLED[i], HIGH);
    digitalWrite(greenLED[i], LOW);
  }
}

void setGreen(int lane) {
  setAllRed();
  digitalWrite(redLED[lane], LOW);
  digitalWrite(greenLED[lane], HIGH);
}

void setRed(int lane) {
  digitalWrite(redLED[lane], HIGH);
  digitalWrite(greenLED[lane], LOW);
}
