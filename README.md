1. Project Overview
This project focuses on the development of a system for forecasting the occurrence of the pathogen Venturia Inaequalis and Cydia Pomonella in apples.
The goal is to provide an efficient tool for early detection and control of these pathogens in apple production.
2. System Components
2.1 Backend:
The core logic of the system is implemented in C++, which is responsible for processing data and controlling hardware components.
2.2 Hardware:
The system uses an Arduino microcontroller as the central unit. It is responsible for collecting data from various sensors and performing initial data processing.
3. Features
3.1 Data Collection via Sensors
Air Temperature and Humidity – Uses a DHT sensor to monitor microclimatic conditions that influence disease development.
Soil Moisture – Measures soil moisture levels, which is crucial for preventing fungal infections.
3.2 Data Processing and Risk Detection
Combines data from various sensors to detect optimal conditions for apple disease development.
Analyzes data trends to identify early signs of potential issues.
3.3 User Alerts
Sends alerts via serial communication when unfavorable conditions are detected.
