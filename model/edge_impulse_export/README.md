# Edge Impulse Model Export

This directory is the designated location for the compiled TinyML model exported from Edge Impulse.

To deploy the model:
1. Train the model on Edge Impulse Studio.
2. Navigate to **Deployment** and select **Arduino library**.
3. Export the unoptimized (Float32) or Quantized (Int8) model.
4. Place the resulting `Smart_Appliance_Identification_inferencing.zip` file in this folder for archival purposes.
5. Install the `.zip` file via the Arduino IDE to compile the ESP8266 firmware.

*Note: The actual `.zip` file may be ignored by `.gitignore` if it exceeds standard repository size limits.*