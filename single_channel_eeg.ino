#define SAMPLE_RATE 256  // Sampling rate in Hz
#define BAUD_RATE 115200 // Baud rate for serial communication
#define INPUT_PIN_1 A0   // Input pin for the first channel
#define MOVING_AVG_SIZE 10  // Size of the moving average filter

// Moving average variables
float movingAvgBuffer[MOVING_AVG_SIZE];
int bufferIndex = 0;

void setup() {
  // Start serial communication
  Serial.begin(BAUD_RATE);
  
  // Allow time for the Serial to initialize before sending data
  delay(1000);
  
  // Initialize the moving average buffer
  for (int i = 0; i < MOVING_AVG_SIZE; i++) {
    movingAvgBuffer[i] = 0;
  }

  // Print CSV header
  Serial.println("Channel 1");
}

void loop() {
  // Calculate elapsed time
  static unsigned long past = 0;
  unsigned long present = micros();
  unsigned long interval = present - past;
  past = present;

  // Timer to ensure proper sampling rate
  static long timer = 0;
  timer -= interval;

  if (timer < 0) {
    timer += 1000000 / SAMPLE_RATE;

    // Read raw values from the input channel
    float sensor_value_1 = analogRead(INPUT_PIN_1);

    // Apply filter to the input channel
    float filtered_signal_1 = EEGFilter(sensor_value_1);

    // Remove power-line artifact using notch filter (50/60 Hz)
    float artifact_removed_signal = NotchFilter(filtered_signal_1, SAMPLE_RATE);

    // Smooth the signal using moving average
    float smoothed_signal = MovingAverage(artifact_removed_signal);

    // Output the filtered data in CSV format with limited precision
    Serial.println(smoothed_signal, 4);  // 4 decimal places for Channel 1
  }
}

// Band-Pass Butterworth filter for EEG signals (12-30 Hz)
float EEGFilter(float input) {
  static float z1_1 = 0, z2_1 = 0; // Filter states
  float x = input - (-1.34325147 * z1_1 + 0.51398189 * z2_1);
  float output = (0.17508764 * x) + (-0.35017528 * z1_1) + (0.17508764 * z2_1);
  z2_1 = z1_1;
  z1_1 = x;

  return output;
}

// Notch filter to remove power-line interference (50/60 Hz)
float NotchFilter(float input, int sampleRate) {
  static float notch_z1 = 0, notch_z2 = 0; // Notch filter states
  static const float notch_b0 = 1.0, notch_b1 = -2.0, notch_b2 = 1.0; 
  static const float notch_a1 = -1.9941, notch_a2 = 0.9940; // For 50 Hz (adjust for 60 Hz)

  // Apply notch filter to remove 50 Hz noise
  float output = notch_b0 * input + notch_b1 * notch_z1 + notch_b2 * notch_z2 - notch_a1 * notch_z1 - notch_a2 * notch_z2;

  // Update filter states
  notch_z2 = notch_z1;
  notch_z1 = input;

  return output;
}

// Moving average filter to smooth the signal
float MovingAverage(float input) {
  // Add new value to buffer
  movingAvgBuffer[bufferIndex] = input;

  // Update buffer index
  bufferIndex = (bufferIndex + 1) % MOVING_AVG_SIZE;

  // Calculate the average of the buffer
  float sum = 0;
  for (int i = 0; i < MOVING_AVG_SIZE; i++) {
    sum += movingAvgBuffer[i];
  }

  return sum / MOVING_AVG_SIZE;
}
