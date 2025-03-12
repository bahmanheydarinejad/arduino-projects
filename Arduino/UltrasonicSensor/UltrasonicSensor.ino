const int trigPin = 9;
const int echoPin = 10;

const int numSamples = 5;  // Number of samples for median filtering
float distances[numSamples];

void setup() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  Serial.begin(115200);
}

void loop() {
  float distance = getFilteredDistance();
  
  // Print filtered distance
  if (distance > 2 && distance < 400) {  // Only show valid readings
    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.println(" cm");
  } else {
    Serial.println("Invalid reading, discarded.");
  }

  delay(50);
}

// Function to take multiple readings and return the median distance
float getFilteredDistance() {
  for (int i = 0; i < numSamples; i++) {
    distances[i] = getRawDistance();
    delay(10);  // Small delay to prevent sensor interference
  }
  
  // Sort array and return median value
  return getMedian(distances, numSamples);
}

// Function to take a single raw distance reading
float getRawDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(5);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH);
  float distance = (duration * 0.0343) / 2.0;
  
  return distance;
}

// Function to find the median of an array
float getMedian(float *arr, int size) {
  float temp[size];
  memcpy(temp, arr, sizeof(temp));  // Copy array to avoid modifying original
  
  // Sort the array
  for (int i = 0; i < size - 1; i++) {
    for (int j = i + 1; j < size; j++) {
      if (temp[i] > temp[j]) {
        float swap = temp[i];
        temp[i] = temp[j];
        temp[j] = swap;
      }
    }
  }
  
  return temp[size / 2];  // Return median value
}
