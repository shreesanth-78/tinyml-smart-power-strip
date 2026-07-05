import pandas as pd
import numpy as np
import os

def generate_windows(input_csv, output_dir, class_name, window_size=200):
    """
    Reads a raw ADC CSV, chunks it into non-overlapping windows,
    and maps physical 500 Hz acquisition to logical 1000 Hz timestamps.
    """
    try:
        # Read raw ADC data (Assuming a single column of values)
        df = pd.read_csv(input_csv, header=None, names=['adc_value'])
    except FileNotFoundError:
        print(f"Error: {input_csv} not found.")
        return

    num_windows = len(df) // window_size
    os.makedirs(output_dir, exist_ok=True)
    
    print(f"Processing {class_name}... generating {num_windows} windows.")

    for i in range(num_windows):
        start_idx = i * window_size
        end_idx = start_idx + window_size
        window_data = df.iloc[start_idx:end_idx].copy()
        
        # Inject 1 ms increment timestamps (1000 Hz logical rate)
        window_data['timestamp_ms'] = np.arange(0, window_size)
        
        # Reorder columns: timestamp_ms first, then adc_value
        window_data = window_data[['timestamp_ms', 'adc_value']]
        
        # Save to processed folder
        output_file = os.path.join(output_dir, f"{class_name}_window_{i:04d}.csv")
        window_data.to_csv(output_file, index=False)

if __name__ == "__main__":
    print("Starting TinyML Window Generation...")
    
    # Example usage for the 5 classes:
    # classes = ['noload', 'phone', 'laptop', 'fan', 'kettle']
    # for c in classes:
    #     generate_windows(f'../data/raw/{c}_raw.csv', f'../data/processed/{c}', c)
    
    print("Done. Ready for Edge Impulse upload.")